#include "TaoQGraphHybrid.h"
#include <QSGGeometryNode>
#include <QSGSimpleTextureNode>
#include <QSGMaterial>
#include <QSGMaterialShader>
#include <QSGFlatColorMaterial>
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <dlfcn.h>
#include <QSGTransformNode>
#include <QPainter>
#include <QtMath>
#include <QRandomGenerator>
#include <QQuickWindow>
#include <QtConcurrent>
#include <QTime>
#include <cstring>
#include <cmath>

// ── ParticleMaterialShader ────────────────────────────────────────────────────

static QString soDir()
{
    Dl_info info;
    if (dladdr(reinterpret_cast<void*>(&soDir), &info) && info.dli_fname)
        return QFileInfo(QString::fromUtf8(info.dli_fname)).absolutePath();
    return QString();
}

class ParticleMaterialShader : public QSGMaterialShader
{
public:
    ParticleMaterialShader()
    {
        const QString dir  = soDir();
        const QString vert = dir + QStringLiteral("/shaders/particle.vert.qsb");
        const QString frag = dir + QStringLiteral("/shaders/particle.frag.qsb");

        qDebug() << "[ParticleShader] .so dir:" << dir;
        qDebug() << "[ParticleShader] vert exists:" << QFile::exists(vert) << vert;
        qDebug() << "[ParticleShader] frag exists:" << QFile::exists(frag) << frag;

        setShaderFileName(VertexStage,   vert);
        setShaderFileName(FragmentStage, frag);
    }

    bool updateUniformData(RenderState &state,
                           QSGMaterial *newMaterial,
                           QSGMaterial * /*oldMaterial*/) override
    {
        bool changed = false;
        QByteArray *buf = state.uniformData();

        if (state.isMatrixDirty()) {
            const QMatrix4x4 m = state.combinedMatrix();
            memcpy(buf->data(), m.constData(), 64);
            changed = true;
        }
        if (state.isOpacityDirty()) {
            const float opacity = state.opacity();
            memcpy(buf->data() + 64, &opacity, 4);
            changed = true;
        }
        Q_UNUSED(newMaterial)
        return changed;
    }
};

// ── ParticleMaterial ──────────────────────────────────────────────────────────
static QSGMaterialType particleMaterialType;

ParticleMaterial::ParticleMaterial()
{
    setFlag(Blending, true);
    setFlag(RequiresFullMatrix, true);
}

QSGMaterialType *ParticleMaterial::type() const { return &particleMaterialType; }

QSGMaterialShader *ParticleMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new ParticleMaterialShader();
}

// ── Helpers ───────────────────────────────────────────────────────────────────

inline quint32 packColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    float af = a / 255.0f;
    unsigned char pr = static_cast<unsigned char>(r * af);
    unsigned char pg = static_cast<unsigned char>(g * af);
    unsigned char pb = static_cast<unsigned char>(b * af);
    return (quint32(a) << 24) | (quint32(pb) << 16) | (quint32(pg) << 8) | quint32(pr);
}

static const QSGGeometry::AttributeSet &particleAttributes() {
    static QSGGeometry::Attribute data[] = {
        QSGGeometry::Attribute::create(0, 2, QSGGeometry::FloatType, false),
        QSGGeometry::Attribute::create(1, 2, QSGGeometry::FloatType),
        QSGGeometry::Attribute::create(2, 4, QSGGeometry::UnsignedByteType, true)
    };
    static QSGGeometry::AttributeSet attrs = { 3, 20, data };
    return attrs;
}

// ── Costruttore / Distruttore ─────────────────────────────────────────────────

TaoQGraphHybrid::TaoQGraphHybrid(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
    m_particles.resize(MAX_PARTICLES);
    m_particlesRender.resize(MAX_PARTICLES);
    // OPT 5: buffer quad pre-buildati, allocato una sola volta al max
    m_quadsRender.resize(MAX_PARTICLES);

    std::memset(m_particles.data(), 0, sizeof(ParticleData) * MAX_PARTICLES);

    connect(&m_watcher, &QFutureWatcher<void>::finished, this, [this]() {
        if (m_particleCount > 0) {
            std::memcpy(m_particlesRender.data(), m_particles.data(),
                        m_particleCount * sizeof(ParticleData));
        }
        m_renderCount = m_particleCount;
        m_simulationPending = false;
        update();
    });

    m_timeTracker.start();
}

