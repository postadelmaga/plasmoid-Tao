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

// Struttura vertice allineata per la GPU
struct ParticleVertex {
    float x, y;
    float u, v;
    unsigned char r, g, b, a;
};

// Definizione attributi geometria
static const QSGGeometry::AttributeSet &particleAttributes() {
    static QSGGeometry::Attribute data[] = {
        QSGGeometry::Attribute::create(0, 2, QSGGeometry::FloatType, true), // Pos
        QSGGeometry::Attribute::create(1, 2, QSGGeometry::FloatType),        // UV
        QSGGeometry::Attribute::create(2, 4, QSGGeometry::UnsignedByteType) // Color
    };
    static QSGGeometry::AttributeSet attrs = { 3, 20, data }; // Stride 20
    return attrs;
}

TaoQGraphHybrid::TaoQGraphHybrid(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
    m_particles.resize(MAX_PARTICLES);
    
    // Inizializza particelle morte
    for(auto& p : m_particles) p.life = 0.0f;

    // Quando il calcolo asincrono finisce, richiediamo un update grafico
    connect(&m_watcher, &QFutureWatcher<void>::finished, this, [this]() {
        m_simulationPending = false;
        update(); // Triggera updatePaintNode nel render thread
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
        update(); // Riavvia il loop se diventa visibile
    }
    QQuickItem::itemChange(change, value);
}

// Logica di simulazione ottimizzata
void TaoQGraphHybrid::updateSimulation() {
    // Stop simulation when hidden, minimized, or in low CPU mode.
    // window()->isVisible() correctly handles cases where the window is obscured or minimized in many environments.
    if (m_simulationPending || m_lowCpuMode || !isVisible() || !window() || !window()->isVisible()) {
        return;
    }
    
    m_simulationPending = true;
    
    // Catturiamo lo stato attuale per il thread
    const int count = m_particleCount;
    const float w = width();
    const float h = height();
    const QPointF mPos = m_mousePos;
    
    // Usiamo QtConcurrent::run senza blockingMap per ridurre overhead.
    // Una CPU moderna itera 20k float in microsecondi, non serve dividere in thread.
    QFuture<void> future = QtConcurrent::run([this, count, w, h, mPos]() {
        if (count <= 0) return;

        float cx = w * 0.5f;
        float cy = h * 0.5f;
        float r = qMin(w, h) / 4.5f; 
        float rSq = r * r;
        
        // Puntatore diretto ai dati per velocità (evita operator[])
        ParticleData* pData = m_particles.data();
        
        // Random generator thread-local (molto più veloce di global() in un loop)
        auto* gen = QRandomGenerator::global(); 

        const float df = m_lastDt * 60.0f; // Delta factor rispetto a 60fps

        for (int i = 0; i < count; ++i) {
            ParticleData& p = pData[i];
            
            if (p.life > 0.0f) {
                // Interattività col Mouse (Attrazione Potenziata)
                float dx = static_cast<float>(mPos.x()) - p.x;
                float dy = static_cast<float>(mPos.y()) - p.y;
                float distSq = dx*dx + dy*dy;
                
                // Se il mouse è "fuori" (es. coordinate negative o troppo grandi), resettiamo l'effetto
                bool isMouseValid = mPos.x() >= 0 && mPos.y() >= 0 && mPos.x() <= w && mPos.y() <= h;

                if (isMouseValid && distSq < 90000.0f) { // Raggio aumentato per "tutto schermo"
                    float f = 3.5f / (distSq + 100.0f);
                    p.vx += dx * f * df;
                    p.vy += dy * f * df;
                } else {
                    // Attrito per tornare a comportamento normale (frame-rate independent)
                    float friction = std::pow(0.98f, df);
                    p.vx *= friction;
                    p.vy *= friction;
                }

                p.x += p.vx * df;
                p.y += p.vy * df;

                // Evitamento del Tao centrale (Rimbalzo/Giramento intorno)
                float tdx = p.x - cx;
                float tdy = p.y - cy;
                float tDistSq = tdx*tdx + tdy*tdy;
                if (tDistSq < rSq) {
                    float tDist = std::sqrt(tDistSq);
                    float nx = tdx / tDist;
                    float ny = tdy / tDist;
                    
                    // Forza di repulsione dal centro
                    float push = (r - tDist) * 0.2f;
                    p.x += nx * push;
                    p.y += ny * push;
                    
                    // Riflessione velocità per "rimbalzare"
                    float dot = p.vx * nx + p.vy * ny;
                    if (dot < 0) {
                        p.vx -= 1.5f * dot * nx;
                        p.vy -= 1.5f * dot * ny;
                    }
                }

                p.life -= p.decay * df;
            } else {
                // Respawn logic
                p.life = 1.0f;
                double angle = gen->generateDouble() * M_PI * 2.0;
                double dist = r * (0.5 + gen->generateDouble() * 2.0); 
                
                p.x = cx + static_cast<float>(std::cos(angle) * dist);
                p.y = cy + static_cast<float>(std::sin(angle) * dist);
                
                p.vx = (gen->generateDouble() - 0.5f) * 0.6f;
                p.vy = (gen->generateDouble() - 0.5f) * 0.6f;
                // Se spawniamo dentro il Tao, spingiamo fuori
                float sdx = p.x - cx;
                float sdy = p.y - cy;
                if (sdx*sdx + sdy*sdy < rSq) {
                  p.x += (sdx > 0 ? r : -r);
                }
                // Decay calibrato per 60fps
                p.decay = 0.003f + gen->generateDouble() * 0.008f;
                p.size = 2.0f + gen->generateDouble() * 8.0f;
            }
        }
    });

    m_watcher.setFuture(future);
}

void TaoQGraphHybrid::setupGeometryIndices(QSGGeometry *geo, int count) {
    geo->allocate(count * 4, count * 6);
    quint16 *indices = geo->indexDataAsUShort();
    for (int i = 0; i < count; ++i) {
        quint16 base = i * 4;
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
        QSGGeometry *pGeo = new QSGGeometry(particleAttributes(), 0);
        pGeo->setDrawingMode(QSGGeometry::DrawTriangles);
        // Hint statico per gli indici, dinamico per i vertici
        pGeo->setIndexDataPattern(QSGGeometry::StaticPattern); 
        pGeo->setVertexDataPattern(QSGGeometry::StreamPattern);
        
        pNode->setGeometry(pGeo);
        pNode->setFlag(QSGNode::OwnsGeometry);

        QSGTextureMaterial *pMat = new QSGTextureMaterial();
        QSGTexture *tGlow = window()->createTextureFromImage(generateGlowTexture(32)); // Texture più piccola basta
        tGlow->setFiltering(QSGTexture::Linear);
        pMat->setTexture(tGlow);
        pNode->setMaterial(pMat);
        pNode->setFlag(QSGNode::OwnsMaterial);
        
        // Blend additivo per effetto glow migliore
        // Nota: QSGTextureMaterial standard fa Alpha Blending. 
        // Per Additive servirebbe custom shader, ma Alpha va bene per ora.
        
        root->appendChildNode(pNode);

        // Nodo Tao
        QSGTransformNode *tNode = new QSGTransformNode();
        
        // Glow Background Node (Nuovo: Colore sofffuso come WebGL)
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

        // Nodo Orologio (Opzionale)
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

    // Calcolo Delta Time per rotazione fluida
    qint64 current = m_timeTracker.elapsed();
    static qint64 last = current;
    float dt = (current - last) / 1000.0f;
    last = current;
    m_lastDt = dt;
    
    // Unifichiamo la velocità con la versione WebGL:
    // WebGL usa (speed / 1000) radianti per frame (a 60fps).
    // Quindi in secondi è (speed / 1000) * 60 radianti/sec.
    // Direzione: Qt rotate è CCW, quindi per Clockwise serve segno negativo.
    // Fix Clockwise Inversion: Qt rotate positive is CW in screen space.
    // Se m_clockwise è true, vogliamo rotazione positiva.
    float dir = m_clockwise ? 1.0f : -1.0f;
    float speedFactor = 0.06f; 
    m_rotation += (m_rotationSpeed * speedFactor * dir * dt);

    // Recupero Nodi figli
    QSGGeometryNode *pNode = static_cast<QSGGeometryNode*>(root->childAtIndex(0));
    QSGTransformNode *tNode = static_cast<QSGTransformNode*>(root->childAtIndex(1));
    
    // Update Tao (Matrice e Rettangolo)
    float w = width(), h = height();
    float r = qMin(w, h) / 4.5f;
    
    // Update Glow (Nodo 0), Tao (Nodo 1), Orologio (Nodo 2)
    QSGSimpleTextureNode *gNode = static_cast<QSGSimpleTextureNode*>(tNode->childAtIndex(0));
    QSGSimpleTextureNode *sNode = static_cast<QSGSimpleTextureNode*>(tNode->childAtIndex(1));
    QSGGeometryNode *cNode = static_cast<QSGGeometryNode*>(tNode->childAtIndex(2));
    
    // Glow più piccolo e soffuso (1.3 invece di 1.8)
    gNode->setRect(-r*1.3f, -r*1.3f, r*2.6f, r*2.6f);
    sNode->setRect(-r, -r, r*2, r*2);
    
    // Update Orologio
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
        cNode->geometry()->allocate(0);
        cNode->markDirty(QSGNode::DirtyGeometry);
    }

    QMatrix4x4 m;
    m.translate(w/2.0f, h/2.0f);
    m.rotate(qRadiansToDegrees(m_rotation), 0, 0, 1);
    tNode->setMatrix(m);

    // Update Particelle (Render Thread)
    if (!m_lowCpuMode) {
        QSGGeometry *geo = pNode->geometry();
        int activeCount = qMin(m_particleCount, MAX_PARTICLES);
        
        // Rialloca solo se il numero cambia drasticamente o non è inizializzato
        if (geo->vertexCount() != activeCount * 4) {
            setupGeometryIndices(geo, activeCount);
        }

        ParticleVertex *v = (ParticleVertex *)geo->vertexData();
        const ParticleData* pData = m_particles.data();

        // Loop critico di rendering: riempimento vertici
        for (int i = 0; i < activeCount; ++i) {
            const ParticleData &p = pData[i];
            float hs = p.size; 
            
            // Colori dinamici (Effetto "Brace" + ogni tanto "Fiamma")
            unsigned char alpha = static_cast<unsigned char>(p.life * 255);
            unsigned char red, green, blue;
            
            if (i % 7 == 0) { // 14% di particelle "Fire/Flame"
                red = 255;
                green = static_cast<unsigned char>(100 + p.life * 155); // Da arancione a giallo
                blue = 0;
            } else {
                float speed = std::sqrt(p.vx*p.vx + p.vy*p.vy);
                // Matched WebGL Cyan: (127, 204, 255) -> (0.5, 0.8, 1.0)
                red   = static_cast<unsigned char>(qMin(255.0f, 127.0f + speed * 400.0f)); 
                green = static_cast<unsigned char>(qMin(255.0f, 204.0f + speed * 200.0f));
                blue  = 255;
            }

            // Unrolling manuale per performance, coordinate UV fisse e colori per vertice
            v[0].x = p.x - hs; v[0].y = p.y - hs; v[0].u = 0; v[0].v = 0;
            v[0].r = red; v[0].g = green; v[0].b = blue; v[0].a = alpha;

            v[1].x = p.x + hs; v[1].y = p.y - hs; v[1].u = 1; v[1].v = 0;
            v[1].r = red; v[1].g = green; v[1].b = blue; v[1].a = alpha;

            v[2].x = p.x - hs; v[2].y = p.y + hs; v[2].u = 0; v[2].v = 1;
            v[2].r = red; v[2].g = green; v[2].b = blue; v[2].a = alpha;

            v[3].x = p.x + hs; v[3].y = p.y + hs; v[3].u = 1; v[3].v = 1;
            v[3].r = red; v[3].g = green; v[3].b = blue; v[3].a = alpha;

            v += 4;
        }
        
        pNode->markDirty(QSGNode::DirtyGeometry);
        
        // Avvia la simulazione per il PROSSIMO frame subito dopo aver disegnato questo
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
    // Colore WebGL: rgba(100, 200, 255, 0.5)
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
    
    // Base bianca
    p.setBrush(Qt::white);
    p.drawEllipse(QPointF(c, c), r, r);
    
    // Metà nera
    p.setBrush(Qt::black);
    p.drawPie(QRectF(2.0f, 2.0f, r*2, r*2), 90 * 16, 180 * 16);
    
    // Cerchio medio bianco (sotto)
    p.setBrush(Qt::white);
    p.drawEllipse(QPointF(c, c + r/2), r/2, r/2);
    
    // Cerchio medio nero (sopra)
    p.setBrush(Qt::black);
    p.drawEllipse(QPointF(c, c - r/2), r/2, r/2);
    
    // Puntini piccoli
    p.setBrush(Qt::black);
    p.drawEllipse(QPointF(c, c + r/2), r/6, r/6);
    
    p.setBrush(Qt::white);
    p.drawEllipse(QPointF(c, c - r/2), r/6, r/6);
    
    return img;
}