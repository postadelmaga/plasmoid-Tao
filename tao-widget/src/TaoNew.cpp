#include "TaoNew.h"

#include <QSGGeometryNode>
#include <QSGSimpleTextureNode>
#include <QSGFlatColorMaterial>
#include <QSGTransformNode>
#include <QQuickWindow>
#include <QPainter>
#include <QRadialGradient>
#include <QFileInfo>
#include <QRandomGenerator>
#include <QtConcurrent>
#include <QtMath>
#include <QTime>
#include <dlfcn.h>
#include <cstring>
#include <cmath>

// ═════════════════════════════════════════════════════════════════════════════
// Helpers interni (file-scope)
// ═════════════════════════════════════════════════════════════════════════════

static QString shaderDirectory()
{
    Dl_info info;
    if (dladdr(reinterpret_cast<void*>(&shaderDirectory), &info) && info.dli_fname)
        return QFileInfo(QString::fromUtf8(info.dli_fname)).absolutePath();
    return {};
}

// Premoltiplica l'alpha e impacchetta RGBA in un quint32 (formato GPU).
static inline quint32 packColor(unsigned char r, unsigned char g,
                                unsigned char b, unsigned char a)
{
    const float af = a / 255.0f;
    const auto pr  = static_cast<unsigned char>(r * af);
    const auto pg  = static_cast<unsigned char>(g * af);
    const auto pb  = static_cast<unsigned char>(b * af);
    return (quint32(a) << 24) | (quint32(pb) << 16) | (quint32(pg) << 8) | quint32(pr);
}

// Attributi geometria: pos(xy) + size(float) + color(4×ubyte) = 16 byte/vertice.
static const QSGGeometry::AttributeSet &particleAttributes()
{
    static QSGGeometry::Attribute data[] = {
        QSGGeometry::Attribute::create(0, 2, QSGGeometry::FloatType,        false), // pos
        QSGGeometry::Attribute::create(1, 1, QSGGeometry::FloatType               ), // size
        QSGGeometry::Attribute::create(2, 4, QSGGeometry::UnsignedByteType,  true ), // color
    };
    static QSGGeometry::AttributeSet attrs = { 3, 16, data };
    return attrs;
}

// ═════════════════════════════════════════════════════════════════════════════
// ParticleMaterialShader
// ═════════════════════════════════════════════════════════════════════════════

class ParticleMaterialShader : public QSGMaterialShader
{
public:
    ParticleMaterialShader()
    {
        const QString dir = shaderDirectory();
        setShaderFileName(VertexStage,   dir + QStringLiteral("/shaders/particle.vert.qsb"));
        setShaderFileName(FragmentStage, dir + QStringLiteral("/shaders/particle.frag.qsb"));
        setFlag(UpdatesGraphicsPipelineState, true);
    }

    // Additive blending: la GPU somma i colori invece di calcolare trasparenze
    // sovrapposte → zero overdraw, particelle luminose senza artefatti.
    bool updateGraphicsPipelineState(RenderState &state, GraphicsPipelineState *ps,
                                     QSGMaterial *newMat, QSGMaterial *oldMat) override
    {
        QSGMaterialShader::updateGraphicsPipelineState(state, ps, newMat, oldMat);

        ps->blendEnable          = true;
        ps->srcColor             = GraphicsPipelineState::SrcAlpha;
        ps->dstColor             = GraphicsPipelineState::One;   // ← additive
        ps->separateBlendFactors = true;                          // richiede Qt 6.5+
        ps->srcAlpha             = GraphicsPipelineState::One;
        ps->dstAlpha             = GraphicsPipelineState::One;

        return true;
    }

    bool updateUniformData(RenderState &state, QSGMaterial *newMat,
                           QSGMaterial * /*oldMat*/) override
    {
        Q_UNUSED(newMat)
        bool changed = false;
        QByteArray *buf = state.uniformData();

        if (state.isMatrixDirty()) {
            memcpy(buf->data(),      state.combinedMatrix().constData(), 64);
            changed = true;
        }
        if (state.isOpacityDirty()) {
            const float op = state.opacity();
            memcpy(buf->data() + 64, &op, 4);
            changed = true;
        }
        return changed;
    }
};

// ═════════════════════════════════════════════════════════════════════════════
// ParticleMaterial
// ═════════════════════════════════════════════════════════════════════════════

static QSGMaterialType particleMaterialType;

