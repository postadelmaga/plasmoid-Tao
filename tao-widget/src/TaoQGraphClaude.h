#ifndef TAOQGRAPHCLAUDE_H
#define TAOQGRAPHCLAUDE_H

#include <QObject>
#include <QQuickItem>
#include <QSGNode>
#include <QVector>
#include <QElapsedTimer>
#include <QtConcurrent>
#include <QRandomGenerator>
#include <QSGTexture>

struct ParticleClaude {
    float x, y;
    float vx, vy;
    float life;
    float decay;
    float size;
    float padding; // GPU alignment
};

class TaoQGraphClaude : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(int particleCount READ particleCount WRITE setParticleCount NOTIFY particleCountChanged)
    Q_PROPERTY(float rotationSpeed READ rotationSpeed WRITE setRotationSpeed NOTIFY rotationSpeedChanged)
    Q_PROPERTY(bool clockwise READ clockwise WRITE setClockwise NOTIFY clockwiseChanged)
    Q_PROPERTY(bool showClock READ showClock WRITE setShowClock NOTIFY showClockChanged)
    Q_PROPERTY(bool lowCpuMode READ lowCpuMode WRITE setLowCpuMode NOTIFY lowCpuModeChanged)
    Q_PROPERTY(int threadCount READ threadCount WRITE setThreadCount NOTIFY threadCountChanged)
    QML_ELEMENT

public:
    explicit TaoQGraphClaude(QQuickItem *parent = nullptr);
    ~TaoQGraphClaude() override;

    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* data) override;
    
    // Getters
    int particleCount() const { return m_particleCount; }
    float rotationSpeed() const { return m_rotationSpeed; }
    bool clockwise() const { return m_clockwise; }
    bool showClock() const { return m_showClock; }
    bool lowCpuMode() const { return m_lowCpuMode; }
    int threadCount() const { return m_threadCount; }
    
    // Setters
    void setParticleCount(int count);
    void setRotationSpeed(float speed);
    void setClockwise(bool clockwise);
    void setShowClock(bool show);
    void setLowCpuMode(bool lowCpu);
    void setThreadCount(int count);

Q_SIGNALS:
    void particleCountChanged();
    void rotationSpeedChanged();
    void clockwiseChanged();
    void showClockChanged();
    void lowCpuModeChanged();
    void threadCountChanged();

private Q_SLOTS:
    void syncAndTask();

private:
    void initParticles();
    void updateParticlesParallel(float dt);
    
    QImage createGlowTexture(int size);
    QImage createTaoTexture(int size);
    
    // Properties
    int m_particleCount = 2000;
    float m_rotationSpeed = 5.0f;
    bool m_clockwise = true;
    bool m_showClock = false;
    bool m_lowCpuMode = false;
    int m_threadCount = 4;

    // State
    QVector<ParticleClaude> m_particles;
    float m_rotation = 0.0f;
    float m_time = 0.0f;
    QElapsedTimer m_timer;
    
    // Texture caching
    QSGTexture* m_glowTexture = nullptr;
    QSGTexture* m_taoTexture = nullptr;
    QImage m_glowImage;
    QImage m_taoImage;
    
    const int MAX_PARTICLES = 5000;
};

#endif // TAOQGRAPHCLAUDE_H