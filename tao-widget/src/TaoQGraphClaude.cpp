#include "TaoQGraphClaude.h"
#include <QSGGeometryNode>
#include <QSGSimpleTextureNode>
#include <QSGTextureMaterial>
#include <QSGTransformNode>
#include <QPainter>
#include <QQuickWindow>
#include <QtMath>
#include <QTimer>

// ============================================================================
// VERTEX FORMAT
// ============================================================================

struct ParticleVertexClaude {
    float x, y, u, v;
    unsigned char r, g, b, a;
};

static const QSGGeometry::AttributeSet &claudeAttributes() {
    static QSGGeometry::Attribute data[] = {
        QSGGeometry::Attribute::create(0, 2, QSGGeometry::FloatType, true),
        QSGGeometry::Attribute::create(1, 2, QSGGeometry::FloatType),
        QSGGeometry::Attribute::create(2, 4, QSGGeometry::UnsignedByteType)
    };
    static QSGGeometry::AttributeSet attrs = { 3, 20, data };
    return attrs;
}

// ============================================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================================

TaoQGraphClaude::TaoQGraphClaude(QQuickItem *parent) 
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
    
    m_particles.resize(MAX_PARTICLES);
    initParticles();
    
    // Pre-generate textures
    m_glowImage = createGlowTexture(128);
    m_taoImage = createTaoTexture(512);
    
    m_timer.start();
    
    // Animation timer
    QTimer *t = new QTimer(this);
    connect(t, &QTimer::timeout, this, &TaoQGraphClaude::syncAndTask);
    t->start(16);
}

TaoQGraphClaude::~TaoQGraphClaude() {
}

// ============================================================================
// PROPERTIES
// ============================================================================

void TaoQGraphClaude::setParticleCount(int c) { 
    if(m_particleCount != c) { 
        m_particleCount = qBound(0, c, MAX_PARTICLES); 
        Q_EMIT particleCountChanged(); 
    } 
}

void TaoQGraphClaude::setRotationSpeed(float s) { 
    if(!qFuzzyCompare(m_rotationSpeed, s)) { 
        m_rotationSpeed = s; 
        Q_EMIT rotationSpeedChanged(); 
    } 
}

void TaoQGraphClaude::setClockwise(bool c) { 
    if(m_clockwise != c) { 
        m_clockwise = c; 
        Q_EMIT clockwiseChanged(); 
    } 
}

void TaoQGraphClaude::setShowClock(bool s) { 
    if(m_showClock != s) { 
        m_showClock = s; 
        update(); 
        Q_EMIT showClockChanged(); 
    } 
}

void TaoQGraphClaude::setLowCpuMode(bool l) { 
    if(m_lowCpuMode != l) { 
        m_lowCpuMode = l; 
        Q_EMIT lowCpuModeChanged(); 
    } 
}

void TaoQGraphClaude::setThreadCount(int count) {
    if (m_threadCount != count) {
        m_threadCount = qBound(1, count, 16);
        Q_EMIT threadCountChanged();
    }
}

// ============================================================================
// PARTICLE INITIALIZATION
// ============================================================================

void TaoQGraphClaude::initParticles() { 
    for(auto &p : m_particles) {
        p.life = 0;
        p.x = p.y = 0;
        p.vx = p.vy = 0;
        p.decay = 0.003f;
        p.size = 1.0f;
    }
}

// ============================================================================
// ANIMATION UPDATE
// ============================================================================

void TaoQGraphClaude::syncAndTask() {
    float dt = m_timer.elapsed() / 1000.0f; 
    m_timer.restart();
    dt = qMin(dt, 0.033f); 
    
    m_time += dt;
    
    // Rotation
    float dir = m_clockwise ? 1.0f : -1.0f;
    m_rotation += (m_rotationSpeed / 1000.0f) * dir * dt * 60.0f;
    
    // Update particles
    if(!m_lowCpuMode) {
        updateParticlesParallel(dt);
    }
    
    update();
}

// ============================================================================
// TRUE MULTI-THREADED PARTICLE UPDATE
// ============================================================================

