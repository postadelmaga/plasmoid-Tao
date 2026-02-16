#ifndef TAOQGRAPHYBRID_H
#define TAOQGRAPHYBRID_H

#include <QQuickItem>
#include <QTimer>
#include <QVector>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QSGTexture>

struct ParticleHybrid {
    float x, y;
    float vx, vy;
    float life;
    float decay;
    float size;
};

class TaoQGraphHybrid : public QQuickItem
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
    explicit TaoQGraphHybrid(QQuickItem *parent = nullptr);
    ~TaoQGraphHybrid() override;

    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;

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
    int threadCount() const { return m_threadCount; }
    void setThreadCount(int count);

Q_SIGNALS:
    void particleCountChanged();
    void rotationSpeedChanged();
    void clockwiseChanged();
    void showClockChanged();
    void lowCpuModeChanged();
    void threadCountChanged();

private Q_SLOTS:
    void checkSimulation();

private:
    void initParticles();
    QImage generateGlowTexture(int s);
    QImage generateTaoTexture(int s);

    // Config
    int m_particleCount = 2000;
    float m_rotationSpeed = 5.0f;
    bool m_clockwise = true;
    bool m_showClock = false;
    bool m_lowCpuMode = false;
    int m_threadCount = 4;

    // Buffers
    QVector<ParticleHybrid> m_particlesA;
    QVector<ParticleHybrid> m_particlesB;
    QVector<ParticleHybrid>* m_readBuffer;
    QVector<ParticleHybrid>* m_writeBuffer;
    
    // Sync
    bool m_simulationRunning = false;
    float m_rotation = 0.0f;
    QFutureWatcher<void> m_watcher;
    
    const int MAX_PARTICLES = 10000; // Increased for hybrid testing
};

#endif
