#pragma once
#include <QWidget>
#include <QImage>
#include <QMatrix4x4>
#include <QVector3D>
#include <QVector2D>
#include <QSlider>
#include <QCheckBox>
#include <QTimer>

/* basic mesh helpers */
struct Vtx  { QVector3D pos; QVector2D uv; };
struct Tri  { int a{}, b{}, c{}; };

struct Frag {
    float sx{}, sy{}, sz_w{};
    float u_w{}, v_w{};
};

/* eye-space copy used only for culling */
struct CamV { QVector3D pos; };

class CylinderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CylinderWidget(QWidget *parent = nullptr);
    bool  loadTexture(const QString& fn);

protected:
    void paintEvent (QPaintEvent*) override;
    void wheelEvent (QWheelEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void keyPressEvent(QKeyEvent *) override;

private slots:
    void updateParams();
    void toggleAuto(bool);
    void stepAuto();

private:
    /* mesh ---------------------------------------------------------- */
    void buildCylinder(int slices);
    QVector<Vtx>  vbo;
    QVector<Tri>  ibo;

    /* transforms ---------------------------------------------------- */
    void        fillMatrices();
    QMatrix4x4  M, V, P, MVP;
    float       rotX{25}, rotY{30}, dist{6};

    /* rasteriser ---------------------------------------------------- */
    void drawScene(QImage &buf);
    void rasterTriangle(QImage &, const Frag&, const Frag&, const Frag&);
    uint sampleTex(float u, float v) const;
    Frag  projectVertex(const Vtx&) const;
    bool  backFaceCam (const CamV&, const CamV&, const CamV&) const;

    /* UI widgets ---------------------------------------------------- */
    QWidget   *panel{};
    QSlider   *sx{}, *sy{}, *sd{};
    QCheckBox *autoBox{}, *wireBox{}, *cullBox{};
    QTimer     tick;
    int        zoom{1};

    /* resources ----------------------------------------------------- */
    QImage texture;
    static constexpr int   CANVAS_W = 640;
    static constexpr int   CANVAS_H = 480;
    static constexpr float EPS       = 1e-4f;

    /* run-time options --------------------------------------------- */
    bool drawWire = false;
    bool enableCulling = true;
};
