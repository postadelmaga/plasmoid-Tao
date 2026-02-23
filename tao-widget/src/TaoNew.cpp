#include "TaoNew.h"
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

class ParticleMaterialShaderNew : public QSGMaterialShader
{
public:
    ParticleMaterialShaderNew()
    {
        const QString dir  = soDir();
        const QString vert = dir + QStringLiteral("/shaders/particle.vert.qsb");
        const QString frag = dir + QStringLiteral("/shaders/particle.frag.qsb");
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

// ── ParticleMaterialNew ───────────────────────────────────────────────────────
static QSGMaterialType particleMaterialTypeNew;

ParticleMaterialNew::ParticleMaterialNew()
{
    setFlag(Blending, true);
    setFlag(RequiresFullMatrix, true);
}

QSGMaterialType *ParticleMaterialNew::type() const { return &particleMaterialTypeNew; }

QSGMaterialShader *ParticleMaterialNew::createShader(QSGRendererInterface::RenderMode) const
{
    return new ParticleMaterialShaderNew();
}

// ── Helpers ───────────────────────────────────────────────────────────────────

inline quint32 packColorNew(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    float af = a / 255.0f;
    unsigned char pr = static_cast<unsigned char>(r * af);
    unsigned char pg = static_cast<unsigned char>(g * af);
    unsigned char pb = static_cast<unsigned char>(b * af);
    return (quint32(a) << 24) | (quint32(pb) << 16) | (quint32(pg) << 8) | quint32(pr);
}

static const QSGGeometry::AttributeSet &particleAttributesNew() {
    static QSGGeometry::Attribute data[] = {
        QSGGeometry::Attribute::create(0, 2, QSGGeometry::FloatType, false),
        QSGGeometry::Attribute::create(1, 2, QSGGeometry::FloatType),
        QSGGeometry::Attribute::create(2, 4, QSGGeometry::UnsignedByteType, true)
    };
    static QSGGeometry::AttributeSet attrs = { 3, 20, data };
    return attrs;
}

// ── Costruttore / Distruttore ─────────────────────────────────────────────────

TaoNew::TaoNew(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
    m_particles.resize(MAX_PARTICLES);
    m_particlesRender.resize(MAX_PARTICLES);
    m_quadsRender.resize(MAX_PARTICLES);

    std::memset(m_particles.data(), 0, sizeof(ParticleData) * MAX_PARTICLES);

    connect(&m_watcher, &QFutureWatcher<void>::finished, this, [this]() {
        if (m_particleCount > 0) {
            std::memcpy(m_particlesRender.data(), m_particles.data(),
                        m_particleCount * sizeof(ParticleData));
        }
        // OPT 6: copia il contatore attivi prodotto dal worker
        m_renderActiveCount = m_pendingActiveCount.load();
        m_simulationPending = false;
        update();
    });

    m_timeTracker.start();
}

TaoNew::~TaoNew() {
    if (m_watcher.isRunning())
        m_watcher.waitForFinished();
}

// ── Setters ───────────────────────────────────────────────────────────────────

void TaoNew::setParticleCount(int count) {
    if (m_particleCount == count) return;
    m_particleCount = qBound(0, count, MAX_PARTICLES);
    Q_EMIT particleCountChanged();
}

void TaoNew::setRotationSpeed(float speed) {
    if (qFuzzyCompare(m_rotationSpeed, speed)) return;
    m_rotationSpeed = speed;
    Q_EMIT rotationSpeedChanged();
}

void TaoNew::setClockwise(bool clockwise) {
    if (m_clockwise == clockwise) return;
    m_clockwise = clockwise;
    Q_EMIT clockwiseChanged();
}

void TaoNew::setShowClock(bool show) {
    if (m_showClock == show) return;
    m_showClock = show;
    Q_EMIT showClockChanged();
    update();
}

void TaoNew::setHourHandColor(const QColor &c) {
    if (m_hourHandColor == c) return;
    m_hourHandColor = c;
    Q_EMIT hourHandColorChanged();
    update();
}

void TaoNew::setMinuteHandColor(const QColor &c) {
    if (m_minuteHandColor == c) return;
    m_minuteHandColor = c;
    Q_EMIT minuteHandColorChanged();
    update();
}

void TaoNew::setSecondHandColor(const QColor &c) {
    if (m_secondHandColor == c) return;
    m_secondHandColor = c;
    Q_EMIT secondHandColorChanged();
    update();
}

void TaoNew::setGlowColor1(const QColor &c) {
    if (m_glowColor1 == c) return;
    m_glowColor1 = c;
    Q_EMIT glowColor1Changed();
    update();
}

void TaoNew::setGlowSize1(double s) {
    if (qFuzzyCompare(m_glowSize1, s)) return;
    m_glowSize1 = s;
    Q_EMIT glowSize1Changed();
    update();
}

void TaoNew::setGlowColor2(const QColor &c) {
    if (m_glowColor2 == c) return;
    m_glowColor2 = c;
    Q_EMIT glowColor2Changed();
    update();
}

void TaoNew::setGlowSize2(double s) {
    if (qFuzzyCompare(m_glowSize2, s)) return;
    m_glowSize2 = s;
    Q_EMIT glowSize2Changed();
    update();
}

void TaoNew::setParticleColor1(const QColor &c) {
    if (m_particleColor1 == c) return;
    m_particleColor1 = c;
    Q_EMIT particleColor1Changed();
    update();
}

void TaoNew::setParticleColor2(const QColor &c) {
    if (m_particleColor2 == c) return;
    m_particleColor2 = c;
    Q_EMIT particleColor2Changed();
    update();
}

void TaoNew::setMousePos(const QPointF &pos) {
    if (m_mousePos == pos) return;
    m_mousePos = pos;
    Q_EMIT mousePosChanged();
}

void TaoNew::itemChange(ItemChange change, const ItemChangeData &value) {
    if (change == ItemVisibleHasChanged && value.boolValue)
        update();
    QQuickItem::itemChange(change, value);
}

// ── updateSimulation ──────────────────────────────────────────────────────────

void TaoNew::updateSimulation() {
    if (m_simulationPending) return;
    m_simulationPending = true;

    const int count      = m_particleCount;
    if (count <= 0) {
        m_renderActiveCount = 0;
        m_pendingActiveCount.store(0);
        m_simulationPending = false;
        update();
        return;
    }

    const float   w        = width();
    const float   h        = height();
    const QPointF mPos     = m_mousePos;
    const float currentDt  = (m_lastDt > 0.001f && m_lastDt < 1.0f) ? m_lastDt : 0.016f;
    const QColor  pc1      = m_particleColor1;
    const QColor  pc2      = m_particleColor2;

    QFuture<void> future = QtConcurrent::run([this, count, w, h, mPos, currentDt, pc1, pc2]() {
        if (count <= 0) return;

        const float cx  = w * 0.5f;
        const float cy  = h * 0.5f;
        const float r   = qMin(w, h) / 4.5f;
        const float rSq = r * r;
        const float df  = currentDt * 60.0f;

        // OPT 1: friction pre-calcolato una volta fuori dal loop
        const float friction = std::pow(0.98f, df);

        // OPT 3: generatore locale, nessun lock su global()
        QRandomGenerator localGen(QRandomGenerator::global()->generate());

        const bool  isMouseValid = (mPos.x() >= 0 && mPos.y() >= 0 &&
                                    mPos.x() <= w  && mPos.y() <= h);
        const float mx = static_cast<float>(mPos.x());
        const float my = static_cast<float>(mPos.y());

        const unsigned char pc1r = static_cast<unsigned char>(pc1.red());
        const unsigned char pc1g = static_cast<unsigned char>(pc1.green());
        const unsigned char pc1b = static_cast<unsigned char>(pc1.blue());
        const unsigned char pc2r = static_cast<unsigned char>(pc2.red());
        const unsigned char pc2g = static_cast<unsigned char>(pc2.green());
        const unsigned char pc2b = static_cast<unsigned char>(pc2.blue());

        ParticleData* pData = m_particles.data();
        ParticleQuad* qData = m_quadsRender.data();

        // OPT 6: indice compatto nel buffer quad — solo particelle vive
        int activeCount = 0;

        for (int i = 0; i < count; ++i) {
            ParticleData& p = pData[i];

            if (p.life > 0.0f) {
                const float dx = mx - p.x;
                const float dy = my - p.y;

                if (isMouseValid && qAbs(dx) < 300 && qAbs(dy) < 300) {
                    const float distSq = dx*dx + dy*dy;
                    if (distSq < 90000.0f) {
                        const float f = 3.5f / (distSq + 100.0f);
                        p.vx += dx * f * df;
                        p.vy += dy * f * df;
                    } else {
                        p.vx *= friction;
                        p.vy *= friction;
                    }
                } else {
                    p.vx *= friction;
                    p.vy *= friction;
                }

                p.x += p.vx * df;
                p.y += p.vy * df;

                if      (p.x < 0) { p.x = 0; p.vx =  qAbs(p.vx) * 0.4f; }
                else if (p.x > w) { p.x = w; p.vx = -qAbs(p.vx) * 0.4f; }
                if      (p.y < 0) { p.y = 0; p.vy =  qAbs(p.vy) * 0.4f; }
                else if (p.y > h) { p.y = h; p.vy = -qAbs(p.vy) * 0.4f; }

                const float tdx     = p.x - cx;
                const float tdy     = p.y - cy;
                const float tDistSq = tdx*tdx + tdy*tdy;
                if (tDistSq < rSq) {
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

                // OPT 4: speedSq invece di sqrt — stesso effetto visivo,
                // fattori ricalibrati per preservare la luminosità media
                const unsigned char alpha = static_cast<unsigned char>(p.life * 255.0f);
                unsigned char red, green, blue;

                if ((i % 7) == 0) {
                    red   = pc2r;
                    green = static_cast<unsigned char>(qMin(255, (int)pc2g + (int)(p.life * 50)));
                    blue  = pc2b;
                } else {
                    const float speedSq = p.vx*p.vx + p.vy*p.vy;
                    red   = static_cast<unsigned char>(qMin(255.0f, (float)pc1r + speedSq * 20.0f));
                    green = static_cast<unsigned char>(qMin(255.0f, (float)pc1g + speedSq * 10.0f));
                    blue  = pc1b;
                }
                p.packedColor = packColorNew(red, green, blue, alpha);

            } else {
                // Respawn
                p.life = 1.0f;
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
                p.packedColor = packColorNew(pc1r, pc1g, pc1b, 255);
            }

            // OPT 5+6: quad scritto in posizione compatta [activeCount]
            const float   px  = p.x;
            const float   py  = p.y;
            const float   hs  = p.size * 0.5f;
            const quint32 col = p.packedColor;
            ParticleQuad& q   = qData[activeCount];
            q.v[0] = { px - hs, py - hs, 0.0f, 0.0f, col };
            q.v[1] = { px + hs, py - hs, 1.0f, 0.0f, col };
            q.v[2] = { px - hs, py + hs, 0.0f, 1.0f, col };
            q.v[3] = { px + hs, py + hs, 1.0f, 1.0f, col };
            ++activeCount;
        }

        m_pendingActiveCount.store(activeCount);
    });

    m_watcher.setFuture(future);
}

// ── setupGeometryIndices ──────────────────────────────────────────────────────
// OPT 1: chiamata UNA sola volta alla creazione del nodo con MAX_PARTICLES.
// Gli indici non cambiano mai → StaticPattern, nessun re-upload GPU.
void TaoNew::setupGeometryIndices(QSGGeometry *geo, int count) {
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

QSGNode *TaoNew::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) {
    QSGNode *root = oldNode;
    const qreal dpr = window() ? window()->devicePixelRatio() : 1.0;

    if (!root) {
        root = new QSGNode();

        // Nodo Particelle
        QSGGeometryNode *pNode = new QSGGeometryNode();
        QSGGeometry *pGeo = new QSGGeometry(particleAttributesNew(), 0, 0, QSGGeometry::UnsignedIntType);
        pGeo->setDrawingMode(QSGGeometry::DrawTriangles);
        // OPT 1: indici statici, vertici dinamici
        pGeo->setIndexDataPattern(QSGGeometry::StaticPattern);
        pGeo->setVertexDataPattern(QSGGeometry::DynamicPattern);
        pNode->setGeometry(pGeo);
        pNode->setFlag(QSGNode::OwnsGeometry);
        pNode->setMaterial(new ParticleMaterialNew());
        pNode->setFlag(QSGNode::OwnsMaterial);
        root->appendChildNode(pNode);

        // OPT 1: alloca indici al massimo UNA SOLA VOLTA
        setupGeometryIndices(pGeo, MAX_PARTICLES);

        QSGTransformNode *systemNode = new QSGTransformNode();
        root->appendChildNode(systemNode);

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

    auto safeReplaceTexture = [](QSGSimpleTextureNode *node, QSGTexture *newTex) {
        QSGTexture *old = node->texture();
        node->setOwnsTexture(false);
        node->setTexture(newTex);
        node->setOwnsTexture(true);
        delete old;
    };

    const bool dprChanged = !qFuzzyCompare(dpr, m_lastDpr);
    if (dprChanged) {
        safeReplaceTexture(sNode,  window()->createTextureFromImage(generateTaoTexture(256, dpr)));
        safeReplaceTexture(gNode1, window()->createTextureFromImage(generateGlowTexture(256, m_glowColor1, dpr)));
        safeReplaceTexture(gNode2, window()->createTextureFromImage(generateGlowTexture(256, m_glowColor2, dpr)));
        m_lastGlowColor1 = m_glowColor1;
        m_lastGlowColor2 = m_glowColor2;
        m_lastDpr = dpr;
    }

    const float gs1 = m_glowSize1;
    gNode1->setRect(gs1 > 0.01f ? QRectF(-r*gs1, -r*gs1, r*2*gs1, r*2*gs1) : QRectF());
    if (!dprChanged && m_lastGlowColor1 != m_glowColor1) {
        safeReplaceTexture(gNode1, window()->createTextureFromImage(generateGlowTexture(256, m_glowColor1, dpr)));
        m_lastGlowColor1 = m_glowColor1;
    }

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

        // OPT 5: markDirty(Material) solo se il colore è cambiato
        auto updateHand = [&](int idx, float angle, float len, const QColor &col) {
            QSGGeometryNode *hn  = static_cast<QSGGeometryNode*>(cGroupNode->childAtIndex(idx));
            QSGGeometry     *geo = hn->geometry();
            if (geo->vertexCount() != 2) geo->allocate(2);
            QSGGeometry::Point2D *v = geo->vertexDataAsPoint2D();
            const float rad = qDegreesToRadians(angle - 90.0f);
            v[0].x = 0;                   v[0].y = 0;
            v[1].x = std::cos(rad) * len; v[1].y = std::sin(rad) * len;
            hn->markDirty(QSGNode::DirtyGeometry);
            QSGFlatColorMaterial *mat = static_cast<QSGFlatColorMaterial*>(hn->material());
            if (mat->color() != col) {
                mat->setColor(col);
                hn->markDirty(QSGNode::DirtyMaterial);
            }
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
    QSGGeometry *geo = pNode->geometry();

    // OPT 3+6: markDirty solo se ci sono dati nuovi e la simulazione è completata.
    // Usa m_renderActiveCount (solo particelle vive) per il memcpy — buffer compatto.
    const int activeCount = qMin(m_renderActiveCount, MAX_PARTICLES);

    if (activeCount > 0 && !m_simulationPending) {
        std::memcpy(geo->vertexData(),
                    m_quadsRender.data(),
                    static_cast<size_t>(activeCount) * sizeof(ParticleQuad));
        pNode->markDirty(QSGNode::DirtyGeometry);
    }

    updateSimulation();
    return root;
}

// ── generateGlowTexture ───────────────────────────────────────────────────────
QImage TaoNew::generateGlowTexture(int s, const QColor &color, qreal dpr) {
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
QImage TaoNew::generateTaoTexture(int s, qreal dpr) {
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
