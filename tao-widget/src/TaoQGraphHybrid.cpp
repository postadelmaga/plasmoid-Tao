#include "TaoQGraphHybrid.h"
#include <QSGGeometryNode>
#include <QSGSimpleTextureNode>
#include <QSGTextureMaterial>
#include <QSGFlatColorMaterial>
#include <QSGTransformNode>
#include <QPainter>
#include <QtMath>
#include <QRandomGenerator>
#include <QQuickWindow>
#include <QtConcurrent>
#include <QTime>
#include <cstring> // Necessario per std::memcpy e std::memset
#include <cmath>

// Helper per packing colore veloce (Little Endian: A-B-G-R in memoria -> 0xAABBGGRR)
inline quint32 packColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    return (a << 24) | (r << 16) | (g << 8) | b;
}

// Struttura vertice allineata per la GPU
struct ParticleVertex {
    float x, y;
    float u, v;
    quint32 color; // 4 bytes (R,G,B,A packed)
};

// CORREZIONE QUI: Restituisce AttributeSet&, non Attribute&
static const QSGGeometry::AttributeSet &particleAttributes() {
    static QSGGeometry::Attribute data[] = {
        QSGGeometry::Attribute::create(0, 2, QSGGeometry::FloatType, false),      // Pos
        QSGGeometry::Attribute::create(1, 2, QSGGeometry::FloatType),             // UV
        QSGGeometry::Attribute::create(2, 4, QSGGeometry::UnsignedByteType, true) // Color
    };
    static QSGGeometry::AttributeSet attrs = { 3, 20, data }; // Stride 20 bytes
    return attrs;
}

TaoQGraphHybrid::TaoQGraphHybrid(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
    m_particles.resize(MAX_PARTICLES);
    m_particlesRender.resize(MAX_PARTICLES);
    
    // Inizializza memoria a zero
    std::memset(m_particles.data(), 0, sizeof(ParticleData) * MAX_PARTICLES);

    connect(&m_watcher, &QFutureWatcher<void>::finished, this, [this]() {
        // Safe swap/copy ottimizzato con memcpy
        if (m_particleCount > 0) {
            std::memcpy(m_particlesRender.data(), m_particles.data(), m_particleCount * sizeof(ParticleData));
        }
        m_renderCount = m_particleCount; 
        m_simulationPending = false;
        update(); // Triggera updatePaintNode
    });

    m_timeTracker.start();
}

TaoQGraphHybrid::~TaoQGraphHybrid() {
    if (m_watcher.isRunning()) {
        m_watcher.waitForFinished();
    }
}

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
    if (change == ItemVisibleHasChanged && value.boolValue) {
        update();
    }
    QQuickItem::itemChange(change, value);
}

