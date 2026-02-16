#include "TaoQuickItem.h"
#include <QPainterPath>
#include <QDateTime>
#include <QGuiApplication>

TaoQuickItem::TaoQuickItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    // Enable continuous rendering via Timer
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &TaoQuickItem::updateAnimation);
    m_timer->start(16); // ~60 FPS

    // Perform initial texture baking
    bakeTextures();
    
    // Resize particles vector
    m_particles.resize(MAX_PARTICLES);
    initParticles();
    
    setAntialiasing(true);
}

TaoQuickItem::~TaoQuickItem()
{
}

void TaoQuickItem::setParticleCount(int count) {
    if (m_particleCount == count) return;
    m_particleCount = qBound(0, count, MAX_PARTICLES);
    Q_EMIT particleCountChanged();
}

void TaoQuickItem::setRotationSpeed(float speed) {
    if (qFuzzyCompare(m_rotationSpeed, speed)) return;
    m_rotationSpeed = speed;
    Q_EMIT rotationSpeedChanged();
}

void TaoQuickItem::setClockwise(bool clockwise) {
    if (m_clockwise == clockwise) return;
    m_clockwise = clockwise;
    Q_EMIT clockwiseChanged();
}

void TaoQuickItem::setShowClock(bool show) {
    if (m_showClock == show) return;
    m_showClock = show;
    Q_EMIT showClockChanged();
}

void TaoQuickItem::setLowCpuMode(bool lowCpu) {
    if (m_lowCpuMode == lowCpu) return;
    m_lowCpuMode = lowCpu;
    Q_EMIT lowCpuModeChanged();
}

void TaoQuickItem::initParticles() {
    // Initial random placement
    for(auto& p : m_particles) {
        p.life = 0; // Start dead to respawn naturally
    }
}