TaoQGraphHybrid::~TaoQGraphHybrid() {
    if (m_watcher.isRunning())
        m_watcher.waitForFinished();
}

// ── Setters ───────────────────────────────────────────────────────────────────

void TaoQGraphHybrid::setParticleCount(int count) {
    if (m_particleCount == count) return;
    m_particleCount = qBound(0, count, MAX_PARTICLES);
    Q_EMIT particleCountChanged();
}

void TaoQGraphHybrid::setRotationSpeed(float speed) {
    if (qFuzzyCompare(m_rotationSpeed, speed)) return;
    m_rotationSpeed = speed;
    Q_EMIT rotationSpeedChanged();
}

void TaoQGraphHybrid::setClockwise(bool clockwise) {
    if (m_clockwise == clockwise) return;
    m_clockwise = clockwise;
    Q_EMIT clockwiseChanged();
}

void TaoQGraphHybrid::setShowClock(bool show) {
    if (m_showClock == show) return;
    m_showClock = show;
    Q_EMIT showClockChanged();
    update();
}

void TaoQGraphHybrid::setHourHandColor(const QColor &c) {
    if (m_hourHandColor == c) return;
    m_hourHandColor = c;
    Q_EMIT hourHandColorChanged();
    update();
}

void TaoQGraphHybrid::setMinuteHandColor(const QColor &c) {
    if (m_minuteHandColor == c) return;
    m_minuteHandColor = c;
    Q_EMIT minuteHandColorChanged();
    update();
}

void TaoQGraphHybrid::setSecondHandColor(const QColor &c) {
    if (m_secondHandColor == c) return;
    m_secondHandColor = c;
    Q_EMIT secondHandColorChanged();
    update();
}

void TaoQGraphHybrid::setGlowColor1(const QColor &c) {
    if (m_glowColor1 == c) return;
    m_glowColor1 = c;
    Q_EMIT glowColor1Changed();
    update();
}

void TaoQGraphHybrid::setGlowSize1(double s) {
    if (qFuzzyCompare(m_glowSize1, s)) return;
    m_glowSize1 = s;
    Q_EMIT glowSize1Changed();
    update();
}

void TaoQGraphHybrid::setGlowColor2(const QColor &c) {
    if (m_glowColor2 == c) return;
    m_glowColor2 = c;
    Q_EMIT glowColor2Changed();
    update();
}

void TaoQGraphHybrid::setGlowSize2(double s) {
    if (qFuzzyCompare(m_glowSize2, s)) return;
    m_glowSize2 = s;
    Q_EMIT glowSize2Changed();
    update();
}

void TaoQGraphHybrid::setParticleColor1(const QColor &c) {
    if (m_particleColor1 == c) return;
    m_particleColor1 = c;
    Q_EMIT particleColor1Changed();
    update();
}

void TaoQGraphHybrid::setParticleColor2(const QColor &c) {
    if (m_particleColor2 == c) return;
    m_particleColor2 = c;
    Q_EMIT particleColor2Changed();
    update();
}

void TaoQGraphHybrid::setLowCpuMode(bool lowCpu) {
    if (m_lowCpuMode == lowCpu) return;
    m_lowCpuMode = lowCpu;
    Q_EMIT lowCpuModeChanged();
}

void TaoQGraphHybrid::setMousePos(const QPointF &pos) {
    if (m_mousePos == pos) return;
    m_mousePos = pos;
    Q_EMIT mousePosChanged();
}

void TaoQGraphHybrid::itemChange(ItemChange change, const ItemChangeData &value) {
    if (change == ItemVisibleHasChanged && value.boolValue)
        update();
    QQuickItem::itemChange(change, value);
}

// ── updateSimulation ──────────────────────────────────────────────────────────