void TaoQGraphHybrid::updateSimulation() {
    if (m_simulationPending || m_lowCpuMode || !isVisible()) {
        return;
    }
    
    m_simulationPending = true;
    
    const int count = m_particleCount;
    const float w = width();
    const float h = height();
    const QPointF mPos = m_mousePos;
    const float currentDt = (m_lastDt > 0.001f && m_lastDt < 1.0f) ? m_lastDt : 0.016f;
    
    QFuture<void> future = QtConcurrent::run([this, count, w, h, mPos, currentDt]() {
        if (count <= 0) return;

        float cx = w * 0.5f;
        float cy = h * 0.5f;
        float r = qMin(w, h) / 4.5f; 
        float rSq = r * r;
        
        ParticleData* pData = m_particles.data();
        auto* gen = QRandomGenerator::global(); 

        const float df = currentDt * 60.0f; 
        const float friction = std::pow(0.98f, df); // Pre-calcolo frizione

        // Pre-calcolo limiti mouse per evitare check ripetuti
        const bool isMouseValid = mPos.x() >= 0 && mPos.y() >= 0 && mPos.x() <= w && mPos.y() <= h;
        const float mx = static_cast<float>(mPos.x());
        const float my = static_cast<float>(mPos.y());

        for (int i = 0; i < count; ++i) {
            ParticleData& p = pData[i];
            
            if (p.life > 0.0f) {
                // Interattività col Mouse
                float dx = mx - p.x;
                float dy = my - p.y;
                
                // Check veloce AABB prima di fare i calcoli pesanti
                if (isMouseValid && qAbs(dx) < 300 && qAbs(dy) < 300) {
                    float distSq = dx*dx + dy*dy;
                    if (distSq < 90000.0f) {
                        float f = 3.5f / (distSq + 100.0f);
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

                // Boundary Checks
                if (p.x < 0) { p.x = 0; p.vx = qAbs(p.vx) * 0.4f; }
                else if (p.x > w) { p.x = w; p.vx = -qAbs(p.vx) * 0.4f; }
                
                if (p.y < 0) { p.y = 0; p.vy = qAbs(p.vy) * 0.4f; }
                else if (p.y > h) { p.y = h; p.vy = -qAbs(p.vy) * 0.4f; }

                // Evitamento Tao
                float tdx = p.x - cx;
                float tdy = p.y - cy;
                float tDistSq = tdx*tdx + tdy*tdy;
                if (tDistSq < rSq) {
                    float tDist = std::sqrt(tDistSq);
                    if (tDist < 0.1f) tDist = 0.1f;
                    float invDist = 1.0f / tDist;
                    float nx = tdx * invDist;
                    float ny = tdy * invDist;
                    
                    float push = (r - tDist) * 0.3f;
                    p.x += nx * push;
                    p.y += ny * push;
                    
                    float dot = p.vx * nx + p.vy * ny;
                    if (dot < 0) {
                        p.vx -= 1.6f * dot * nx;
                        p.vy -= 1.6f * dot * ny;
                    }
                }

                p.life -= p.decay * df;
                
                // CALCOLO COLORE (Ottimizzato: spostato nel thread simulazione)
                unsigned char alpha = static_cast<unsigned char>(p.life * 255);
                unsigned char red, green, blue;

                // (i & 7) == 0 è equivalente veloce a (i % 8) == 0. 
                // Usiamo modulo 7 come originale se critico, ma &7 è più veloce.
                if ((i % 7) == 0) { 
                    red = 255;
                    green = static_cast<unsigned char>(100 + p.life * 155);
                    blue = 0;
                } else {
                    float speed = std::sqrt(p.vx*p.vx + p.vy*p.vy);
                    red   = static_cast<unsigned char>(qMin(255.0f, 127.0f + speed * 400.0f)); 
                    green = static_cast<unsigned char>(qMin(255.0f, 204.0f + speed * 200.0f));
                    blue  = 255;
                }
                p.packedColor = packColor(red, green, blue, alpha);

            } else {
                // Respawn
                p.life = 1.0f;
                double angle = gen->generateDouble() * 6.28318; // 2PI approx
                double dist = r * (0.5 + gen->generateDouble() * 2.0); 
                
                p.x = cx + static_cast<float>(std::cos(angle) * dist);
                p.y = cy + static_cast<float>(std::sin(angle) * dist);
                
                p.vx = (gen->generateDouble() - 0.5f) * 0.6f;
                p.vy = (gen->generateDouble() - 0.5f) * 0.6f;
                
                float sdx = p.x - cx;
                float sdy = p.y - cy;
                if (sdx*sdx + sdy*sdy < rSq) {
                  p.x += (sdx > 0 ? r : -r);
                }
                
                p.decay = 0.003f + gen->generateDouble() * 0.008f;
                p.size = 2.0f + gen->generateDouble() * 8.0f;
                // Colore iniziale (Alpha 255)
                p.packedColor = packColor(127, 204, 255, 255);
            }
        }
    });

    m_watcher.setFuture(future);
}

void TaoQGraphHybrid::setupGeometryIndices(QSGGeometry *geo, int count) {
    if (count <= 0) return;
    geo->allocate(count * 4, count * 6);
    quint32 *indices = static_cast<quint32*>(geo->indexData());
    for (int i = 0; i < count; ++i) {
        quint32 base = i * 4;
        indices[0] = base;
        indices[1] = base + 1;
        indices[2] = base + 2;
        indices[3] = base + 2;
        indices[4] = base + 1;
        indices[5] = base + 3;
        indices += 6;
    }
}

QSGNode *TaoQGraphHybrid::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) {
    QSGNode *root = oldNode;
    
    // Inizializzazione Nodo
    if (!root) {
        root = new QSGNode();

        // Nodo Particelle
        QSGGeometryNode *pNode = new QSGGeometryNode();
        
        // CORREZIONE QUI: Chiamata corretta al costruttore QSGGeometry
        // particleAttributes() ora ritorna AttributeSet&, quindi funziona.
        QSGGeometry *pGeo = new QSGGeometry(particleAttributes(), 0, 0, QSGGeometry::UnsignedIntType);
        
        pGeo->setDrawingMode(QSGGeometry::DrawTriangles);
        pGeo->setIndexDataPattern(QSGGeometry::StaticPattern); 
        pGeo->setVertexDataPattern(QSGGeometry::StreamPattern); // Stream per aggiornamenti frequenti
        
        pNode->setGeometry(pGeo);
        pNode->setFlag(QSGNode::OwnsGeometry);

        QSGTextureMaterial *pMat = new QSGTextureMaterial();
        QSGTexture *tGlow = window()->createTextureFromImage(generateGlowTexture(32));
        tGlow->setFiltering(QSGTexture::Linear);
        pMat->setTexture(tGlow);
        pNode->setMaterial(pMat);
        pNode->setFlag(QSGNode::OwnsMaterial);
        
        root->appendChildNode(pNode);

        // Nodo Tao e Sfondo
        QSGTransformNode *tNode = new QSGTransformNode();
        
        QSGSimpleTextureNode *gNode = new QSGSimpleTextureNode();
        QSGTexture *tGlowBg = window()->createTextureFromImage(generateGlowTexture(256));
        gNode->setTexture(tGlowBg);
        gNode->setOwnsTexture(true);
        tNode->appendChildNode(gNode);

        QSGSimpleTextureNode *sNode = new QSGSimpleTextureNode();
        QSGTexture *tTao = window()->createTextureFromImage(generateTaoTexture(256)); 
        tTao->setFiltering(QSGTexture::Linear);
        sNode->setTexture(tTao);
        sNode->setOwnsTexture(true);
        tNode->appendChildNode(sNode);

        // Nodo Orologio
        QSGGeometryNode *cNode = new QSGGeometryNode();
        QSGGeometry *cGeo = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 0);
        cGeo->setLineWidth(2);
        cGeo->setDrawingMode(QSGGeometry::DrawLines);
        cNode->setGeometry(cGeo);
        cNode->setFlag(QSGNode::OwnsGeometry);
        QSGFlatColorMaterial *cMat = new QSGFlatColorMaterial();
        cMat->setColor(QColor(255, 255, 255, 180));
        cNode->setMaterial(cMat);
        cNode->setFlag(QSGNode::OwnsMaterial);
        tNode->appendChildNode(cNode);

        root->appendChildNode(tNode);
    }

    qint64 current = m_timeTracker.elapsed();
    if (m_lastTime == 0) m_lastTime = current;
    float dt = (current - m_lastTime) / 1000.0f;
    m_lastTime = current;
    m_lastDt = dt;
    
    float dir = m_clockwise ? 1.0f : -1.0f;
    float speedFactor = 0.06f; 
    m_rotation += (m_rotationSpeed * speedFactor * dir * dt);

    QSGGeometryNode *pNode = static_cast<QSGGeometryNode*>(root->childAtIndex(0));
    QSGTransformNode *tNode = static_cast<QSGTransformNode*>(root->childAtIndex(1));
    
    float w = width(), h = height();
    float r = qMin(w, h) / 4.5f;
    
    QSGSimpleTextureNode *gNode = static_cast<QSGSimpleTextureNode*>(tNode->childAtIndex(0));
    QSGSimpleTextureNode *sNode = static_cast<QSGSimpleTextureNode*>(tNode->childAtIndex(1));
    QSGGeometryNode *cNode = static_cast<QSGGeometryNode*>(tNode->childAtIndex(2));
    
    gNode->setRect(-r*1.3f, -r*1.3f, r*2.6f, r*2.6f);
    sNode->setRect(-r, -r, r*2, r*2);
    
    if (m_showClock) {
        QSGGeometry *geo = cNode->geometry();
        geo->allocate(6);
        QSGGeometry::Point2D *v = geo->vertexDataAsPoint2D();
        QTime now = QTime::currentTime();
        float ms = now.msec() / 1000.0f;
        float s = (now.second() + ms) * 6.0f;
        float m = (now.minute() + s / 360.0f) * 6.0f;
        float h = (now.hour() % 12 + m / 360.0f) * 30.0f;

        auto setHand = [&](float angle, float len, int idx) {
            float rad = qDegreesToRadians(angle - 90);
            v[idx*2].x = 0; v[idx*2].y = 0;
            v[idx*2+1].x = std::cos(rad) * len; v[idx*2+1].y = std::sin(rad) * len;
        };
        setHand(h, r * 0.5f, 0);
        setHand(m, r * 0.8f, 1);
        setHand(s, r * 0.9f, 2);
        cNode->markDirty(QSGNode::DirtyGeometry);
    } else {
        if (cNode->geometry()->vertexCount() > 0)
            cNode->geometry()->allocate(0);
    }

    QMatrix4x4 m;
    m.translate(w/2.0f, h/2.0f);
    m.rotate(qRadiansToDegrees(m_rotation), 0, 0, 1);
    tNode->setMatrix(m);

    // Update Particelle (Render Loop Ottimizzato)
    if (!m_lowCpuMode) {
        QSGGeometry *geo = pNode->geometry();
        int activeCount = qMin(m_renderCount, MAX_PARTICLES);
        
        if (geo->vertexCount() != activeCount * 4) {
            setupGeometryIndices(geo, activeCount);
        }

        ParticleVertex *v = (ParticleVertex *)geo->vertexData();
        const ParticleData* pData = m_particlesRender.data();

        // LOOP CRITICO: Solo assegnazione di valori pre-calcolati
        for (int i = 0; i < activeCount; ++i) {
            const ParticleData &p = pData[i];
            float px = p.x;
            float py = p.y;
            float hs = p.size;
            quint32 col = p.packedColor; // Colore pronto all'uso

            v[0].x = px - hs; v[0].y = py - hs; v[0].u = 0; v[0].v = 0; v[0].color = col;
            v[1].x = px + hs; v[1].y = py - hs; v[1].u = 1; v[1].v = 0; v[1].color = col;
            v[2].x = px - hs; v[2].y = py + hs; v[2].u = 0; v[2].v = 1; v[2].color = col;
            v[3].x = px + hs; v[3].y = py + hs; v[3].u = 1; v[3].v = 1; v[3].color = col;

            v += 4;
        }
        
        pNode->markDirty(QSGNode::DirtyGeometry);
        
        updateSimulation();
    } else {
        if (pNode->geometry()->vertexCount() > 0)
            pNode->geometry()->allocate(0, 0);
    }

    return root;
}

