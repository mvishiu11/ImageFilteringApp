#pragma once
#include <QWidget>
#include <QImage>
#include <QMatrix4x4>
#include <QVector3D>
#include <QVector2D>
#include <QSlider>
#include <QCheckBox>
#include <QTimer>

/* === tiny helpers =================================================== */
struct Vtx         /* one mesh vertex */
{
    QVector3D pos;     // local position
    QVector2D uv;      // [0..1] texture coord
};
struct Tri         /* triangle index triplet */
{
    int a{},b{},c{};
};
struct Frag        /* derived per-vertex after MVP */
{
    float  sx{}, sy{}, sz_w{};    // screen x,y  |  1/w (for perspect-corr)
    float  u_w{}, v_w{};          // u/w , v/w
};

/* === textured cylinder widget ======================================= */
class CylinderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CylinderWidget(QWidget *parent=nullptr);

    /* public API used by main window */
    QSize   canvasSize() const { return {CANVAS_W, CANVAS_H}; }
    bool    loadTexture(const QString& fn);

protected:
    void paintEvent        (QPaintEvent*) override;
    void wheelEvent        (QWheelEvent*) override;      /* bonus zoom */
    void resizeEvent       (QResizeEvent*) override;

private slots:
    void updateParams();          /* sliders changed          */
    void toggleAuto(bool on);     /* auto-rotate checkbox     */
    void stepAuto();              /* timer tick               */

private:
    /* -- 1. mesh ----------------------------------------------------- */
    void buildCylinder(int slices);
    QVector<Vtx>  vbo;            /* vertices  */
    QVector<Tri>  ibo;            /* triangles */

    /* -- 2. transformation pipeline --------------------------------- */
    void fillMatrices();
    QMatrix4x4   M, V, P, MVP;    /* model, view, proj         */
    float        rotX=25.f, rotY=30.f, dist=6.f;

    /* -- 3. software rasteriser ------------------------------------- */
    void drawScene(QImage& buf);
    void rasterTriangle(QImage& buf, const Tri& t,
                        const Frag& f0, const Frag& f1, const Frag& f2);
    inline uint  sampleTex(float u, float v) const;

    /* -- 4. helpers -------------------------------------------------- */
    Frag projectVertex(const Vtx& v) const;
    bool backFace(const Frag& f0,const Frag& f1,const Frag& f2) const;

    /* -- 5. UI ------------------------------------------------------- */
    QWidget   *panel=nullptr;
    QSlider   *sx,*sy,*sd;
    QCheckBox *autoBox;
    QTimer     tick;
    int        zoom = 1;                 /* mouse-wheel zoom     */

    /* -- 6. resources ------------------------------------------------ */
    QImage    texture;
    static constexpr int CANVAS_W = 640;
    static constexpr int CANVAS_H = 480;
    static constexpr float EPS = -1e-6f;
};