void TaoQGraphHybrid::updateSimulation() {
    if (m_simulationPending || m_lowCpuMode || !isVisible())
        return;

    m_simulationPending = true;

    const int count       = m_particleCount;
    if (count <= 0) {
        m_renderCount = 0;
        m_simulationPending = false;
        update();
        return;
    }

    const float  w         = width();
    const float  h         = height();
    const QPointF mPos     = m_mousePos;
    const float currentDt  = (m_lastDt > 0.001f && m_lastDt < 1.0f) ? m_lastDt : 0.016f;
    const QColor pc1       = m_particleColor1;
    const QColor pc2       = m_particleColor2;

    QFuture<void> future = QtConcurrent::run([this, count, w, h, mPos, currentDt, pc1, pc2]() {
        if (count <= 0) return;

        const float cx  = w * 0.5f;
        const float cy  = h * 0.5f;
        const float r   = qMin(w, h) / 4.5f;
        const float rSq = r * r;
        const float df  = currentDt * 60.0f;

        // ── OPT 1: friction pre-calcolato UNA VOLTA fuori dal loop ────────────
        // std::pow(0.98f, df) è una funzione trascendente (~20-40 cicli CPU).
        // df è costante per tutto il frame: ricalcolarlo per ogni particella
        // è uno spreco puro. Con 1000 particelle = 1000 pow evitati per frame.
        const float friction = std::pow(0.98f, df);

        // ── OPT 3: generatore casuale locale al thread ────────────────────────
        // QRandomGenerator::global() acquisisce un lock ad ogni chiamata per
        // garantire thread-safety. Un generatore locale elimina la contesa
        // sul mutex, con beneficio visibile quando molte particelle fanno
        // respawn contemporaneamente (es. al primo frame o dopo un reset).
        QRandomGenerator localGen(QRandomGenerator::global()->generate());

        const bool  isMouseValid = (mPos.x() >= 0 && mPos.y() >= 0 &&
                                    mPos.x() <= w  && mPos.y() <= h);
        const float mx = static_cast<float>(mPos.x());
        const float my = static_cast<float>(mPos.y());

        // Pre-lettura canali colore fuori dal loop (evita N chiamate ai getter QColor)
        const unsigned char pc1r = static_cast<unsigned char>(pc1.red());
        const unsigned char pc1g = static_cast<unsigned char>(pc1.green());
        const unsigned char pc1b = static_cast<unsigned char>(pc1.blue());
        const unsigned char pc2r = static_cast<unsigned char>(pc2.red());
        const unsigned char pc2g = static_cast<unsigned char>(pc2.green());
        const unsigned char pc2b = static_cast<unsigned char>(pc2.blue());

        ParticleData* pData = m_particles.data();
        // OPT 5: puntatore al buffer quad — scritto qui nel worker, letto
        // in updatePaintNode via memcpy senza più loop di costruzione vertici.
        ParticleQuad* qData = m_quadsRender.data();

        for (int i = 0; i < count; ++i) {
            ParticleData& p = pData[i];

            if (p.life > 0.0f) {
                // ── Interattività col Mouse ───────────────────────────────────
                const float dx = mx - p.x;
                const float dy = my - p.y;

                if (isMouseValid && qAbs(dx) < 300 && qAbs(dy) < 300) {
                    const float distSq = dx*dx + dy*dy;
                    if (distSq < 90000.0f) {
                        const float f = 3.5f / (distSq + 100.0f);
                        p.vx += dx * f * df;
                        p.vy += dy * f * df;
                    } else {
                        p.vx *= friction; // OPT 1
                        p.vy *= friction;
                    }
                } else {
                    p.vx *= friction; // OPT 1
                    p.vy *= friction;
                }

                p.x += p.vx * df;
                p.y += p.vy * df;

                // ── Boundary Checks ───────────────────────────────────────────
                if      (p.x < 0) { p.x = 0; p.vx =  qAbs(p.vx) * 0.4f; }
                else if (p.x > w) { p.x = w; p.vx = -qAbs(p.vx) * 0.4f; }
                if      (p.y < 0) { p.y = 0; p.vy =  qAbs(p.vy) * 0.4f; }
                else if (p.y > h) { p.y = h; p.vy = -qAbs(p.vy) * 0.4f; }

                // ── Evitamento Tao ────────────────────────────────────────────
                const float tdx     = p.x - cx;
                const float tdy     = p.y - cy;
                const float tDistSq = tdx*tdx + tdy*tdy;
                if (tDistSq < rSq) {
                    // OPT 2: singola divisione invece di sqrt separata + reciproco.
                    // Calcoliamo invDist = 1/sqrt(tDistSq) in un'unica operazione,
                    // poi lo riusiamo sia per normalizzare il vettore che per il push.
                    const float tDist    = std::sqrt(tDistSq);
                    const float safeDist = (tDist < 0.1f) ? 0.1f : tDist;
                    const float invDist  = 1.0f / safeDist;
                    const float nx = tdx * invDist;
                    const float ny = tdy * invDist;

                    const float push = (r - safeDist) * 0.3f;
                    p.x += nx * push;
                    p.y += ny * push;

                    const float dot = p.vx * nx + p.vy * ny;
                    if (dot < 0) {
                        p.vx -= 1.6f * dot * nx;
                        p.vy -= 1.6f * dot * ny;
                    }
                }

                p.life -= p.decay * df;

                // ── Calcolo Colore ────────────────────────────────────────────
                const unsigned char alpha = static_cast<unsigned char>(p.life * 255.0f);
                unsigned char red, green, blue;

                if ((i % 7) == 0) {
                    red   = pc2r;
                    green = static_cast<unsigned char>(qMin(255, (int)pc2g + (int)(p.life * 50)));
                    blue  = pc2b;
                } else {
                    const float speed = std::sqrt(p.vx*p.vx + p.vy*p.vy);
                    red   = static_cast<unsigned char>(qMin(255.0f, (float)pc1r + speed * 400.0f));
                    green = static_cast<unsigned char>(qMin(255.0f, (float)pc1g + speed * 200.0f));
                    blue  = pc1b;
                }
                p.packedColor = packColor(red, green, blue, alpha);

            } else {
                // ── Respawn ───────────────────────────────────────────────────
                p.life = 1.0f;
                // OPT 3: localGen invece di QRandomGenerator::global() — nessun lock
                const double angle = localGen.generateDouble() * 6.28318;
                const double dist  = r * (0.5 + localGen.generateDouble() * 2.0);

                p.x  = cx + static_cast<float>(std::cos(angle) * dist);
                p.y  = cy + static_cast<float>(std::sin(angle) * dist);
                p.vx = static_cast<float>((localGen.generateDouble() - 0.5) * 0.6);
                p.vy = static_cast<float>((localGen.generateDouble() - 0.5) * 0.6);

                const float sdx = p.x - cx;
                const float sdy = p.y - cy;
                if (sdx*sdx + sdy*sdy < rSq)
                    p.x += (sdx > 0 ? r : -r);

                p.decay = 0.003f + static_cast<float>(localGen.generateDouble()) * 0.008f;
                p.size  = 2.0f  + static_cast<float>(localGen.generateDouble()) * 8.0f;
                p.packedColor = packColor(pc1r, pc1g, pc1b, 255);
            }

            // ── OPT 5: pre-build del quad nel thread worker ───────────────────
            // I 4 vertici del quad vengono costruiti qui, nel thread simulazione,
            // invece che in updatePaintNode nel thread GUI.
            // In updatePaintNode basterà un singolo std::memcpy contiguo,
            // che il compilatore trasforma in un'operazione SIMD ottimale.
            const float    px  = p.x;
            const float    py  = p.y;
            // p.size equivale al diametro del punto WebGL (gl_PointSize).
            // Nel quad C++ hs è il RAGGIO, quindi il quad sarebbe 2× più grande.
            // Dimezzando qui si allinea la dimensione visiva alla versione WebGL
            // senza alterare la logica di simulazione (collisioni, respawn).
            const float    hs  = p.size * 0.5f;
            const quint32  col = p.packedColor;
            ParticleQuad&  q   = qData[i];
            q.v[0] = { px - hs, py - hs, 0.0f, 0.0f, col };
            q.v[1] = { px + hs, py - hs, 1.0f, 0.0f, col };
            q.v[2] = { px - hs, py + hs, 0.0f, 1.0f, col };
            q.v[3] = { px + hs, py + hs, 1.0f, 1.0f, col };
        }
    });

    m_watcher.setFuture(future);
}