void TaoQGraphClaude::updateParticlesParallel(float dt) {
    float w = width(), h = height(); 
    if(w <= 0 || h <= 0) return;
    
    float cx = w/2, cy = h/2, r = qMin(w,h)/4;
    int active = qMin(m_particleCount, MAX_PARTICLES);
    
    // Create range indices for parallel processing
    QVector<int> indices;
    int chunkSize = (active + m_threadCount - 1) / m_threadCount;
    
    for (int i = 0; i < m_threadCount; ++i) {
        indices.append(i);
    }
    
    // Process chunks in parallel
    QtConcurrent::blockingMap(indices, [=](int threadIdx) {
        int start = threadIdx * chunkSize;
        int end = qMin(start + chunkSize, active);
        
        if (start >= end) return;
        
        auto gen = QRandomGenerator::global();
        
        for(int i = start; i < end; ++i) {
            auto &p = m_particles[i];
            
            // Update position
            p.x += p.vx * dt * 60.0f;
            p.y += p.vy * dt * 60.0f;
            p.life -= p.decay * dt * 60.0f;
            
            // Respawn if dead
            if(p.life <= 0) {
                p.life = 1.0f;
                
                float angle = gen->generateDouble() * M_PI * 2.0f;
                float dist = r * (1.1f + gen->generateDouble() * 0.5f);
                
                p.x = cx + std::cos(angle) * dist;
                p.y = cy + std::sin(angle) * dist;
                
                p.vx = (gen->generateDouble() - 0.5f) * 0.5f;
                p.vy = (gen->generateDouble() - 0.5f) * 0.5f;
                
                p.decay = 0.002f + gen->generateDouble() * 0.004f;
                p.size = 0.5f + gen->generateDouble() * 2.0f;
            }
        }
    });
}

// ============================================================================
// SCENE GRAPH UPDATE
// ============================================================================

QSGNode* TaoQGraphClaude::updatePaintNode(QSGNode* old, UpdatePaintNodeData*) {
    QSGNode* root = old;
    
    // Initialize scene graph structure
    if(!root) {
        root = new QSGNode();
        
        // Create GPU textures
        if (window()) {
            m_glowTexture = window()->createTextureFromImage(m_glowImage);
            m_taoTexture = window()->createTextureFromImage(m_taoImage);
        }
        
        // Particle node
        QSGGeometryNode *pn = new QSGGeometryNode();
        QSGGeometry *pg = new QSGGeometry(claudeAttributes(), 0);
        pg->setDrawingMode(QSGGeometry::DrawTriangles);
        pn->setGeometry(pg);
        pn->setFlag(QSGNode::OwnsGeometry);
        
        QSGTextureMaterial *pm = new QSGTextureMaterial();
        pm->setTexture(m_glowTexture);
        pn->setMaterial(pm); 
        pn->setFlag(QSGNode::OwnsMaterial);
        
        root->appendChildNode(pn);

        // Tao symbol node
        QSGTransformNode *tn = new QSGTransformNode();
        QSGSimpleTextureNode *sn = new QSGSimpleTextureNode();
        sn->setTexture(m_taoTexture);
        sn->setOwnsTexture(false);
        sn->setFiltering(QSGTexture::Linear);
        
        tn->appendChildNode(sn); 
        root->appendChildNode(tn);
    }
    
    // Update nodes
    QSGGeometryNode *particleNode = static_cast<QSGGeometryNode*>(root->childAtIndex(0));
    QSGTransformNode *taoTransform = static_cast<QSGTransformNode*>(root->childAtIndex(1));
    QSGSimpleTextureNode *taoNode = static_cast<QSGSimpleTextureNode*>(taoTransform->childAtIndex(0));

    float cx = width()/2, cy = height()/2, r = qMin(width(), height())/4.2f;
    
    // Update Tao symbol
    taoNode->setRect(-r, -r, r*2, r*2);
    QMatrix4x4 m; 
    m.translate(cx, cy); 
    m.rotate(m_rotation * 180.0f / M_PI, 0, 0, 1);
    taoTransform->setMatrix(m);

    // Update particles
    if(!m_lowCpuMode) {
        QSGGeometry *g = particleNode->geometry(); 
        int active = qMin(m_particleCount, MAX_PARTICLES);
        
        g->allocate(active * 4, active * 6);
        
        ParticleVertexClaude *v = (ParticleVertexClaude*)g->vertexData();
        quint16 *idx = g->indexDataAsUShort();
        
        for(int i = 0; i < active; ++i) {
            const auto &p = m_particles[i];
            
            float s = p.size * 12.0f;
            float hs = s / 2.0f;
            
            unsigned char alpha = p.life > 0 ? (unsigned char)(p.life * 200) : 0;
            unsigned char red = 136, green = 204, blue = 255; // Cyan
            
            // Assign vertices properly
            v[0].x = p.x - hs; v[0].y = p.y - hs; v[0].u = 0; v[0].v = 0; 
            v[0].r = red; v[0].g = green; v[0].b = blue; v[0].a = alpha;
            
            v[1].x = p.x + hs; v[1].y = p.y - hs; v[1].u = 1; v[1].v = 0;
            v[1].r = red; v[1].g = green; v[1].b = blue; v[1].a = alpha;
            
            v[2].x = p.x - hs; v[2].y = p.y + hs; v[2].u = 0; v[2].v = 1;
            v[2].r = red; v[2].g = green; v[2].b = blue; v[2].a = alpha;
            
            v[3].x = p.x + hs; v[3].y = p.y + hs; v[3].u = 1; v[3].v = 1;
            v[3].r = red; v[3].g = green; v[3].b = blue; v[3].a = alpha;
            
            int base = i * 4;
            idx[0] = base;     idx[1] = base + 1; idx[2] = base + 2;
            idx[3] = base + 2; idx[4] = base + 1; idx[5] = base + 3;
            
            v += 4;
            idx += 6;
        }
        
        particleNode->markDirty(QSGNode::DirtyGeometry);
    } else {
        particleNode->geometry()->allocate(0, 0);
    }
    
    return root;
}

