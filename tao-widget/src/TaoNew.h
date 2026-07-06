#ifndef TAONEW_H
#define TAONEW_H

#include <QQuickItem>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QSGNode>
#include <QSGGeometryNode>
#include <QSGSimpleTextureNode>
#include <QSGTransformNode>
#include <QSGGeometry>
#include <QSGMaterial>
#include <QSGMaterialShader>
#include <QSGTexture>
#include <vector>

// ── Strutture dati particelle ─────────────────────────────────────────────────

struct ParticleData {
    float   x, y;
    float   vx, vy;
    float   life;
    float   decay;
    float   size;
    quint8  secondary;   // classe colore decisa allo spawn (evita i%7 per frame)
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
    // Simula un passo e scrive i vertici direttamente nel buffer geometria.
    void   simulate(ParticleVertex *vData, int count, float dt, float dpr);
    // Vero se il prossimo frame va comunque ridisegnato (animazione attiva).
    bool   needsAnimation() const;
    QImage generateGlowTexture(int physSize, const QColor &color);
    QImage generateTaoTexture (int physSize);

    // ── Proprietà configurabili ───────────────────────────────────────────────
    int     m_particleCount   = 120;
    QColor  m_particleColor1  = QColor("#a1f2fc");
    QColor  m_particleColor2  = QColor("#ff7200");
    double  m_particleSize     = 4.0;
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

    // ── Stato simulazione (toccato solo nella fase di sync: GUI bloccata) ────
    std::vector<ParticleData> m_particles;
    QRandomGenerator          m_rng { QRandomGenerator::global()->generate() };

    float         m_rotation = 0.0f;
    QElapsedTimer m_timeTracker;
    qint64        m_lastTime  = 0;

    // ── Stato render ──────────────────────────────────────────────────────────
    int    m_allocatedCount  = -1;   // vertici attualmente allocati nella geometria
    QColor m_lastGlowColor1;
    QColor m_lastGlowColor2;
    int    m_taoTexPx   = 0;         // lato fisico (px) della texture Tao corrente
    int    m_glowTexPx1 = 0;
    int    m_glowTexPx2 = 0;
    float  m_lastHandAngle[3] = { -1e9f, -1e9f, -1e9f };
    float  m_lastHandLen [3]  = { -1.0f, -1.0f, -1.0f };

    // ── Puntatori ai nodi SG (evita childAtIndex() fragili) ──────────────────
    QSGGeometryNode      *m_particleNode = nullptr;
    QSGTransformNode     *m_systemNode   = nullptr;
    QSGTransformNode     *m_taoRotNode   = nullptr;
    QSGNode              *m_clockGroup   = nullptr;
    QSGSimpleTextureNode *m_glowNode1    = nullptr;
    QSGSimpleTextureNode *m_glowNode2    = nullptr;
    QSGSimpleTextureNode *m_taoNode      = nullptr;
};

#endif // TAONEW_H