// ── setupGeometryIndices ──────────────────────────────────────────────────────

void TaoQGraphHybrid::setupGeometryIndices(QSGGeometry *geo, int count) {
    if (count <= 0) { geo->allocate(0, 0); return; }
    geo->allocate(count * 4, count * 6);
    quint32 *indices = static_cast<quint32*>(geo->indexData());
    for (int i = 0; i < count; ++i) {
        const quint32 base = i * 4;
        indices[0] = base;     indices[1] = base + 1; indices[2] = base + 2;
        indices[3] = base + 2; indices[4] = base + 1; indices[5] = base + 3;
        indices += 6;
    }
}

// ── updatePaintNode ───────────────────────────────────────────────────────────

QSGNode *TaoQGraphHybrid::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) {
    QSGNode *root = oldNode;

    const qreal dpr = window() ? window()->devicePixelRatio() : 1.0;

    if (!root) {
        root = new QSGNode();

        // 0. Nodo Particelle
        QSGGeometryNode *pNode = new QSGGeometryNode();
        QSGGeometry *pGeo = new QSGGeometry(particleAttributes(), 0, 0, QSGGeometry::UnsignedIntType);
        pGeo->setDrawingMode(QSGGeometry::DrawTriangles);
        pGeo->setIndexDataPattern(QSGGeometry::StaticPattern);
        pGeo->setVertexDataPattern(QSGGeometry::StreamPattern);
        pNode->setGeometry(pGeo);
        pNode->setFlag(QSGNode::OwnsGeometry);
        pNode->setMaterial(new ParticleMaterial());
        pNode->setFlag(QSGNode::OwnsMaterial);
        root->appendChildNode(pNode);

        // 1. Nodo Sistema (centrato)
        QSGTransformNode *systemNode = new QSGTransformNode();
        root->appendChildNode(systemNode);

        // 1a. Nodo Tao (ruotante)
        QSGTransformNode *tNode = new QSGTransformNode();
        systemNode->appendChildNode(tNode);

        QSGSimpleTextureNode *gNode1 = new QSGSimpleTextureNode();
        gNode1->setTexture(window()->createTextureFromImage(generateGlowTexture(256, m_glowColor1, dpr)));
        gNode1->setOwnsTexture(true);
        gNode1->setFiltering(QSGTexture::Linear);
        tNode->appendChildNode(gNode1);

        QSGSimpleTextureNode *gNode2 = new QSGSimpleTextureNode();
        gNode2->setTexture(window()->createTextureFromImage(generateGlowTexture(256, m_glowColor2, dpr)));
        gNode2->setOwnsTexture(true);
        gNode2->setFiltering(QSGTexture::Linear);
        tNode->appendChildNode(gNode2);

        QSGSimpleTextureNode *sNode = new QSGSimpleTextureNode();
        sNode->setTexture(window()->createTextureFromImage(generateTaoTexture(256, dpr)));
        sNode->setOwnsTexture(true);
        sNode->setFiltering(QSGTexture::Linear);
        tNode->appendChildNode(sNode);

        // 1b. Nodo Orologio
        QSGNode *cGroupNode = new QSGNode();
        systemNode->appendChildNode(cGroupNode);

        auto createHand = [&](float lineWidth, QColor col) {
            QSGGeometryNode *hn = new QSGGeometryNode();
            QSGGeometry *hg = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 2);
            hg->setLineWidth(lineWidth);
            hg->setDrawingMode(QSGGeometry::DrawLines);
            hn->setGeometry(hg);
            hn->setFlag(QSGNode::OwnsGeometry);
            QSGFlatColorMaterial *hm = new QSGFlatColorMaterial();
            hm->setColor(col);
            hn->setMaterial(hm);
            hn->setFlag(QSGNode::OwnsMaterial);
            return hn;
        };

        cGroupNode->appendChildNode(createHand(5.0f, m_hourHandColor));
        cGroupNode->appendChildNode(createHand(3.0f, m_minuteHandColor));
        cGroupNode->appendChildNode(createHand(1.5f, m_secondHandColor));

        m_lastDpr = dpr;
    }

    // ── Timing ────────────────────────────────────────────────────────────────
    const qint64 current = m_timeTracker.elapsed();
    if (m_lastTime == 0) m_lastTime = current;
    const float dt = (current - m_lastTime) / 1000.0f;
    m_lastTime = current;
    m_lastDt   = dt;

    const float dir         = m_clockwise ? 1.0f : -1.0f;
    const float speedFactor = 0.06f;
    m_rotation += (m_rotationSpeed * speedFactor * dir * dt);

    // ── Recupero nodi ─────────────────────────────────────────────────────────
    QSGGeometryNode  *pNode      = static_cast<QSGGeometryNode* >(root->childAtIndex(0));
    QSGTransformNode *systemNode = static_cast<QSGTransformNode*>(root->childAtIndex(1));
    QSGTransformNode *tNode      = static_cast<QSGTransformNode*>(systemNode->childAtIndex(0));
    QSGNode          *cGroupNode = systemNode->childAtIndex(1);

    const float w = width(), h = height();
    const float r = qMin(w, h) / 4.5f;

    QMatrix4x4 sysM;
    sysM.translate(w / 2.0f, h / 2.0f);
    systemNode->setMatrix(sysM);

    QSGSimpleTextureNode *gNode1 = static_cast<QSGSimpleTextureNode*>(tNode->childAtIndex(0));
    QSGSimpleTextureNode *gNode2 = static_cast<QSGSimpleTextureNode*>(tNode->childAtIndex(1));
    QSGSimpleTextureNode *sNode  = static_cast<QSGSimpleTextureNode*>(tNode->childAtIndex(2));

    // ── OPT 4: sostituzione texture con ownership esplicita ───────────────────
    // Dissociamo l'ownership prima di sostituire, eliminiamo la vecchia texture
    // manualmente, poi riassegnamo. Evita memory leak di oggetti GPU che
    // altrimenti rimarrebbero allocati in VRAM fino alla distruzione del nodo.
    auto safeReplaceTexture = [](QSGSimpleTextureNode *node, QSGTexture *newTex) {
        QSGTexture *old = node->texture();
        node->setOwnsTexture(false);
        node->setTexture(newTex);
        node->setOwnsTexture(true);
        delete old;
    };

    // Rigenera tutto se DPR è cambiato (spostamento su altro schermo)
    const bool dprChanged = !qFuzzyCompare(dpr, m_lastDpr);
    if (dprChanged) {
        safeReplaceTexture(sNode,  window()->createTextureFromImage(generateTaoTexture(256, dpr)));
        safeReplaceTexture(gNode1, window()->createTextureFromImage(generateGlowTexture(256, m_glowColor1, dpr)));
        safeReplaceTexture(gNode2, window()->createTextureFromImage(generateGlowTexture(256, m_glowColor2, dpr)));
        m_lastGlowColor1 = m_glowColor1;
        m_lastGlowColor2 = m_glowColor2;
        m_lastDpr = dpr;
    }

    // ── Glow 1 ────────────────────────────────────────────────────────────────
    const float gs1 = m_glowSize1;
    gNode1->setRect(gs1 > 0.01f ? QRectF(-r*gs1, -r*gs1, r*2*gs1, r*2*gs1) : QRectF());
    if (!dprChanged && m_lastGlowColor1 != m_glowColor1) {
        safeReplaceTexture(gNode1, window()->createTextureFromImage(generateGlowTexture(256, m_glowColor1, dpr)));
        m_lastGlowColor1 = m_glowColor1;
    }

    // ── Glow 2 ────────────────────────────────────────────────────────────────
    const float gs2 = m_glowSize2;
    gNode2->setRect(gs2 > 0.01f ? QRectF(-r*gs2, -r*gs2, r*2*gs2, r*2*gs2) : QRectF());
    if (!dprChanged && m_lastGlowColor2 != m_glowColor2) {
        safeReplaceTexture(gNode2, window()->createTextureFromImage(generateGlowTexture(256, m_glowColor2, dpr)));
        m_lastGlowColor2 = m_glowColor2;
    }

    sNode->setRect(-r, -r, r*2, r*2);

    // ── Orologio ──────────────────────────────────────────────────────────────
    if (m_showClock) {
        const QTime now = QTime::currentTime();
        const float ms  = now.msec() / 1000.0f;
        const float s   = (now.second() + ms) * 6.0f;
        const float m   = (now.minute() + s / 360.0f) * 6.0f;
        const float hh  = (now.hour() % 12 + m / 360.0f) * 30.0f;

        auto updateHand = [&](int idx, float angle, float len, const QColor &col) {
            QSGGeometryNode *hn  = static_cast<QSGGeometryNode*>(cGroupNode->childAtIndex(idx));
            QSGGeometry     *geo = hn->geometry();
            if (geo->vertexCount() != 2) geo->allocate(2);
            QSGGeometry::Point2D *v = geo->vertexDataAsPoint2D();
            const float rad = qDegreesToRadians(angle - 90.0f);
            v[0].x = 0;                   v[0].y = 0;
            v[1].x = std::cos(rad) * len; v[1].y = std::sin(rad) * len;
            hn->markDirty(QSGNode::DirtyGeometry);
            static_cast<QSGFlatColorMaterial*>(hn->material())->setColor(col);
            hn->markDirty(QSGNode::DirtyMaterial);
        };

        updateHand(0, hh, r * 0.5f, m_hourHandColor);
        updateHand(1, m,  r * 0.8f, m_minuteHandColor);
        updateHand(2, s,  r * 0.9f, m_secondHandColor);
    } else {
        for (int i = 0; i < 3; ++i) {
            QSGGeometryNode *hn = static_cast<QSGGeometryNode*>(cGroupNode->childAtIndex(i));
            if (hn->geometry()->vertexCount() > 0) {
                hn->geometry()->allocate(0);
                hn->markDirty(QSGNode::DirtyGeometry);
            }
        }
    }

    // ── Rotazione Tao ─────────────────────────────────────────────────────────
    QMatrix4x4 tM;
    tM.rotate(qRadiansToDegrees(m_rotation), 0, 0, 1);
    tNode->setMatrix(tM);

    // ── Particelle ────────────────────────────────────────────────────────────
    if (!m_lowCpuMode) {
        QSGGeometry *geo       = pNode->geometry();
        const int activeCount  = qMin(m_renderCount, MAX_PARTICLES);

        if (geo->vertexCount() != activeCount * 4)
            setupGeometryIndices(geo, activeCount);

        // OPT 5: singolo memcpy invece del loop di costruzione vertici.
        // activeCount * sizeof(ParticleQuad) = activeCount * 80 byte.
        // Il compilatore ottimizza memcpy in trasferimenti SIMD (SSE/AVX/NEON).
        if (activeCount > 0) {
            std::memcpy(geo->vertexData(),
                        m_quadsRender.data(),
                        static_cast<size_t>(activeCount) * sizeof(ParticleQuad));
        }

        pNode->markDirty(QSGNode::DirtyGeometry);
        updateSimulation();
    } else {
        if (pNode->geometry()->vertexCount() > 0)
            pNode->geometry()->allocate(0, 0);
    }

    return root;
}