ParticleMaterial::ParticleMaterial()
{
    setFlag(Blending);
    setFlag(RequiresFullMatrix);
}

QSGMaterialType   *ParticleMaterial::type() const { return &particleMaterialType; }
QSGMaterialShader *ParticleMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new ParticleMaterialShader();
}

// ═════════════════════════════════════════════════════════════════════════════
// TaoNew — costruttore / distruttore
// ═════════════════════════════════════════════════════════════════════════════

TaoNew::TaoNew(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);

    m_particles.resize(MAX_PARTICLES);
    m_verticesRender.resize(MAX_PARTICLES);
    std::memset(m_particles.data(), 0, sizeof(ParticleData) * MAX_PARTICLES);

    connect(&m_watcher, &QFutureWatcher<void>::finished, this, [this]() {
        m_renderActiveCount = m_pendingActiveCount.load();
        m_simulationPending = false;
        update();
    });

    m_timeTracker.start();
}

TaoNew::~TaoNew()
{
    if (m_watcher.isRunning())
        m_watcher.waitForFinished();
}

// ═════════════════════════════════════════════════════════════════════════════
// Setters
// ═════════════════════════════════════════════════════════════════════════════

void TaoNew::setParticleCount(int count) {
    const int bounded = qBound(0, count, MAX_PARTICLES);
    if (m_particleCount == bounded) return;
    m_particleCount = bounded;
    Q_EMIT particleCountChanged();
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

void TaoNew::setParticleSize(double s) {
    if (qFuzzyCompare(m_particleSize, s)) return;
    m_particleSize = s;
    Q_EMIT particleSizeChanged();
    update();
}

void TaoNew::setParticleSizeRandom(double s) {
    if (qFuzzyCompare(m_particleSizeRandom, s)) return;
    m_particleSizeRandom = s;
    Q_EMIT particleSizeRandomChanged();
    update();
}

void TaoNew::setMousePos(const QPointF &pos) {
    if (m_mousePos == pos) return;
    m_mousePos = pos;
    Q_EMIT mousePosChanged();
}

// ═════════════════════════════════════════════════════════════════════════════
// itemChange
// ═════════════════════════════════════════════════════════════════════════════

void TaoNew::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == ItemVisibleHasChanged && value.boolValue)
        update();
    QQuickItem::itemChange(change, value);
}

// ═════════════════════════════════════════════════════════════════════════════
// updateSimulation  (asincrono, worker thread)
// ═════════════════════════════════════════════════════════════════════════════

