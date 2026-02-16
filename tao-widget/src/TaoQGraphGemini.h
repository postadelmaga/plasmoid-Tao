#ifndef TAOQGRAPHGEMINI_H
#define TAOQGRAPHGEMINI_H

#include <QQuickItem>
#include <QTimer>
#include <QVector>
#include <QMutex>
#include <QRunnable>
#include <QSGTexture>

struct ParticleData {
    float x, y;
    float vx, vy;
    float life;
    float decay;
    float size;
};

// Worker per calcolo parallelo
class ParticleWorker : public QRunnable {
public:
    void run() override;
    
    // Input
    float width, height, dt, rotation;
    int count;
    bool clockwise;
    
    // Data Buffers
    QVector<ParticleData> *particles; // Puntatore al buffer da scrivere
};

class TaoQGraphGemini : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(int particleCount READ particleCount WRITE setParticleCount NOTIFY particleCountChanged)
    Q_PROPERTY(float rotationSpeed READ rotationSpeed WRITE setRotationSpeed NOTIFY rotationSpeedChanged)
    Q_PROPERTY(bool clockwise READ clockwise WRITE setClockwise NOTIFY clockwiseChanged)
    Q_PROPERTY(bool showClock READ showClock WRITE setShowClock NOTIFY showClockChanged)
    Q_PROPERTY(bool lowCpuMode READ lowCpuMode WRITE setLowCpuMode NOTIFY lowCpuModeChanged)
    QML_ELEMENT

public:
    explicit TaoQGraphGemini(QQuickItem *parent = nullptr);
    ~TaoQGraphGemini() override;

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

Q_SIGNALS:
    void particleCountChanged();
    void rotationSpeedChanged();
    void clockwiseChanged();
    void showClockChanged();
    void lowCpuModeChanged();

private Q_SLOTS:
    void checkSimulation(); // Controlla se il thread ha finito

private:
    void initParticles();
    QImage generateGlowTexture(int s);
    QImage generateTaoTexture(int s);

    // Configurazione
    int m_particleCount = 1000;
    float m_rotationSpeed = 5.0f;
    bool m_clockwise = true;
    bool m_showClock = false;
    bool m_lowCpuMode = false;

    // Double Buffering per Threading
    QVector<ParticleData> m_particlesA;
    QVector<ParticleData> m_particlesB;
    QVector<ParticleData>* m_readBuffer;  // Letto da updatePaintNode
    QVector<ParticleData>* m_writeBuffer; // Scritto dal Worker
    
    // Sincronizzazione
    ParticleWorker *m_currentWorker = nullptr;
    bool m_simulationRunning = false;
    float m_rotation = 0.0f;
    
    const int MAX_PARTICLES = 5000;
};

#endif