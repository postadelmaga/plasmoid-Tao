#include "TaoQGraphGemini.h"
#include <QSGGeometryNode>
#include <QSGSimpleTextureNode>
#include <QSGTextureMaterial>
#include <QSGTransformNode>
#include <QPainter>
#include <QtMath>
#include <QRandomGenerator>
#include <QQuickWindow>
#include <QThreadPool>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>

// --- Strutture GPU ---
struct ParticleVertex {
    float x, y;
    float u, v;
    unsigned char r, g, b, a;
};

static const QSGGeometry::AttributeSet &particleAttributes() {
    static QSGGeometry::Attribute data[] = {
        QSGGeometry::Attribute::create(0, 2, QSGGeometry::FloatType, true),
        QSGGeometry::Attribute::create(1, 2, QSGGeometry::FloatType),
        QSGGeometry::Attribute::create(2, 4, QSGGeometry::UnsignedByteType)
    };
    static QSGGeometry::AttributeSet attrs = { 3, 20, data };
    return attrs;
}

// --- Worker Logic (Thread Separato) ---
void ParticleWorker::run() {
    if (count <= 0) return;
    
    float cx = width / 2.0f;
    float cy = height / 2.0f;
    float r = qMin(width, height) / 4.0f;
    float flowSpeed = 1.0f; // Costante interna
    
    // Uso un generatore locale per thread-safety
    auto gen = QRandomGenerator::global(); 
    int activeCount = qMin(count, particles->size());
    ParticleData* pData = particles->data();

    for (int i = 0; i < activeCount; ++i) {
        ParticleData& p = pData[i];
        
        // Fisica base
        p.x += p.vx * flowSpeed;
        p.y += p.vy * flowSpeed;
        p.life -= p.decay * flowSpeed;
        
        // Respawn
        if (p.life <= 0) {
            p.life = 1.0f;
            // Posiziona sull'anello
            float angle = gen->generateDouble() * M_PI * 2.0f;
            // Distanza leggermente variabile
            float dist = r * (1.1f + gen->generateDouble() * 0.5f);
            
            p.x = cx + std::cos(angle) * dist;
            p.y = cy + std::sin(angle) * dist;
            
            // Velocità casuale
            p.vx = (gen->generateDouble() - 0.5f) * 0.4f;
            p.vy = (gen->generateDouble() - 0.5f) * 0.4f;
            
            p.decay = 0.002f + gen->generateDouble() * 0.003f;
            p.size = 0.5f + gen->generateDouble() * 1.5f;
        }
    }
}

// --- Implementation ---

TaoQGraphGemini::TaoQGraphGemini(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
    
    // Resize buffers
    m_particlesA.resize(MAX_PARTICLES);
    m_particlesB.resize(MAX_PARTICLES);
    
    // Init pointers
    m_readBuffer = &m_particlesA;
    m_writeBuffer = &m_particlesB;
    
    initParticles(); // Init both buffers ideally, but one is enough to start

    // Timer ad alta frequenza per controllare il thread
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TaoQGraphGemini::checkSimulation);
    timer->start(16); // ~60fps check
}

TaoQGraphGemini::~TaoQGraphGemini() {
    // Attendere eventuali thread attivi se necessario
}

void TaoQGraphGemini::setParticleCount(int count) {
    if (m_particleCount == count) return;
    m_particleCount = qBound(0, count, MAX_PARTICLES);
    Q_EMIT particleCountChanged();
}
// ... (Altri setter standard omessi per brevità, identici a prima) ...
void TaoQGraphGemini::setRotationSpeed(float speed) {
    if (qFuzzyCompare(m_rotationSpeed, speed)) return;
    m_rotationSpeed = speed; 
    Q_EMIT rotationSpeedChanged();
}
void TaoQGraphGemini::setClockwise(bool clockwise) {
    if (m_clockwise == clockwise) return;
    m_clockwise = clockwise; 
    Q_EMIT clockwiseChanged();
}
void TaoQGraphGemini::setShowClock(bool show) {
    if (m_showClock == show) return;
    m_showClock = show; 
    update(); 
    Q_EMIT showClockChanged();
}
void TaoQGraphGemini::setLowCpuMode(bool lowCpu) {
    if (m_lowCpuMode == lowCpu) return;
    m_lowCpuMode = lowCpu; 
    Q_EMIT lowCpuModeChanged();
}

void TaoQGraphGemini::initParticles() {
    for(auto& p : m_particlesA) p.life = 0;
    for(auto& p : m_particlesB) p.life = 0;
}