void TaoNew::updateSimulation()
{
    if (m_simulationPending) return;
    m_simulationPending = true;

    const int count = m_particleCount;

    if (count <= 0) {
        for (int i = 0; i < MAX_PARTICLES; ++i)
            m_verticesRender[i].size = 0.0f;
        m_renderActiveCount = 0;
        m_pendingActiveCount.store(0);
        m_simulationPending = false;
        update();
        return;
    }

    // Snapshot dei parametri necessari al worker — nessun accesso a `this`
    // dentro la lambda eccetto per i buffer che sono stabili per tutta la vita
    // dell'oggetto e non vengono riallocati durante la simulazione.
    const float   w          = width();
    const float   h          = height();
    const QPointF mPos       = m_mousePos;
    const float   dt         = (m_lastDt > 0.001f && m_lastDt < 1.0f) ? m_lastDt : 0.016f;
    const QColor  pc1        = m_particleColor1;
    const QColor  pc2        = m_particleColor2;
    const float   pSize      = static_cast<float>(m_particleSize);
    const float   pSizeRand  = static_cast<float>(m_particleSizeRandom);
    const float   dpr        = window() ? static_cast<float>(window()->devicePixelRatio()) : 1.0f;

    QFuture<void> future = QtConcurrent::run([this, count, w, h, mPos, dt, pc1, pc2, pSize, pSizeRand, dpr]()
    {
        const float cx  = w * 0.5f;
        const float cy  = h * 0.5f;
        const float r   = qMin(w, h) / 4.5f;
        const float rSq = r * r;
        const float df  = dt * 60.0f;

        // Friction pre-calcolata fuori dal loop
        const float friction = std::pow(0.98f, df);

        // Generatore casuale locale → nessun lock sul generatore globale
        QRandomGenerator rng(QRandomGenerator::global()->generate());

        const bool  mouseValid = (mPos.x() >= 0 && mPos.x() <= w &&
                                  mPos.y() >= 0 && mPos.y() <= h);
        const float mx = static_cast<float>(mPos.x());
        const float my = static_cast<float>(mPos.y());

        // Canali colore estratti una volta per tutte
        const auto pc1r = static_cast<unsigned char>(pc1.red());
        const auto pc1g = static_cast<unsigned char>(pc1.green());
        const auto pc1b = static_cast<unsigned char>(pc1.blue());
        const auto pc2r = static_cast<unsigned char>(pc2.red());
        const auto pc2g = static_cast<unsigned char>(pc2.green());
        const auto pc2b = static_cast<unsigned char>(pc2.blue());

        ParticleData   *pData = m_particles.data();
        ParticleVertex *vData = m_verticesRender.data();

        // Buffer fisso: le particelle "morte" ricevono size=0 e vengono
        // scartate dalla GPU senza alcuna riallocazione del buffer driver.
        for (int i = 0; i < count; ++i)
        {
            ParticleData   &p = pData[i];
            ParticleVertex &v = vData[i];

            if (p.life > 0.0f)
            {
                // ── Interazione mouse ──────────────────────────────────────
                const float dx = mx - p.x;
                const float dy = my - p.y;

                if (mouseValid && qAbs(dx) < 300.0f && qAbs(dy) < 300.0f) {
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

                // ── Integrazione posizione ─────────────────────────────────
                p.x += p.vx * df;
                p.y += p.vy * df;

                // Rimbalzo sui bordi
                if      (p.x < 0) { p.x = 0; p.vx =  qAbs(p.vx) * 0.4f; }
                else if (p.x > w) { p.x = w; p.vx = -qAbs(p.vx) * 0.4f; }
                if      (p.y < 0) { p.y = 0; p.vy =  qAbs(p.vy) * 0.4f; }
                else if (p.y > h) { p.y = h; p.vy = -qAbs(p.vy) * 0.4f; }

                // ── Collisione con il cerchio Tao ──────────────────────────
                const float tdx     = p.x - cx;
                const float tdy     = p.y - cy;
                const float tDistSq = tdx*tdx + tdy*tdy;
                if (tDistSq < rSq) {
                    const float tDist   = std::sqrt(tDistSq);
                    const float safeDist = (tDist < 0.1f) ? 0.1f : tDist;
                    const float inv     = 1.0f / safeDist;
                    const float nx      = tdx * inv;
                    const float ny      = tdy * inv;
                    const float push    = (r - safeDist) * 0.3f;
                    p.x += nx * push;
                    p.y += ny * push;
                    const float dot = p.vx * nx + p.vy * ny;
                    if (dot < 0) {
                        p.vx -= 1.6f * dot * nx;
                        p.vy -= 1.6f * dot * ny;
                    }
                }

                // ── Aging e colore ─────────────────────────────────────────
                p.life -= p.decay * df;

                const auto alpha = static_cast<unsigned char>(p.life * 255.0f * 0.85f);
                unsigned char red, green, blue;

                if ((i % 7) == 0) {
                    // Colore secondario: variazione in base alla vita residua
                    red   = pc2r;
                    green = static_cast<unsigned char>(qMin(255, (int)pc2g + (int)(p.life * 50)));
                    blue  = pc2b;
                } else {
                    // Colore primario: shift warm in base alla velocità
                    const float speedSq = p.vx*p.vx + p.vy*p.vy;
                    const float speed   = (speedSq > 0.0001f) ? std::sqrt(speedSq) : 0.0f;
                    red   = static_cast<unsigned char>(qMin(255.0f, (float)pc1r + speed * 400.0f));
                    green = static_cast<unsigned char>(qMin(255.0f, (float)pc1g + speed * 200.0f));
                    blue  = pc1b;
                }
                p.packedColor = packColor(red, green, blue, alpha);

                v.x     = p.x;
                v.y     = p.y;
                v.size  = p.size * dpr;   // scala per HiDPI/Retina
                v.color = p.packedColor;
            }
            else
            {
                // ── Respawn ────────────────────────────────────────────────
                p.life = 1.0f;
                const double angle = rng.generateDouble() * 6.28318;
                const double dist  = r * (0.5 + rng.generateDouble() * 2.0);
                p.x  = cx + static_cast<float>(std::cos(angle) * dist);
                p.y  = cy + static_cast<float>(std::sin(angle) * dist);
                p.vx = static_cast<float>((rng.generateDouble() - 0.5) * 0.6);
                p.vy = static_cast<float>((rng.generateDouble() - 0.5) * 0.6);

                // Sposta fuori dal cerchio se ci è finita dentro
                const float sdx = p.x - cx;
                const float sdy = p.y - cy;
                if (sdx*sdx + sdy*sdy < rSq)
                    p.x += (sdx > 0 ? r : -r);

                p.decay = 0.003f + static_cast<float>(rng.generateDouble()) * 0.008f;
                // Raggio personalizzabile
                p.size  = pSize + static_cast<float>(rng.generateDouble()) * pSizeRand;
                p.packedColor = packColor(pc1r, pc1g, pc1b, static_cast<unsigned char>(255 * 0.85f));

                // Frame invisibile per il respawn: evita pop visivi
                v.x = p.x; v.y = p.y; v.size = 0.0f; v.color = p.packedColor;
            }
        }

        // Nasconde le particelle oltre il contatore attivo corrente
        for (int i = count; i < MAX_PARTICLES; ++i)
            vData[i].size = 0.0f;

        m_pendingActiveCount.store(count);
    });

    m_watcher.setFuture(future);
}

// ═════════════════════════════════════════════════════════════════════════════
// updatePaintNode
// ═════════════════════════════════════════════════════════════════════════════

QSGNode *TaoNew::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    QSGNode *root    = oldNode;
    const qreal dpr  = window() ? window()->devicePixelRatio() : 1.0;

    // ── Creazione albero nodi (eseguita una sola volta) ────────────────────────
    if (!root) {
        root = new QSGNode();

        // Particelle
        m_particleNode = new QSGGeometryNode();
        auto *pGeo = new QSGGeometry(particleAttributes(), 0, 0);
        pGeo->setDrawingMode(QSGGeometry::DrawPoints);
        pGeo->setVertexDataPattern(QSGGeometry::StreamPattern);
        m_particleNode->setGeometry(pGeo);
        m_particleNode->setFlag(QSGNode::OwnsGeometry);
        m_particleNode->setMaterial(new ParticleMaterial());
        m_particleNode->setFlag(QSGNode::OwnsMaterial);
        root->appendChildNode(m_particleNode);

        // Sistema (traslazione al centro)
        m_systemNode = new QSGTransformNode();
        root->appendChildNode(m_systemNode);

        // Rotazione Tao
        m_taoRotNode = new QSGTransformNode();
        m_systemNode->appendChildNode(m_taoRotNode);

        // Glow 1
        m_glowNode1 = new QSGSimpleTextureNode();
        m_glowNode1->setTexture(window()->createTextureFromImage(generateGlowTexture(256, m_glowColor1, dpr)));
        m_glowNode1->setOwnsTexture(true);
        m_glowNode1->setFiltering(QSGTexture::Linear);
        m_taoRotNode->appendChildNode(m_glowNode1);

        // Glow 2
        m_glowNode2 = new QSGSimpleTextureNode();
        m_glowNode2->setTexture(window()->createTextureFromImage(generateGlowTexture(256, m_glowColor2, dpr)));
        m_glowNode2->setOwnsTexture(true);
        m_glowNode2->setFiltering(QSGTexture::Linear);
        m_taoRotNode->appendChildNode(m_glowNode2);

        // Tao
        m_taoNode = new QSGSimpleTextureNode();
        m_taoNode->setTexture(window()->createTextureFromImage(generateTaoTexture(1024, dpr)));
        m_taoNode->setOwnsTexture(true);
        m_taoNode->setFiltering(QSGTexture::Linear);
        m_taoRotNode->appendChildNode(m_taoNode);

        // Lancette orologio
        m_clockGroup = new QSGNode();
        m_systemNode->appendChildNode(m_clockGroup);

        auto createHand = [&](float lineWidth, const QColor &col) {
            auto *hn = new QSGGeometryNode();
            auto *hg = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 2);
            hg->setLineWidth(lineWidth);
            hg->setDrawingMode(QSGGeometry::DrawLines);
            hn->setGeometry(hg);
            hn->setFlag(QSGNode::OwnsGeometry);
            auto *hm = new QSGFlatColorMaterial();
            hm->setColor(col);
            hn->setMaterial(hm);
            hn->setFlag(QSGNode::OwnsMaterial);
            return hn;
        };
        m_clockGroup->appendChildNode(createHand(5.0f, m_hourHandColor));
        m_clockGroup->appendChildNode(createHand(3.0f, m_minuteHandColor));
        m_clockGroup->appendChildNode(createHand(1.5f, m_secondHandColor));

        m_lastDpr = dpr;
    }

    // ── Timing ────────────────────────────────────────────────────────────────
    const qint64 now = m_timeTracker.elapsed();
    if (m_lastTime == 0) m_lastTime = now;
    m_lastDt   = (now - m_lastTime) / 1000.0f;
    m_lastTime = now;

    // ── Rotazione ─────────────────────────────────────────────────────────────
    const float dir = m_clockwise ? 1.0f : -1.0f;
    m_rotation += m_rotationSpeed * 0.06f * dir * m_lastDt;

    // ── Layout ────────────────────────────────────────────────────────────────
    const float w = width();
    const float h = height();
    const float r = qMin(w, h) / 4.5f;

    QMatrix4x4 sysM;
    sysM.translate(w * 0.5f, h * 0.5f);
    m_systemNode->setMatrix(sysM);

    // ── Texture sostituzione sicura ───────────────────────────────────────────
    // N.B.: la texture precedente viene eliminata qui, lato render thread,
    // dove il driver ha già completato il frame che la usava.
    auto replaceTexture = [this](QSGSimpleTextureNode *node, QSGTexture *newTex) {
        QSGTexture *old = node->texture();
        node->setOwnsTexture(false);
        node->setTexture(newTex);
        node->setOwnsTexture(true);
        delete old;
    };

    const bool dprChanged = !qFuzzyCompare(dpr, m_lastDpr);
    if (dprChanged) {
        replaceTexture(m_taoNode,   window()->createTextureFromImage(generateTaoTexture(1024, dpr)));
        replaceTexture(m_glowNode1, window()->createTextureFromImage(generateGlowTexture(256, m_glowColor1, dpr)));
        replaceTexture(m_glowNode2, window()->createTextureFromImage(generateGlowTexture(256, m_glowColor2, dpr)));
        m_lastGlowColor1 = m_glowColor1;
        m_lastGlowColor2 = m_glowColor2;
        m_lastDpr = dpr;
    }

    // Aggiorna glow 1
    {
        const float gs = static_cast<float>(m_glowSize1);
        m_glowNode1->setRect(gs > 0.01f ? QRectF(-r*gs, -r*gs, r*2*gs, r*2*gs) : QRectF());
        if (!dprChanged && m_lastGlowColor1 != m_glowColor1) {
            replaceTexture(m_glowNode1, window()->createTextureFromImage(generateGlowTexture(256, m_glowColor1, dpr)));
            m_lastGlowColor1 = m_glowColor1;
        }
    }

    // Aggiorna glow 2
    {
        const float gs = static_cast<float>(m_glowSize2);
        m_glowNode2->setRect(gs > 0.01f ? QRectF(-r*gs, -r*gs, r*2*gs, r*2*gs) : QRectF());
        if (!dprChanged && m_lastGlowColor2 != m_glowColor2) {
            replaceTexture(m_glowNode2, window()->createTextureFromImage(generateGlowTexture(256, m_glowColor2, dpr)));
            m_lastGlowColor2 = m_glowColor2;
        }
    }

    m_taoNode->setRect(-r, -r, r*2, r*2);

    // ── Rotazione Tao ─────────────────────────────────────────────────────────
    QMatrix4x4 tM;
    tM.rotate(qRadiansToDegrees(m_rotation), 0, 0, 1);
    m_taoRotNode->setMatrix(tM);

    // ── Orologio ──────────────────────────────────────────────────────────────
    if (m_showClock) {
        const QTime t   = QTime::currentTime();
        const float ms  = t.msec() / 1000.0f;
        const float sec = (t.second() + ms) * 6.0f;
        const float min = (t.minute() + sec / 360.0f) * 6.0f;
        const float hr  = (t.hour() % 12 + min / 360.0f) * 30.0f;

        auto updateHand = [&](int idx, float angle, float len, const QColor &col) {
            auto *hn  = static_cast<QSGGeometryNode*>(m_clockGroup->childAtIndex(idx));
            auto *geo = hn->geometry();
            if (geo->vertexCount() != 2) geo->allocate(2);
            QSGGeometry::Point2D *v = geo->vertexDataAsPoint2D();
            const float rad = qDegreesToRadians(angle - 90.0f);
            v[0].set(0, 0);
            v[1].set(std::cos(rad) * len, std::sin(rad) * len);
            hn->markDirty(QSGNode::DirtyGeometry);
            auto *mat = static_cast<QSGFlatColorMaterial*>(hn->material());
            if (mat->color() != col) {
                mat->setColor(col);
                hn->markDirty(QSGNode::DirtyMaterial);
            }
        };

        updateHand(0, hr,  r * 0.5f, m_hourHandColor);
        updateHand(1, min, r * 0.8f, m_minuteHandColor);
        updateHand(2, sec, r * 0.9f, m_secondHandColor);
    } else {
        // Nasconde le lancette senza deallocare
        for (int i = 0; i < 3; ++i) {
            auto *hn = static_cast<QSGGeometryNode*>(m_clockGroup->childAtIndex(i));
            if (hn->geometry()->vertexCount() > 0) {
                hn->geometry()->allocate(0);
                hn->markDirty(QSGNode::DirtyGeometry);
            }
        }
    }

    // ── Particelle ────────────────────────────────────────────────────────────
    // Buffer fisso MAX_PARTICLES: nessuna riallocazione driver tra i frame.
    // Le particelle inattive hanno size=0 e vengono scartate dalla GPU.
    QSGGeometry *pGeo = m_particleNode->geometry();
    if (pGeo->vertexCount() != MAX_PARTICLES)
        pGeo->allocate(MAX_PARTICLES);

    if (!m_simulationPending) {
        std::memcpy(pGeo->vertexData(),
                    m_verticesRender.data(),
                    static_cast<size_t>(MAX_PARTICLES) * sizeof(ParticleVertex));
        m_particleNode->markDirty(QSGNode::DirtyGeometry);
    }

    updateSimulation();
    return root;
}

