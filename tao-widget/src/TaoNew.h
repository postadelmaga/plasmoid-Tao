#ifndef TAONEW_H
#define TAONEW_H

#include <QQuickItem>
#include <QFutureWatcher>
#include <QElapsedTimer>
#include <QSGNode>
#include <QSGGeometryNode>
#include <QSGSimpleTextureNode>
#include <QSGTransformNode>
#include <QSGGeometry>
#include <QSGMaterial>
#include <QSGMaterialShader>
#include <QSGTexture>
#include <atomic>
#include <vector>

// ── Strutture dati particelle ─────────────────────────────────────────────────

struct ParticleData {
    float   x, y;
    float   vx, vy;
    float   life;
    float   decay;
    float   size;
    quint32 packedColor;
};

struct ParticleVertex {
    float   x, y;
    float   size;
    quint32 color;
};

// ── ParticleMaterial ──────────────────────────────────────────────────────────

class ParticleMaterial : public QSGMaterial
{
public:
    ParticleMaterial();
    QSGMaterialType   *type()                                           const override;
    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode)   const override;
};

// ── TaoNew ────────────────────────────────────────────────────────────────────

class TaoNew : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

    // Particelle
    Q_PROPERTY(int    particleCount  READ particleCount  WRITE setParticleCount  NOTIFY particleCountChanged)
    Q_PROPERTY(QColor particleColor1 READ particleColor1 WRITE setParticleColor1 NOTIFY particleColor1Changed)
    Q_PROPERTY(QColor particleColor2 READ particleColor2 WRITE setParticleColor2 NOTIFY particleColor2Changed)
    Q_PROPERTY(double particleSize   READ particleSize   WRITE setParticleSize   NOTIFY particleSizeChanged)
    Q_PROPERTY(double particleSizeRandom READ particleSizeRandom WRITE setParticleSizeRandom NOTIFY particleSizeRandomChanged)

    // Rotazione
    Q_PROPERTY(float rotationSpeed READ rotationSpeed WRITE setRotationSpeed NOTIFY rotationSpeedChanged)
    Q_PROPERTY(bool  clockwise     READ clockwise     WRITE setClockwise     NOTIFY clockwiseChanged)

    // Glow
    Q_PROPERTY(QColor glowColor1 READ glowColor1 WRITE setGlowColor1 NOTIFY glowColor1Changed)
    Q_PROPERTY(double glowSize1  READ glowSize1  WRITE setGlowSize1  NOTIFY glowSize1Changed)
    Q_PROPERTY(QColor glowColor2 READ glowColor2 WRITE setGlowColor2 NOTIFY glowColor2Changed)
    Q_PROPERTY(double glowSize2  READ glowSize2  WRITE setGlowSize2  NOTIFY glowSize2Changed)

    // Orologio
    Q_PROPERTY(bool   showClock      READ showClock      WRITE setShowClock      NOTIFY showClockChanged)
    Q_PROPERTY(QColor hourHandColor  READ hourHandColor  WRITE setHourHandColor  NOTIFY hourHandColorChanged)
    Q_PROPERTY(QColor minuteHandColor READ minuteHandColor WRITE setMinuteHandColor NOTIFY minuteHandColorChanged)
    Q_PROPERTY(QColor secondHandColor READ secondHandColor WRITE setSecondHandColor NOTIFY secondHandColorChanged)

    // Input
    Q_PROPERTY(QPointF mousePos READ mousePos WRITE setMousePos NOTIFY mousePosChanged)

public:
    explicit TaoNew(QQuickItem *parent = nullptr);
    ~TaoNew() override;

    // Getters
    int     particleCount()   const { return m_particleCount; }
    QColor  particleColor1()  const { return m_particleColor1; }
    QColor  particleColor2()  const { return m_particleColor2; }
    float   rotationSpeed()   const { return m_rotationSpeed; }
    bool    clockwise()       const { return m_clockwise; }
    QColor  glowColor1()      const { return m_glowColor1; }
    double  glowSize1()       const { return m_glowSize1; }
    QColor  glowColor2()      const { return m_glowColor2; }
    double  glowSize2()       const { return m_glowSize2; }
    bool    showClock()       const { return m_showClock; }
    QColor  hourHandColor()   const { return m_hourHandColor; }
    QColor  minuteHandColor() const { return m_minuteHandColor; }
    QColor  secondHandColor() const { return m_secondHandColor; }
    double  particleSize()    const { return m_particleSize; }
    double  particleSizeRandom() const { return m_particleSizeRandom; }
    QPointF mousePos()        const { return m_mousePos; }

    // Setters
    void setParticleCount  (int count);
    void setParticleColor1 (const QColor &c);
    void setParticleColor2 (const QColor &c);
    void setRotationSpeed  (float speed);
    void setClockwise      (bool clockwise);
    void setGlowColor1     (const QColor &c);
    void setGlowSize1      (double s);
    void setGlowColor2     (const QColor &c);
    void setGlowSize2      (double s);
    void setShowClock      (bool show);
    void setHourHandColor  (const QColor &c);
    void setMinuteHandColor(const QColor &c);
    void setSecondHandColor(const QColor &c);
    void setParticleSize(double s);
    void setParticleSizeRandom(double s);
    void setMousePos       (const QPointF &pos);

