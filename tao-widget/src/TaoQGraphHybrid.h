#ifndef TAOQGRAPHYBRID_H
#define TAOQGRAPHYBRID_H

#include <QQuickItem>
#include <QFuture>
#include <QFutureWatcher>
#include <QElapsedTimer>
#include <vector>
#include <QSGNode>
#include <QSGGeometry>
#include <QSGMaterial>
#include <QSGMaterialShader>
#include <QSGTexture>

// ── ParticleData ──────────────────────────────────────────────────────────────
// Struttura ottimizzata per la simulazione (back/front buffer).
struct ParticleData {
    float x, y;
    float vx, vy;
    float life;
    float decay;
    float size;
    quint32 packedColor; // Colore pre-calcolato con alpha premoltiplicato
};

// ── ParticleVertex ────────────────────────────────────────────────────────────
// Singolo vertice nel vertex buffer GPU.
// Stride = 20 byte: 2×float pos + 2×float uv + 4×byte color.
struct ParticleVertex {
    float x, y;
    float u, v;
    quint32 color;
};

// ── ParticleQuad ──────────────────────────────────────────────────────────────
// I 4 vertici di una particella, pre-buildati nel thread simulazione (OPT 5).
// In updatePaintNode vengono copiati nel vertex buffer con un singolo memcpy,
// eliminando il loop di costruzione vertici dal thread GUI.
struct ParticleQuad {
    ParticleVertex v[4];
};

// ── ParticleMaterial ──────────────────────────────────────────────────────────
// Custom material con tre input vertice: position(0), uv(1), color(2).
// QSGTextureMaterial conosce solo 0 e 1, quindi usiamo la nostra implementazione.
class ParticleMaterial : public QSGMaterial
{
public:
    ParticleMaterial();
    QSGMaterialType *type() const override;
    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode) const override;
};

// ── TaoQGraphHybrid ───────────────────────────────────────────────────────────
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

    int     particleCount()  const { return m_particleCount; }
    float   rotationSpeed()  const { return m_rotationSpeed; }
    bool    clockwise()      const { return m_clockwise; }
    bool    showClock()      const { return m_showClock; }
    bool    lowCpuMode()     const { return m_lowCpuMode; }
    QPointF mousePos()       const { return m_mousePos; }
    QColor  hourHandColor()  const { return m_hourHandColor; }
    QColor  minuteHandColor()const { return m_minuteHandColor; }
    QColor  secondHandColor()const { return m_secondHandColor; }
    QColor  glowColor1()     const { return m_glowColor1; }
    double  glowSize1()      const { return m_glowSize1; }
    QColor  glowColor2()     const { return m_glowColor2; }
    double  glowSize2()      const { return m_glowSize2; }
    QColor  particleColor1() const { return m_particleColor1; }
    QColor  particleColor2() const { return m_particleColor2; }

    void setParticleCount(int count);
    void setRotationSpeed(float speed);
    void setClockwise(bool clockwise);
    void setShowClock(bool show);
    void setLowCpuMode(bool lowCpu);
    void setMousePos(const QPointF &pos);
    void setHourHandColor(const QColor &c);
    void setMinuteHandColor(const QColor &c);
    void setSecondHandColor(const QColor &c);
    void setGlowColor1(const QColor &c);
    void setGlowSize1(double s);
    void setGlowColor2(const QColor &c);
    void setGlowSize2(double s);
    void setParticleColor1(const QColor &c);
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
    void setupGeometryIndices(QSGGeometry *geo, int count);

    // OPT DPR: dpr aggiunto come parametro per texture HiDPI-aware.
    // Valore di default = 1.0 per compatibilità con display normali.
    QImage generateGlowTexture(int s, const QColor &color, qreal dpr = 1.0);
    QImage generateTaoTexture(int s, qreal dpr = 1.0);

    // ── Configurazione ────────────────────────────────────────────────────────
    int     m_particleCount   = 80;
    float   m_rotationSpeed   = 5.0f;
    bool    m_clockwise       = true;
    bool    m_showClock       = false;
    bool    m_lowCpuMode      = false;
    QPointF m_mousePos;
    QColor  m_hourHandColor   = Qt::white;
    QColor  m_minuteHandColor = QColor("#88ccff");
    QColor  m_secondHandColor = QColor("#ffaa88");
    QColor  m_glowColor1      = QColor("#64c8ff");
    double  m_glowSize1       = 1.3;
    QColor  m_glowColor2      = QColor("#64c8ff");
    double  m_glowSize2       = 0.0;
    QColor  m_particleColor1  = QColor("#7fcdff");
    QColor  m_particleColor2  = QColor("#ffaa00");

    // ── Stato simulazione ─────────────────────────────────────────────────────
    std::vector<ParticleData>  m_particles;       // Back buffer (thread simulazione)
    std::vector<ParticleData>  m_particlesRender; // Front buffer (thread render)

    // OPT 5: buffer quad pre-buildati nel thread simulazione.
    // Ogni elemento contiene i 4 vertici pronti per il memcpy diretto in GPU.
    std::vector<ParticleQuad>  m_quadsRender;

    float   m_rotation  = 0.0f;
    QElapsedTimer m_timeTracker;
    qint64  m_lastTime  = 0;
    float   m_lastDt    = 0.016f;
    int     m_renderCount = 0;

    // Tracking colori glow per rigenerazione texture solo su cambio effettivo
    QColor  m_lastGlowColor1;
    QColor  m_lastGlowColor2;

    // OPT DPR: device pixel ratio dell'ultimo frame — rileva cambio schermo.
    // Inizializzato a 0 così al primo frame dprChanged è sempre true
    // e le texture vengono generate con il DPR corretto fin dall'inizio.
    qreal   m_lastDpr = 0.0;

    // ── Async ─────────────────────────────────────────────────────────────────
    QFutureWatcher<void> m_watcher;
    bool m_simulationPending = false;

    const int MAX_PARTICLES = 20000;
};

#endif