// Chiamato dal Timer nel Main Thread
void TaoQGraphGemini::checkSimulation() {
    float dir = m_clockwise ? 1.0f : -1.0f;
    m_rotation += (m_rotationSpeed / 1000.0f) * dir;

    if (m_lowCpuMode) {
        update();
        return;
    }

    // Se la simulazione precedente è ancora in corso, saltiamo questo tick
    // per evitare di accumulare thread o causare data race.
    if (m_simulationRunning) {
        return;
    }

    // Passaggio dati al thread di calcolo
    // Scambiamo i buffer: il buffer di scrittura precedente diventa quello di lettura (per il render)
    std::swap(m_readBuffer, m_writeBuffer);
    update(); // Notifica allo Scene Graph di usare il nuovo m_readBuffer

    m_simulationRunning = true;

    // Catturiamo lo stato necessario per il thread
    float w = width();
    float h = height();
    int count = m_particleCount;
    
    // Usiamo QtConcurrent::run per eseguire il calcolo in modo asincrono
    // Nota: copiamo i dati nel buffer di scrittura prima di lasciarli al thread
    *m_writeBuffer = *m_readBuffer;
    ParticleData* dataPtr = m_writeBuffer->data();
    int dataSize = m_writeBuffer->size();

    QFuture<void> future = QtConcurrent::run([this, dataPtr, dataSize, w, h, count]() {
        if (count <= 0) return;
        
        float cx = w / 2.0f;
        float cy = h / 2.0f;
        float r = qMin(w, h) / 4.0f;
        float flowSpeed = 1.0f;
        
        auto gen = QRandomGenerator::global(); 
        int activeCount = qMin(count, dataSize);

        for (int i = 0; i < activeCount; ++i) {
            ParticleData& p = dataPtr[i];
            
            p.x += p.vx * flowSpeed;
            p.y += p.vy * flowSpeed;
            p.life -= p.decay * flowSpeed;
            
            if (p.life <= 0) {
                p.life = 1.0f;
                float angle = gen->generateDouble() * M_PI * 2.0f;
                float dist = r * (1.1f + gen->generateDouble() * 0.5f);
                p.x = cx + std::cos(angle) * dist;
                p.y = cy + std::sin(angle) * dist;
                p.vx = (gen->generateDouble() - 0.5f) * 0.4f;
                p.vy = (gen->generateDouble() - 0.5f) * 0.4f;
                p.decay = 0.002f + gen->generateDouble() * 0.003f;
                p.size = 0.5f + gen->generateDouble() * 1.5f;
            }
        }
    });

    // Usiamo un watcher (o semplicemente connettiamo al future se possibile)
    // In questo caso, per semplicità e performance, usiamo un watcher membro
    // gestito per resettare il flag quando il thread finisce.
    static QFutureWatcher<void>* watcher = nullptr;
    if (!watcher) {
        watcher = new QFutureWatcher<void>(this);
        connect(watcher, &QFutureWatcher<void>::finished, this, [this]() {
            m_simulationRunning = false;
        });
    }
    watcher->setFuture(future);
}

