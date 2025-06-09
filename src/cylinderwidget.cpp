#include "cylinderwidget.h"
#include "cylindermesh.h"
#include "drawingengine.h"
#include <QPainter>
#include <QWheelEvent>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QtMath>

/* ========= constructor & UI ====================================== */
CylinderWidget::CylinderWidget(QWidget *parent) : QWidget(parent)
{
    buildCylinder(CYL_SLICES);
    texture = QImage(256,256,QImage::Format_RGB32);   texture.fill(Qt::gray);

    sx = new QSlider(Qt::Horizontal);  sx->setRange(0,360); sx->setValue(int(rotX));
    sy = new QSlider(Qt::Horizontal);  sy->setRange(0,360); sy->setValue(int(rotY));
    sd = new QSlider(Qt::Horizontal);  sd->setRange(3,15);  sd->setValue(int(dist));

    autoBox = new QCheckBox(tr("Auto-rotate (A)"));
    wireBox = new QCheckBox(tr("Wireframe (W)"));  wireBox->setChecked(drawWire);
    cullBox = new QCheckBox(tr("Culling (C)"));   cullBox->setChecked(enableCulling);

    connect(sx,  &QSlider::valueChanged, this, &CylinderWidget::updateParams);
    connect(sy,  &QSlider::valueChanged, this, &CylinderWidget::updateParams);
    connect(sd,  &QSlider::valueChanged, this, &CylinderWidget::updateParams);
    connect(autoBox, &QCheckBox::toggled, this, &CylinderWidget::toggleAuto);
    connect(wireBox, &QCheckBox::toggled, this, [this](bool on){ drawWire=on; update(); });
    connect(cullBox, &QCheckBox::toggled, this, [this](bool on){ enableCulling=on; update(); });

    tick.setInterval(30); connect(&tick, &QTimer::timeout, this, &CylinderWidget::stepAuto);

    auto *lay = new QVBoxLayout;  lay->setContentsMargins(6,6,6,6);
    lay->addWidget(new QLabel("rot X")); lay->addWidget(sx);
    lay->addWidget(new QLabel("rot Y")); lay->addWidget(sy);
    lay->addWidget(new QLabel("dist"));  lay->addWidget(sd);
    lay->addWidget(autoBox);
    lay->addWidget(wireBox);
    lay->addWidget(cullBox);
    lay->addStretch(1);

    panel = new QWidget; panel->setLayout(lay); panel->setFixedWidth(140);
    auto *main = new QHBoxLayout(this); main->addWidget(panel); main->addStretch(1);
}

/* ========= procedural mesh building ================================= */
void CylinderWidget::buildCylinder(int slices) { makeCylinderMesh(vbo,ibo,slices); }

/* ========= slot helpers ========================================== */
void CylinderWidget::updateParams()
{ rotX=sx->value(); rotY=sy->value(); dist=sd->value(); update(); }

void CylinderWidget::toggleAuto(bool on){ on?tick.start():tick.stop(); }
void CylinderWidget::stepAuto(){ sy->setValue((sy->value()+1)%360); }

/* ========= matrices ============================================== */

QMatrix4x4 createIdentity()
{
    QMatrix4x4 mat;
    for(int row = 0; row < 4; ++row) {
        for(int col = 0; col < 4; ++col) {
            mat(row, col) = 0.0f;
        }
    }
    mat(0, 0) = 1.0f;
    mat(1, 1) = 1.0f;
    mat(2, 2) = 1.0f;
    mat(3, 3) = 1.0f;
    return mat;
}

QMatrix4x4 createRotationX(float degrees)
{
    float radians = degrees * M_PI / 180.0f;
    float c = std::cos(radians);
    float s = std::sin(radians);

    QMatrix4x4 mat;
    mat.setToIdentity();

    mat(1, 1) = c;   mat(1, 2) = -s;
    mat(2, 1) = s;  mat(2, 2) = c;

    return mat;
}

