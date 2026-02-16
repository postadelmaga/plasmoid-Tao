#ifndef TAOQGRAPHYBRID_H
#define TAOQGRAPHYBRID_H

#include <QQuickItem>
#include <QFuture>
#include <QFutureWatcher>
#include <QElapsedTimer>
#include <vector>
#include <QSGNode>
#include <QSGGeometry>

// Struttura ottimizzata per la simulazione
struct ParticleData {
    float x, y;
    float vx, vy;
    float life;
    float decay;
    float size;
    quint32 packedColor; // Colore pre-calcolato (ARGB/ABGR)
};

class TaoQGraphHybrid : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(int particleCount READ particleCount WRITE setParticleCount NOTIFY particleCountChanged)
    Q_PROPERTY(float rotationSpeed READ rotationSpeed WRITE setRotationSpeed NOTIFY rotationSpeedChanged)
    Q_PROPERTY(bool clockwise READ clockwise WRITE setClockwise NOTIFY clockwiseChanged)
    Q_PROPERTY(bool showClock READ showClock WRITE setShowClock NOTIFY showClockChanged)
    Q_PROPERTY(bool lowCpuMode READ lowCpuMode WRITE setLowCpuMode NOTIFY lowCpuModeChanged)
    Q_PROPERTY(QPointF mousePos READ mousePos WRITE setMousePos NOTIFY mousePosChanged)
    QML_ELEMENT

public:
    explicit TaoQGraphHybrid(QQuickItem *parent = nullptr);
    ~TaoQGraphHybrid() override;

    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;
    void itemChange(ItemChange change, const ItemChangeData &value) override;

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

    QPointF mousePos() const { return m_mousePos; }
    void setMousePos(const QPointF &pos);

Q_SIGNALS:
    void particleCountChanged();
    void rotationSpeedChanged();
    void clockwiseChanged();
    void showClockChanged();
    void lowCpuModeChanged();
    void mousePosChanged();

private:
    void updateSimulation();
    QImage generateGlowTexture(int s);
    QImage generateTaoTexture(int s);
    void setupGeometryIndices(QSGGeometry *geo, int count);

    // Configuration
    int m_particleCount = 80;
    float m_rotationSpeed = 5.0f;
    bool m_clockwise = true;
    bool m_showClock = false;
    bool m_lowCpuMode = false;
    QPointF m_mousePos;

    // Simulation State
    std::vector<ParticleData> m_particles;       // Back buffer (thread simulazione)
    std::vector<ParticleData> m_particlesRender; // Front buffer (thread render)
    float m_rotation = 0.0f;
    QElapsedTimer m_timeTracker;
    qint64 m_lastTime = 0;
    float m_lastDt = 0.016f;
    int m_renderCount = 0;

    // Async handling
    QFutureWatcher<void> m_watcher;
    bool m_simulationPending = false;
    
    const int MAX_PARTICLES = 20000;
};

#endif