// ── generateGlowTexture ───────────────────────────────────────────────────────
// Genera una texture radiale per l'effetto glow, HiDPI-aware.
// La texture fisica è (s * dpr)² pixel; Qt Quick la scala al rect logico
// automaticamente tramite il campo devicePixelRatio della QImage.
QImage TaoQGraphHybrid::generateGlowTexture(int s, const QColor &color, qreal dpr) {
    const int phys = qRound(s * dpr);
    QImage img(phys, phys, QImage::Format_ARGB32_Premultiplied);
    img.setDevicePixelRatio(dpr);
    img.fill(Qt::transparent);

    QPainter p(&img);
    QRadialGradient g(phys / 2.0, phys / 2.0, phys / 2.0);
    g.setColorAt(0.0, color);
    QColor fade = color;
    fade.setAlpha(color.alpha() * 0.3);
    g.setColorAt(0.7, fade);
    g.setColorAt(1.0, Qt::transparent);
    p.setBrush(g);
    p.setPen(Qt::NoPen);
    p.drawEllipse(0, 0, phys, phys);
    return img;
}

// ── generateTaoTexture ────────────────────────────────────────────────────────
// Genera il simbolo Tao (yin-yang) come QImage HiDPI-aware.
// Tutta la geometria è calcolata in coordinate fisiche (phys = s * dpr),
// producendo curve perfettamente lisce su qualsiasi densità di pixel.
QImage TaoQGraphHybrid::generateTaoTexture(int s, qreal dpr) {
    const int phys = qRound(s * dpr);
    QImage img(phys, phys, QImage::Format_ARGB32_Premultiplied);
    img.setDevicePixelRatio(dpr);
    img.fill(Qt::transparent);

    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);

    const float c = phys / 2.0f;
    const float r = c - 2.0f * static_cast<float>(dpr);

    p.setPen(Qt::NoPen);

    p.setBrush(Qt::white);
    p.drawEllipse(QPointF(c, c), r, r);

    p.setBrush(Qt::black);
    p.drawPie(QRectF(c - r, c - r, r * 2, r * 2), 90 * 16, 180 * 16);

    p.setBrush(Qt::white);
    p.drawEllipse(QPointF(c, c + r / 2), r / 2, r / 2);

    p.setBrush(Qt::black);
    p.drawEllipse(QPointF(c, c - r / 2), r / 2, r / 2);

    p.setBrush(Qt::black);
    p.drawEllipse(QPointF(c, c + r / 2), r / 6, r / 6);

    p.setBrush(Qt::white);
    p.drawEllipse(QPointF(c, c - r / 2), r / 6, r / 6);

    return img;
}