QSGNode *TaoQGraphGemini::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) {
    QSGNode *root = oldNode;
    
    // --- Setup Iniziale Nodi (eseguito una sola volta) ---
    if (!root) {
        root = new QSGNode();

        // 1. Nodo Particelle
        QSGGeometryNode *pNode = new QSGGeometryNode();
        QSGGeometry *pGeo = new QSGGeometry(particleAttributes(), 0);
        pGeo->setDrawingMode(QSGGeometry::DrawTriangles);
        pNode->setGeometry(pGeo);
        pNode->setFlag(QSGNode::OwnsGeometry);

        // Uso Materiale Standard invece di Custom (risolve errore compilazione)
        QSGTextureMaterial *pMat = new QSGTextureMaterial();
        // Genera texture glow FIXATA (con padding trasparente)
        QSGTexture *tGlow = window()->createTextureFromImage(generateGlowTexture(64));
        tGlow->setHorizontalWrapMode(QSGTexture::ClampToEdge); // FONDAMENTALE per evitare bordi
        tGlow->setVerticalWrapMode(QSGTexture::ClampToEdge);
        pMat->setTexture(tGlow);
        
        // Impostazioni Materiale per "Glow"
        // QSGTextureMaterial usa Alpha Blending standard. 
        // Per migliorare l'aspetto, assicuriamoci che l'immagine sia premoltiplicata bene.
        pNode->setMaterial(pMat);
        pNode->setFlag(QSGNode::OwnsMaterial);
        
        root->appendChildNode(pNode);

        // 2. Nodo Tao (trasformazione)
        QSGTransformNode *tNode = new QSGTransformNode();
        QSGSimpleTextureNode *sNode = new QSGSimpleTextureNode();
        QSGTexture *tTao = window()->createTextureFromImage(generateTaoTexture(512));
        tTao->setFiltering(QSGTexture::Linear);
        sNode->setTexture(tTao);
        sNode->setOwnsTexture(true);
        tNode->appendChildNode(sNode);
        root->appendChildNode(tNode);
    }

    // --- Aggiornamento GPU Frame-by-Frame ---
    QSGGeometryNode *pNode = static_cast<QSGGeometryNode*>(root->childAtIndex(0));
    QSGTransformNode *tNode = static_cast<QSGTransformNode*>(root->childAtIndex(1));
    QSGSimpleTextureNode *sNode = static_cast<QSGSimpleTextureNode*>(tNode->childAtIndex(0));

    // 1. Aggiorna Tao
    float w = width();
    float h = height();
    float cx = w/2.0f; 
    float cy = h/2.0f;
    float r = qMin(w, h)/4.5f;

    sNode->setRect(-r, -r, r*2, r*2);
    QMatrix4x4 m; 
    m.translate(cx, cy); 
    m.rotate(m_rotation * 180.0f / M_PI, 0, 0, 1);
    tNode->setMatrix(m);

    // 2. Aggiorna Particelle (dal Read Buffer)
    if (!m_lowCpuMode) {
        QSGGeometry *geo = pNode->geometry();
        int activeCount = qMin(m_particleCount, MAX_PARTICLES);
        
        // Alloca vertici solo se necessario (QSGGeometry gestisce internamente la riallocazione smart)
        geo->allocate(activeCount * 4, activeCount * 6); 

        ParticleVertex *v = (ParticleVertex *)geo->vertexData();
        quint16 *indices = geo->indexDataAsUShort();
        
        // Usiamo il buffer di lettura (sicuro, il thread scrive sull'altro)
        const ParticleData* pData = m_readBuffer->data();

        // Ottimizzazione loop: variabili locali nei registri
        for (int i = 0; i < activeCount; ++i) {
            const ParticleData &p = pData[i];
            
            if (p.life > 0) {
                float size = p.size * 15.0f;
                float hs = size / 2.0f;
                // Alpha: 255 pieno, moduliamo solo A. 
                // Colore RGB bianco puro per massimo effetto luce.
                unsigned char a = static_cast<unsigned char>(p.life * 255.0f * 0.9f);
                
                float x = p.x; 
                float y = p.y;

                // Scrittura diretta
                v[0] = {x-hs, y-hs, 0, 0, 255, 255, 255, a};
                v[1] = {x+hs, y-hs, 1, 0, 255, 255, 255, a};
                v[2] = {x-hs, y+hs, 0, 1, 255, 255, 255, a};
                v[3] = {x+hs, y+hs, 1, 1, 255, 255, 255, a};

                int b = i * 4;
                indices[0]=b; indices[1]=b+1; indices[2]=b+2; 
                indices[3]=b+2; indices[4]=b+1; indices[5]=b+3;

                v += 4; indices += 6;
            } else {
                // Collasso particelle morte (invisibili)
                v[0].a=0; v[1].a=0; v[2].a=0; v[3].a=0;
                v+=4; indices+=6;
            }
        }
        pNode->markDirty(QSGNode::DirtyGeometry);
    } else {
        pNode->geometry()->allocate(0, 0);
    }

    return root;
}

// Generazione Texture Ottimizzata (NO ARTIFATTI)
QImage TaoQGraphGemini::generateGlowTexture(int s) {
    // Aumentiamo la dimensione interna per evitare bordi duri
    QImage img(s, s, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    
    QPainter p(&img);
    // Disegna il gradiente un po' più piccolo della texture intera (padding)
    // es: texture 64px, disegno raggio 30px al centro (32,32)
    // Lascia 2px di bordo completamente trasparente
    float radius = (s / 2.0f) - 2.0f; 
    
    QRadialGradient g(s/2, s/2, radius);
    g.setColorAt(0.0, QColor(255, 255, 255, 255)); // Centro bianco
    g.setColorAt(0.2, QColor(200, 240, 255, 200)); // Alone azzurrino
    g.setColorAt(1.0, QColor(0, 0, 0, 0));         // Fine trasparente
    
    p.setBrush(g);
    p.setPen(Qt::NoPen);
    p.drawEllipse(QPointF(s/2, s/2), radius, radius);
    return img;
}

QImage TaoQGraphGemini::generateTaoTexture(int s) {
    QImage img(s, s, QImage::Format_ARGB32_Premultiplied); 
    img.fill(Qt::transparent);
    QPainter p(&img); 
    p.setRenderHint(QPainter::Antialiasing);
    
    // Lasciamo un piccolo bordo anche qui per l'antialiasing
    float c = s/2.0f;
    float r = s/2.0f - 4.0f; 

    auto drawC = [&](float cx, float cy, float rad, float sa, float span, QColor col) {
        p.setBrush(col); p.setPen(Qt::NoPen);
        p.drawPie(QRectF(cx-rad, cy-rad, rad*2, rad*2), (int)(sa*180/M_PI*16), (int)(span*180/M_PI*16));
    };
    drawC(c,c,r,0,M_PI*2, QColor(235,235,235));
    drawC(c,c,r,-M_PI/2,M_PI,QColor(10,10,10));
    drawC(c,c-r/2,r/2,0,M_PI*2,QColor(10,10,10));
    drawC(c,c+r/2,r/2,0,M_PI*2,QColor(235,235,235));
    drawC(c,c-r/2,r/7,0,M_PI*2,QColor(255,255,255));
    drawC(c,c+r/2,r/7,0,M_PI*2,QColor(0,0,0));
    return img;
}