// ============================================================================
// TEXTURE GENERATION
// ============================================================================

QImage TaoQGraphClaude::createGlowTexture(int s) {
    QImage img(s, s, QImage::Format_ARGB32_Premultiplied); 
    img.fill(Qt::transparent);
    
    float center = s / 2.0f;
    
    for (int y = 0; y < s; ++y) {
        for (int x = 0; x < s; ++x) {
            float dx = x - center;
            float dy = y - center;
            float dist = std::sqrt(dx*dx + dy*dy);
            
            float alpha = (dist < center) ? std::pow(1.0f - dist/center, 4.0f) : 0.0f;
            
            img.setPixelColor(x, y, QColor(255, 255, 255, (int)(alpha * 255)));
        }
    }
    
    return img;
}

QImage TaoQGraphClaude::createTaoTexture(int s) {
    QImage img(s, s, QImage::Format_ARGB32_Premultiplied); 
    img.fill(Qt::transparent);
    
    QPainter p(&img); 
    p.setRenderHint(QPainter::Antialiasing);
    
    float center = s / 2.0f;
    float radius = s / 2.0f - 2;
    
    // Full white circle
    p.setBrush(QColor(245, 245, 245)); 
    p.setPen(Qt::NoPen);
    p.drawEllipse(QRectF(center - radius, center - radius, radius * 2, radius * 2));
    
    // Black half
    p.setBrush(QColor(20, 20, 20)); 
    p.drawPie(QRectF(center - radius, center - radius, radius * 2, radius * 2), 
              -90 * 16, 180 * 16);
    
    // Yin/yang curves
    p.setBrush(QColor(20, 20, 20)); 
    p.drawEllipse(QRectF(center - radius/2, center - radius, radius, radius));
    
    p.setBrush(QColor(245, 245, 245)); 
    p.drawEllipse(QRectF(center - radius/2, center, radius, radius));
    
    // Eyes
    p.setBrush(QColor(255, 255, 255)); 
    p.drawEllipse(QRectF(center - radius/8, center - radius * 0.625f, radius/4, radius/4));
    
    p.setBrush(QColor(0, 0, 0)); 
    p.drawEllipse(QRectF(center - radius/8, center + radius * 0.375f, radius/4, radius/4));
    
    return img;
}