QMatrix4x4 createRotationY(float degrees)
{
    float radians = degrees * M_PI / 180.0f;
    float c = std::cos(radians);
    float s = std::sin(radians);

    QMatrix4x4 mat;
    mat.setToIdentity();

    mat(0, 0) = c;   mat(0, 2) = s;
    mat(2, 0) = -s;  mat(2, 2) = c;

    return mat;
}

QMatrix4x4 createTranslation(float x, float y, float z)
{
    QMatrix4x4 mat;
    mat.setToIdentity();

    mat(0, 3) = x;
    mat(1, 3) = y;
    mat(2, 3) = z;

    return mat;
}

QMatrix4x4 createFrustum(float left, float right, float bottom, float top, float near, float far)
{
    QMatrix4x4 mat;

    float A = (right + left) / (right - left);
    float B = (top + bottom) / (top - bottom);
    float C = -(far + near) / (far - near);
    float D = -(2.0f * far * near) / (far - near);

    mat(0, 0) = (2.0f * near) / (right - left);
    mat(0, 1) = 0.0f;
    mat(0, 2) = A;
    mat(0, 3) = 0.0f;

    mat(1, 0) = 0.0f;
    mat(1, 1) = (2.0f * near) / (top - bottom);
    mat(1, 2) = B;
    mat(1, 3) = 0.0f;

    mat(2, 0) = 0.0f;
    mat(2, 1) = 0.0f;
    mat(2, 2) = C;
    mat(2, 3) = D;

    mat(3, 0) = 0.0f;
    mat(3, 1) = 0.0f;
    mat(3, 2) = -1.0f;
    mat(3, 3) = 0.0f;

    return mat;
}

void CylinderWidget::fillMatrices()
{
    QMatrix4x4 rotX_mat = createRotationX(rotX);
    QMatrix4x4 rotY_mat = createRotationY(rotY);
    M = rotX_mat * rotY_mat;

    V = createTranslation(0.0f, 0.0f, -dist);

    float n = 0.1f, f = 100.0f, fov = 60.0f;
    float ar = float(CANVAS_W) / CANVAS_H;
    float t = n * std::tan(fov * M_PI / 360.0f);
    float r = t * ar;

    P = createFrustum(-r, r, -t, t, n, f);

    MVP = P * V * M;
}

/* ---------- back-face test ---------------- */
bool CylinderWidget::backFaceCam(const CamV& a,
                                 const CamV& b,
                                 const CamV& c) const
{
    QVector3D ab = b.pos - a.pos;
    QVector3D ac = c.pos - a.pos;
    QVector3D normal = QVector3D::crossProduct(ab, ac);

    QVector3D triangleCenter = (a.pos + b.pos + c.pos) / 3.0f;

    QVector3D toCamera = -triangleCenter;

    float dotProduct = QVector3D::dotProduct(normal, toCamera);

    return dotProduct > 0.0f;
}

/* ---------- texture sampling ----------- */
uint CylinderWidget::sampleTex(float u, float v) const
{
    u = u - std::floor(u);
    v = v - std::floor(v);

    int x = int(u * (texture.width() - 1));
    int y = int(v * (texture.height() - 1));

    x = qBound(0, x, texture.width() - 1);
    y = qBound(0, y, texture.height() - 1);

    return texture.pixel(x, y);
}

/* ========= projection helpers ==================================== */
Frag CylinderWidget::projectVertex(const Vtx& v) const
{
    QVector4D hp = MVP * QVector4D(v.pos,1);
    float invW = 1.f/hp.w();
    return { ( hp.x() * invW * 0.5f + 0.5f) * (CANVAS_W-1),
            (-hp.y() * invW * 0.5f + 0.5f) * (CANVAS_H-1),
            invW,
            v.uv.x() * invW,     // TODO: Why do we multiply here?
            v.uv.y() * invW };   // TODO: Just to divide later at pixel level?
}