// ═════════════════════════════════════════════════════════════════════════════
// generateGlowTexture
// ═════════════════════════════════════════════════════════════════════════════

QImage TaoNew::generateGlowTexture(int size, const QColor &color, qreal dpr)
{
    const int phys = qRound(size * dpr);
    QImage img(phys, phys, QImage::Format_ARGB32_Premultiplied);
    img.setDevicePixelRatio(dpr);
    img.fill(Qt::transparent);

    QRadialGradient g(phys * 0.5, phys * 0.5, phys * 0.5);
    g.setColorAt(0.0, color);
    QColor fade = color;
    fade.setAlpha(static_cast<int>(color.alpha() * 0.3));
    g.setColorAt(0.7, fade);
    g.setColorAt(1.0, Qt::transparent);

    QPainter p(&img);
    p.setPen(Qt::NoPen);
    p.setBrush(g);
    p.drawEllipse(0, 0, phys, phys);
    return img;
}

// ═════════════════════════════════════════════════════════════════════════════
// generateTaoTexture
// ═════════════════════════════════════════════════════════════════════════════

QImage TaoNew::generateTaoTexture(int size, qreal dpr)
{
    const int   phys = qRound(size * dpr);
    const float c    = phys * 0.5f;
    const float r    = c - 2.0f * static_cast<float>(dpr);

    QImage img(phys, phys, QImage::Format_ARGB32_Premultiplied);
    img.setDevicePixelRatio(dpr);
    img.fill(Qt::transparent);

    QPainter p(&img);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    p.setPen(Qt::NoPen);

    // 1. Cerchio base nero (evita bleeding sui bordi)
    p.setBrush(Qt::black);
    p.drawEllipse(QPointF(c, c), r, r);

    // 2. Metà destra bianca
    p.setBrush(Qt::white);
    p.drawPie(QRectF(c-r, c-r, r*2, r*2), 90*16, -180*16);

    // 3. Cerchio medio inferiore bianco
    p.setBrush(Qt::white);
    p.drawEllipse(QPointF(c, c + r*0.5f), r*0.5f, r*0.5f);

    // 4. Cerchio medio superiore nero
    p.setBrush(Qt::black);
    p.drawEllipse(QPointF(c, c - r*0.5f), r*0.5f, r*0.5f);

    // 5. Puntino inferiore nero
    p.setBrush(Qt::black);
    p.drawEllipse(QPointF(c, c + r*0.5f), r/6.0f, r/6.0f);

    // 6. Puntino superiore bianco
    p.setBrush(Qt::white);
    p.drawEllipse(QPointF(c, c - r*0.5f), r/6.0f, r/6.0f);

    return img;
}