#ifndef TAOQUICKITEM_H
#define TAOQUICKITEM_H

#include <QQuickPaintedItem>
#include <QTimer>
#include <QImage>
#include <QVector>
#include <QPainter>
#include <QtMath>
#include <QRandomGenerator>

struct Particle {
    float x, y;     // Position
    float vx, vy;   // Velocity
    float life;     // Life 0.0 - 1.0
    float decay;    // Die speed
    float size;     // Size
};

class TaoQuickItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(int particleCount READ particleCount WRITE setParticleCount NOTIFY particleCountChanged)
    Q_PROPERTY(float rotationSpeed READ rotationSpeed WRITE setRotationSpeed NOTIFY rotationSpeedChanged)
    Q_PROPERTY(bool clockwise READ clockwise WRITE setClockwise NOTIFY clockwiseChanged)
    Q_PROPERTY(bool showClock READ showClock WRITE setShowClock NOTIFY showClockChanged)
    Q_PROPERTY(bool lowCpuMode READ lowCpuMode WRITE setLowCpuMode NOTIFY lowCpuModeChanged)
    QML_ELEMENT

public:
    explicit TaoQuickItem(QQuickItem *parent = nullptr);
    ~TaoQuickItem() override;

    void paint(QPainter *painter) override;

    int particleCount() const { return m_particleCount; }
    void setParticleCount(int count);

    float rotationSpeed() const { return m_rotationSpeed; }
    void setRotationSpeed(float speed);

    bool clockwise() const { return m_clockwise; }
    void setClockwise(bool clockwise);

    bool showClock() const { return m_showClock; }
    void setShowClock(bool show);

    bool lowCpuMode() const { return m_lowCpuMode; }
    void setLowCpuMode(bool lowCpu);

Q_SIGNALS:
    void particleCountChanged();
    void rotationSpeedChanged();
    void clockwiseChanged();
    void showClockChanged();
    void lowCpuModeChanged();

private Q_SLOTS:
    void updateAnimation();

private:
    void initParticles();
    void bakeTextures();
    QImage makeGlow(int s);
    void drawTao(QPainter* painter, float cx, float cy, float r, float rot);

    // Properties
    int m_particleCount = 1200;
    float m_rotationSpeed = 1.0f;
    bool m_clockwise = true;
    bool m_showClock = false;
    bool m_lowCpuMode = false;

    // Interior
    QVector<Particle> m_particles;
    QImage m_texGlow;
    QImage m_texTao;
    QTimer *m_timer;
    float m_rotation = 0.0f;
    float m_time = 0.0f;
    
    // Constants
    const int MAX_PARTICLES = 3000;
    const float FLOW_SPEED = 1.0f;
};

#endif // TAOQUICKITEM_H