QImage TaoQuickItem::makeGlow(int s) {
    QImage img(s, s, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    
    float c = s / 2.0f;
    for (int y = 0; y < s; ++y) {
        for (int x = 0; x < s; ++x) {
            float dx = (x - c);
            float dy = (y - c);
            float d = std::sqrt(dx*dx + dy*dy);
            float a = (d < c) ? std::pow(1.0f - d/c, 4.0f) : 0.0f;
            img.setPixelColor(x, y, QColor(255, 255, 255, (int)(a * 255)));
        }
    }
    return img;
}

void TaoQuickItem::bakeTextures() {
    m_texGlow = makeGlow(32);
    
    // Tao texture baked at fixed size (1024x1024) for quality
    int s = 1024;
    m_texTao = QImage(s, s, QImage::Format_ARGB32_Premultiplied);
    m_texTao.fill(Qt::transparent);
    
    QPainter p(&m_texTao);
    p.setRenderHint(QPainter::Antialiasing);
    
    float c = s / 2.0f;
    float r = s / 2.0f - 2;

    // Helper to draw circle
    auto drawCircle = [&](float cx, float cy, float rad, float startAngle, float spanAngle, QColor col) {
        p.setBrush(col);
        p.setPen(Qt::NoPen);
        // Qt angles are in 1/16th degrees. 0 is 3 o'clock. + is CCW (default).
        // Standard mathematical angles used in SDL logic.
        // startAngle/spanAngle in radians.
        p.drawPie(QRectF(cx-rad, cy-rad, rad*2, rad*2), 
                  (int)(startAngle * 180.0 / M_PI * 16), 
                  (int)(spanAngle * 180.0 / M_PI * 16));
    };

    // Full white circle background
    drawCircle(c, c, r, 0, M_PI * 2, QColor(235, 235, 235));
    // Black half
    drawCircle(c, c, r, -M_PI/2, M_PI, QColor(10, 10, 10)); // Start -90 (top), span 180 (down to bottom)
    
    // Small circles for yin/yang curve
    drawCircle(c, c - r/2, r/2, 0, M_PI * 2, QColor(10, 10, 10)); // Top black
    drawCircle(c, c + r/2, r/2, 0, M_PI * 2, QColor(235, 235, 235)); // Bottom white
    
    // Eyes
    drawCircle(c, c - r/2, r/7, 0, M_PI * 2, QColor(255, 255, 255)); // Top eye white
    drawCircle(c, c + r/2, r/7, 0, M_PI * 2, QColor(0, 0, 0)); // Bottom eye black
}


void TaoQuickItem::updateAnimation() {
    // Logic update
    float dt = 0.016f; // Fixed step
    m_time += dt;
    
    // Rotation logic
    float dir = m_clockwise ? 1.0f : -1.0f;
    // Harmonized with JS version: rotationSpeed / 1000 Radians per frame
    m_rotation += (m_rotationSpeed / 1000.0f) * dir;

    // Particle Logic
    float cx = width() / 2.0f;
    float cy = height() / 2.0f;
    float r = qMin(width(), height()) / 4.0f;

    // Random generator
    auto gen = QRandomGenerator::global();

    int activeCount = qMin(m_particleCount, MAX_PARTICLES);
    
    // Update loop
    for (int i = 0; i < activeCount; ++i) {
        Particle& p = m_particles[i];
        
        p.x += p.vx * FLOW_SPEED;
        p.y += p.vy * FLOW_SPEED;
        p.life -= p.decay * FLOW_SPEED;
        
        if (p.life <= 0) {
            p.life = 1.0f;
            // Respawn in ring
            float angle = gen->generateDouble() * M_PI * 2.0f;
            float dist = r * (1.1f + gen->generateDouble() * 0.5f);
            
            p.x = cx + std::cos(angle) * dist;
            p.y = cy + std::sin(angle) * dist;
            
            // Velocity (harmonized with JS range [-0.2, 0.2])
            p.vx = (gen->generateDouble() - 0.5f) * 0.4f;
            p.vy = (gen->generateDouble() - 0.5f) * 0.4f;
            
            p.decay = 0.002f + gen->generateDouble() * 0.003f;
            p.size = 0.5f + gen->generateDouble() * 1.5f;
        }
    }

    // Trigger repaint
    update();
}

void TaoQuickItem::paint(QPainter *painter) {
    if (!painter) return;

    painter->setRenderHint(QPainter::Antialiasing); // Optional, maybe slower
    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    // Clear background handled by Plasma usually, but we want transparency
    // painter->eraseRect(boundingRect()); // Typically not needed if transparent
    
    float cx = width() / 2.0f;
    float cy = height() / 2.0f;
    float r = qMin(width(), height()) / 4.5f;

    // 1. PARTICLES (Behind)
    if (!m_lowCpuMode) {
        float glowInt = 0.8f; // Fixed from config for now
        
        // Setup composition for glow
        painter->setCompositionMode(QPainter::CompositionMode_Plus); // Additive blending
        
        // We can batch draw or draw individually. QImage drawing is fast-ish.
        // Color modulation
        // QPainter doesn't have direct color mod for pixmaps easily without overhead.
        // But we can use opacity.
        // The texture is white. We want Cyan/White.
        // Efficient way: Pre-color the texture or use QBrush.
        // For now, raw drawing.
        
        int activeCount = qMin(m_particleCount, MAX_PARTICLES);
        for (int i = 0; i < activeCount; ++i) {
            const Particle& p = m_particles[i];
            if (p.life > 0) {
                 float alpha = p.life * glowInt * 0.8; // scaling
                 if (alpha > 1.0f) alpha = 1.0f;
                 painter->setOpacity(alpha);
                 
                 float s = p.size * 15.0f; // Scale factor matches SDL
                 // Apply a tint if possible? Qt's drawImage is just the image.
                 // To tint, we'd need a colored texture.
                 // Let's assume white glow is fine, or pre-tint m_texGlow to Cyan.
                 // Re-using m_texGlow (white)
                 painter->drawImage(QRectF(p.x - s/2, p.y - s/2, s, s), m_texGlow);
            }
        }
    }

    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter->setOpacity(1.0);

    // 2. ORBITING LIGHTS
    if (!m_lowCpuMode) {
        // ... (Simplified for brevity, can enable if critical)
    }

    // 3. TAO (Center)
    painter->save();
    painter->translate(cx, cy);
    
    // Tao Symbol
    painter->save();
    painter->rotate(m_rotation * 180.0 / M_PI); // Rad to Deg
    painter->drawImage(QRectF(-r, -r, r*2, r*2), m_texTao);
    painter->restore();

    // Extra decoration (Satellites & Aura)
    if (!m_lowCpuMode) {
        float pulse = std::sin(m_time * 3.0f) * 0.15f + 0.85f;

        // Aura ring
        painter->setOpacity(0.25f * pulse);
        painter->setPen(QPen(QColor(136, 204, 255), 1.5, Qt::SolidLine, Qt::RoundCap));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(QPointF(0, 0), r + 40, r + 40);

        // Satellites
        painter->setOpacity(0.45f);
        for (int j = 0; j < 4; ++j) {
            float sa2 = (m_time * 1.25f) + (j * M_PI / 2.0f);
            painter->setBrush(j % 2 == 0 ? QColor(136, 204, 255) : QColor(255, 170, 136));
            painter->setPen(Qt::NoPen);
            painter->drawEllipse(QPointF(std::cos(sa2) * (r + 25), std::sin(sa2) * (r + 25)), 2.5, 2.5);
        }
    }
    painter->restore();

    // 4. CLOCK (Overlay)
    if (m_showClock) {
        QDateTime now = QDateTime::currentDateTime();
        QTime t = now.time();
        
        float h = t.hour() % 12 + t.minute() / 60.0f;
        float m = t.minute() + t.second() / 60.0f;
        float s = t.second() + t.msec() / 1000.0f;

        painter->setPen(QPen(QColor(255, 255, 255, 200), 5, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(QPointF(cx, cy), QPointF(cx + std::cos((h/12.0*M_PI*2)-M_PI/2.0)*r*0.5, cy + std::sin((h/12.0*M_PI*2)-M_PI/2.0)*r*0.5));
            
        // Simplified lines for minutes/seconds...
        // Minutes
        painter->setPen(QPen(QColor(136, 204, 255, 200), 3, Qt::SolidLine, Qt::RoundCap));
        float ma = (m / 60.0f) * M_PI * 2 - M_PI_2;
        painter->drawLine(QPointF(cx, cy), QPointF(cx + std::cos(ma)*r*0.8, cy + std::sin(ma)*r*0.8));

        // Seconds
        painter->setPen(QPen(QColor(255, 170, 136, 200), 1.5, Qt::SolidLine, Qt::RoundCap));
        float sa = (s / 60.0f) * M_PI * 2 - M_PI_2;
        painter->drawLine(QPointF(cx, cy), QPointF(cx + std::cos(sa)*r*0.9, cy + std::sin(sa)*r*0.9));
        
        // Center Dot
        painter->setBrush(Qt::white);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(cx, cy), 3, 3);
    }
}