/* ========= triangle fill / wire ================================== */
void CylinderWidget::rasterTriangle(QImage& buf,const Frag& A,const Frag& B,const Frag& C)
{
    if (drawWire) {
        drawLineWu(buf,int(A.sx),int(A.sy),int(B.sx),int(B.sy),0xFF000000);
        drawLineWu(buf,int(B.sx),int(B.sy),int(C.sx),int(C.sy),0xFF000000);
        drawLineWu(buf,int(C.sx),int(C.sy),int(A.sx),int(A.sy),0xFF000000);
        return;
    }

    auto edge=[&](const Frag&p,const Frag&q,float x,float y){
        return (q.sx-p.sx)*(y-p.sy) - (q.sy-p.sy)*(x-p.sx);
    };

    float area=edge(A,B,C.sx,C.sy);
    if(qAbs(area)<EPS) return;

    int minX=std::floor(std::min({A.sx,B.sx,C.sx}));
    int maxX=std::ceil (std::max({A.sx,B.sx,C.sx}));
    int minY=std::floor(std::min({A.sy,B.sy,C.sy}));
    int maxY=std::ceil (std::max({A.sy,B.sy,C.sy}));

    minX=qMax(minX,0); minY=qMax(minY,0);
    maxX=qMin(maxX,buf.width()-1); maxY=qMin(maxY,buf.height()-1);

    for(int y=minY;y<=maxY;++y)
        for(int x=minX;x<=maxX;++x){
            float w0=edge(B,C,x,y), w1=edge(C,A,x,y), w2=edge(A,B,x,y);
            if((w0*area)<0 || (w1*area)<0 || (w2*area)<0) continue;

            w0 /= area; w1 /= area; w2 /= area;

            // TODO: Why is it divided here?
            float invW = w0*A.sz_w + w1*B.sz_w + w2*C.sz_w;
            float  u = (w0*A.u_w + w1*B.u_w + w2*C.u_w)/invW;
            float  v = (w0*A.v_w + w1*B.v_w + w2*C.v_w)/invW;

            buf.setPixel(x,y, sampleTex(u,v));
        }
}

/* ========= scene ================================================== */
void CylinderWidget::drawScene(QImage& buf)
{
    fillMatrices();

    QVector<Frag> frag(vbo.size());
    QVector<CamV> cam (vbo.size());
    QMatrix4x4 MV = V*M;

    for(int i=0;i<vbo.size();++i){
        frag[i] = projectVertex(vbo[i]);
        cam [i].pos = (MV * QVector4D(vbo[i].pos,1)).toVector3D();
    }

    for(const Tri& t : ibo){
        if(enableCulling && backFaceCam(cam[t.a],cam[t.b],cam[t.c])) {
            continue;
        }
        rasterTriangle(buf, frag[t.a], frag[t.b], frag[t.c]);
    }
}

/* ========= paint ================================================== */
void CylinderWidget::paintEvent(QPaintEvent*)
{
    QImage buf(CANVAS_W,CANVAS_H,QImage::Format_RGB32);
    buf.fill(Qt::white);
    drawScene(buf);

    QPainter p(this);
    QSize target = buf.size()*zoom;
    QPoint pos(panel->width()+(width()-panel->width()-target.width())/2,
               (height()-target.height())/2);
    p.drawImage(QRect(pos,target),buf);
}

/* ========= misc =================================================== */
void CylinderWidget::wheelEvent(QWheelEvent *e)
{ zoom=qBound(1,zoom+(e->angleDelta().y()>0?1:-1),8); update(); }

void CylinderWidget::resizeEvent(QResizeEvent *e)
{ QWidget::resizeEvent(e); update(); }

void CylinderWidget::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key()==Qt::Key_W){
        drawWire = !drawWire;
        wireBox->setChecked(drawWire);
        update();
    }
    else if(ev->key()==Qt::Key_C){
        enableCulling = !enableCulling;
        cullBox->setChecked(enableCulling);
        update();
    } else if(ev->key()==Qt::Key_A){
        toggleAuto(true);
        autoBox->toggle();
        update();
    }
    QWidget::keyPressEvent(ev);
}

bool CylinderWidget::loadTexture(const QString& fn)
{
    QImage img(fn);
    if(img.isNull()) return false;
    texture = img.convertToFormat(QImage::Format_RGB32);
    update();
    return true;
}
