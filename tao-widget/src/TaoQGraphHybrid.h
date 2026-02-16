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
    Q_PROPERTY(QColor hourHandColor READ hourHandColor WRITE setHourHandColor NOTIFY hourHandColorChanged)
    Q_PROPERTY(QColor minuteHandColor READ minuteHandColor WRITE setMinuteHandColor NOTIFY minuteHandColorChanged)
    Q_PROPERTY(QColor secondHandColor READ secondHandColor WRITE setSecondHandColor NOTIFY secondHandColorChanged)
    Q_PROPERTY(QColor glowColor1 READ glowColor1 WRITE setGlowColor1 NOTIFY glowColor1Changed)
    Q_PROPERTY(double glowSize1 READ glowSize1 WRITE setGlowSize1 NOTIFY glowSize1Changed)
    Q_PROPERTY(QColor glowColor2 READ glowColor2 WRITE setGlowColor2 NOTIFY glowColor2Changed)
    Q_PROPERTY(double glowSize2 READ glowSize2 WRITE setGlowSize2 NOTIFY glowSize2Changed)
    Q_PROPERTY(QColor particleColor1 READ particleColor1 WRITE setParticleColor1 NOTIFY particleColor1Changed)
    Q_PROPERTY(QColor particleColor2 READ particleColor2 WRITE setParticleColor2 NOTIFY particleColor2Changed)
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

    QColor hourHandColor() const { return m_hourHandColor; }
    void setHourHandColor(const QColor &c);

    QColor minuteHandColor() const { return m_minuteHandColor; }
    void setMinuteHandColor(const QColor &c);

    QColor secondHandColor() const { return m_secondHandColor; }
    void setSecondHandColor(const QColor &c);

    QColor glowColor1() const { return m_glowColor1; }
    void setGlowColor1(const QColor &c);

    double glowSize1() const { return m_glowSize1; }
    void setGlowSize1(double s);

    QColor glowColor2() const { return m_glowColor2; }
    void setGlowColor2(const QColor &c);

    double glowSize2() const { return m_glowSize2; }
    void setGlowSize2(double s);

    QColor particleColor1() const { return m_particleColor1; }
    void setParticleColor1(const QColor &c);

    QColor particleColor2() const { return m_particleColor2; }
    void setParticleColor2(const QColor &c);

Q_SIGNALS:
    void particleCountChanged();
    void rotationSpeedChanged();
    void clockwiseChanged();
    void showClockChanged();
    void lowCpuModeChanged();
    void mousePosChanged();
    void hourHandColorChanged();
    void minuteHandColorChanged();
    void secondHandColorChanged();
    void glowColor1Changed();
    void glowSize1Changed();
    void glowColor2Changed();
    void glowSize2Changed();
    void particleColor1Changed();
    void particleColor2Changed();

private:
    void updateSimulation();
    QImage generateGlowTexture(int s, const QColor &color);
    QImage generateTaoTexture(int s);
    void setupGeometryIndices(QSGGeometry *geo, int count);

    // Configuration
    int m_particleCount = 80;
    float m_rotationSpeed = 5.0f;
    bool m_clockwise = true;
    bool m_showClock = false;
    bool m_lowCpuMode = false;
    QPointF m_mousePos;
    QColor m_hourHandColor = Qt::white;
    QColor m_minuteHandColor = QColor("#88ccff");
    QColor m_secondHandColor = QColor("#ffaa88");
    QColor m_glowColor1 = QColor("#64c8ff");
    double m_glowSize1 = 1.3;
    QColor m_glowColor2 = QColor("#64c8ff");
    double m_glowSize2 = 0.0;
    QColor m_particleColor1 = QColor("#7fcdff");
    QColor m_particleColor2 = QColor("#ffaa00");

    // Simulation State
    std::vector<ParticleData> m_particles;       // Back buffer (thread simulazione)
    std::vector<ParticleData> m_particlesRender; // Front buffer (thread render)
    float m_rotation = 0.0f;
    QElapsedTimer m_timeTracker;
    qint64 m_lastTime = 0;
    float m_lastDt = 0.016f;
    int m_renderCount = 0;
    QColor m_lastGlowColor1;
    QColor m_lastGlowColor2;

    // Async handling
    QFutureWatcher<void> m_watcher;
    bool m_simulationPending = false;
    
    const int MAX_PARTICLES = 20000;
};

#endif