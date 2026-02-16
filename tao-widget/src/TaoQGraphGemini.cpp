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

// --- Implementation ---

TaoQGraphGemini::TaoQGraphGemini(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
    
    m_particlesA.resize(MAX_PARTICLES);
    m_particlesB.resize(MAX_PARTICLES);
    
    m_readBuffer = &m_particlesA;
    m_writeBuffer = &m_particlesB;
    
    initParticles();

    connect(&m_watcher, &QFutureWatcher<void>::finished, this, [this]() {
        m_simulationRunning = false;
    });

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TaoQGraphGemini::checkSimulation);
    timer->start(16);
}

TaoQGraphGemini::~TaoQGraphGemini() {
    m_watcher.waitForFinished();
}

void TaoQGraphGemini::setParticleCount(int count) {
    if (m_particleCount == count) return;
    m_particleCount = qBound(0, count, MAX_PARTICLES);
    Q_EMIT particleCountChanged();
}
// ... (Setter implementazioni) ...
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

void TaoQGraphGemini::checkSimulation() {
    float dir = m_clockwise ? 1.0f : -1.0f;
    m_rotation += (m_rotationSpeed / 1000.0f) * dir;

    if (m_lowCpuMode || m_simulationRunning) {
        if (m_lowCpuMode) update();
        return;
    }

    std::swap(m_readBuffer, m_writeBuffer);
    update(); 

    m_simulationRunning = true;

    float w = width();
    float h = height();
    int count = m_particleCount;
    
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

    m_watcher.setFuture(future);
}

QSGNode *TaoQGraphGemini::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) {
    QSGNode *root = oldNode;
    
    if (!root) {
        root = new QSGNode();

        QSGGeometryNode *pNode = new QSGGeometryNode();
        QSGGeometry *pGeo = new QSGGeometry(particleAttributes(), 0);
        pGeo->setDrawingMode(QSGGeometry::DrawTriangles);
        pNode->setGeometry(pGeo);
        pNode->setFlag(QSGNode::OwnsGeometry);

        QSGTextureMaterial *pMat = new QSGTextureMaterial();
        QSGTexture *tGlow = window()->createTextureFromImage(generateGlowTexture(64));
        tGlow->setHorizontalWrapMode(QSGTexture::ClampToEdge); 
        tGlow->setVerticalWrapMode(QSGTexture::ClampToEdge);
        pMat->setTexture(tGlow);
        pNode->setMaterial(pMat);
        pNode->setFlag(QSGNode::OwnsMaterial);
        root->appendChildNode(pNode);

        QSGTransformNode *tNode = new QSGTransformNode();
        QSGSimpleTextureNode *sNode = new QSGSimpleTextureNode();
        QSGTexture *tTao = window()->createTextureFromImage(generateTaoTexture(512));
        tTao->setFiltering(QSGTexture::Linear);
        sNode->setTexture(tTao);
        sNode->setOwnsTexture(true);
        tNode->appendChildNode(sNode);
        root->appendChildNode(tNode);
    }

    QSGGeometryNode *pNode = static_cast<QSGGeometryNode*>(root->childAtIndex(0));
    QSGTransformNode *tNode = static_cast<QSGTransformNode*>(root->childAtIndex(1));
    QSGSimpleTextureNode *sNode = static_cast<QSGSimpleTextureNode*>(tNode->childAtIndex(0));

    float w = width(), h = height();
    float cx = w/2.0f, cy = h/2.0f, r = qMin(w, h)/4.5f;

    sNode->setRect(-r, -r, r*2, r*2);
    QMatrix4x4 m; 
    m.translate(cx, cy); 
    m.rotate(m_rotation * 180.0f / M_PI, 0, 0, 1);
    tNode->setMatrix(m);

    if (!m_lowCpuMode) {
        QSGGeometry *geo = pNode->geometry();
        int activeCount = qMin(m_particleCount, MAX_PARTICLES);
        geo->allocate(activeCount * 4, activeCount * 6); 

        ParticleVertex *v = (ParticleVertex *)geo->vertexData();
        quint16 *indices = geo->indexDataAsUShort();
        const ParticleData* pData = m_readBuffer->data();

        for (int i = 0; i < activeCount; ++i) {
            const ParticleData &p = pData[i];
            if (p.life > 0) {
                float size = p.size * 15.0f;
                float hs = size / 2.0f;
                unsigned char a = static_cast<unsigned char>(p.life * 255.0f * 0.9f);
                v[0] = {p.x-hs, p.y-hs, 0, 0, 255, 255, 255, a};
                v[1] = {p.x+hs, p.y-hs, 1, 0, 255, 255, 255, a};
                v[2] = {p.x-hs, p.y+hs, 0, 1, 255, 255, 255, a};
                v[3] = {p.x+hs, p.y+hs, 1, 1, 255, 255, 255, a};

                int b = i * 4;
                indices[0]=b; indices[1]=b+1; indices[2]=b+2; 
                indices[3]=b+2; indices[4]=b+1; indices[5]=b+3;
                v += 4; indices += 6;
            } else {
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

QImage TaoQGraphGemini::generateGlowTexture(int s) {
    QImage img(s, s, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter p(&img);
    float radius = (s / 2.0f) - 2.0f; 
    QRadialGradient g(s/2, s/2, radius);
    g.setColorAt(0.0, QColor(255, 255, 255, 255)); 
    g.setColorAt(0.2, QColor(200, 240, 255, 200)); 
    g.setColorAt(1.0, QColor(0, 0, 0, 0));         
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
    float c = s/2.0f, r = s/2.0f - 4.0f; 
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