QImage TaoQGraphHybrid::generateGlowTexture(int s) {
    QImage img(s, s, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter p(&img);
    QRadialGradient g(s/2.0, s/2.0, s/2.0);
    g.setColorAt(0.0, QColor(100, 200, 255, 128)); 
    g.setColorAt(0.7, QColor(100, 200, 255, 40));
    g.setColorAt(1.0, Qt::transparent);
    p.setBrush(g);
    p.setPen(Qt::NoPen);
    p.drawEllipse(0, 0, s, s);
    return img;
}

QImage TaoQGraphHybrid::generateTaoTexture(int s) {
    QImage img(s, s, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    
    float c = s / 2.0f;
    float r = c - 2.0f; 
    
    p.setPen(Qt::NoPen);
    
    p.setBrush(Qt::white);
    p.drawEllipse(QPointF(c, c), r, r);
    
    p.setBrush(Qt::black);
    p.drawPie(QRectF(2.0f, 2.0f, r*2, r*2), 90 * 16, 180 * 16);
    
    p.setBrush(Qt::white);
    p.drawEllipse(QPointF(c, c + r/2), r/2, r/2);
    
    p.setBrush(Qt::black);
    p.drawEllipse(QPointF(c, c - r/2), r/2, r/2);
    
    p.setBrush(Qt::black);
    p.drawEllipse(QPointF(c, c + r/2), r/6, r/6);
    
    p.setBrush(Qt::white);
    p.drawEllipse(QPointF(c, c - r/2), r/6, r/6);
    
    return img;
}