Q_SIGNALS:
    void particleCountChanged();
    void particleColor1Changed();
    void particleColor2Changed();
    void rotationSpeedChanged();
    void clockwiseChanged();
    void glowColor1Changed();
    void glowSize1Changed();
    void glowColor2Changed();
    void glowSize2Changed();
    void showClockChanged();
    void hourHandColorChanged();
    void minuteHandColorChanged();
    void secondHandColorChanged();
    void particleSizeChanged();
    void particleSizeRandomChanged();
    void mousePosChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;
    void     itemChange(ItemChange change, const ItemChangeData &value) override;

private:
    // ── Costanti ──────────────────────────────────────────────────────────────
    static constexpr int MAX_PARTICLES = 3000;

    // ── Metodi privati ────────────────────────────────────────────────────────
    void   updateSimulation();
    QImage generateGlowTexture(int size, const QColor &color, qreal dpr = 1.0);
    QImage generateTaoTexture (int size, qreal dpr = 1.0);

    // ── Proprietà configurabili ───────────────────────────────────────────────
    int     m_particleCount   = 80;
    QColor  m_particleColor1  = QColor("#7fcdff");
    QColor  m_particleColor2  = QColor("#ffaa00");
    double  m_particleSize     = 2.0;
    double  m_particleSizeRandom = 8.0;

    float   m_rotationSpeed   = 5.0f;
    bool    m_clockwise       = true;

    QColor  m_glowColor1      = QColor("#64c8ff");
    double  m_glowSize1       = 1.3;
    QColor  m_glowColor2      = QColor("#64c8ff");
    double  m_glowSize2       = 0.0;

    bool    m_showClock       = false;
    QColor  m_hourHandColor   = Qt::white;
    QColor  m_minuteHandColor = QColor("#88ccff");
    QColor  m_secondHandColor = QColor("#ffaa88");

    QPointF m_mousePos;

    // ── Stato simulazione ─────────────────────────────────────────────────────
    std::vector<ParticleData>   m_particles;
    std::vector<ParticleVertex> m_verticesRender;

    float         m_rotation = 0.0f;
    QElapsedTimer m_timeTracker;
    qint64        m_lastTime  = 0;
    float         m_lastDt    = 0.016f;

    // ── Stato render ──────────────────────────────────────────────────────────
    // Separazione netta: m_pendingActiveCount scritto dal worker thread,
    // m_renderActiveCount letto solo dal render thread (copiato in finished()).
    std::atomic<int>  m_pendingActiveCount { 0 };
    int               m_renderActiveCount  = 0;

    QColor m_lastGlowColor1;
    QColor m_lastGlowColor2;
    qreal  m_lastDpr = 0.0;

    // ── Async ─────────────────────────────────────────────────────────────────
    QFutureWatcher<void> m_watcher;
    // Atomic: garantisce visibilità cross-thread senza mutex, overhead ~zero.
    std::atomic<bool> m_simulationPending { false };

    // ── Puntatori ai nodi SGG (evita childAtIndex() fragili) ─────────────────
    QSGGeometryNode     *m_particleNode = nullptr;
    QSGTransformNode    *m_systemNode   = nullptr;
    QSGTransformNode    *m_taoRotNode   = nullptr;
    QSGNode             *m_clockGroup   = nullptr;
    QSGSimpleTextureNode *m_glowNode1   = nullptr;
    QSGSimpleTextureNode *m_glowNode2   = nullptr;
    QSGSimpleTextureNode *m_taoNode     = nullptr;
};

#endif // TAONEW_H