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

    autoBox = new QCheckBox(tr("auto-rotate"));
    wireBox = new QCheckBox(tr("Wireframe (W)"));  wireBox->setChecked(drawWire);
    cullBox = new QCheckBox(tr("Culling (C)"));   cullBox->setChecked(enableCulling);  // New culling checkbox

    connect(sx,  &QSlider::valueChanged, this, &CylinderWidget::updateParams);
    connect(sy,  &QSlider::valueChanged, this, &CylinderWidget::updateParams);
    connect(sd,  &QSlider::valueChanged, this, &CylinderWidget::updateParams);
    connect(autoBox, &QCheckBox::toggled, this, &CylinderWidget::toggleAuto);
    connect(wireBox, &QCheckBox::toggled, this, [this](bool on){ drawWire=on; update(); });
    connect(cullBox, &QCheckBox::toggled, this, [this](bool on){ enableCulling=on; update(); });  // New connection

    tick.setInterval(30); connect(&tick, &QTimer::timeout, this, &CylinderWidget::stepAuto);

    auto *lay = new QVBoxLayout;  lay->setContentsMargins(6,6,6,6);
    lay->addWidget(new QLabel("rot X")); lay->addWidget(sx);
    lay->addWidget(new QLabel("rot Y")); lay->addWidget(sy);
    lay->addWidget(new QLabel("dist"));  lay->addWidget(sd);
    lay->addWidget(autoBox);
    lay->addWidget(wireBox);
    lay->addWidget(cullBox);  // Add culling checkbox to UI
    lay->addStretch(1);

    panel = new QWidget; panel->setLayout(lay); panel->setFixedWidth(140);
    auto *main = new QHBoxLayout(this); main->addWidget(panel); main->addStretch(1);
}

/* ========= build procedural mesh ================================= */
void CylinderWidget::buildCylinder(int slices) { makeCylinderMesh(vbo,ibo,slices); }

/* ========= slot helpers ========================================== */
void CylinderWidget::updateParams()
{ rotX=sx->value(); rotY=sy->value(); dist=sd->value(); update(); }

void CylinderWidget::toggleAuto(bool on){ on?tick.start():tick.stop(); }
void CylinderWidget::stepAuto(){ sy->setValue((sy->value()+1)%360); }

/* ========= matrices ============================================== */
void CylinderWidget::fillMatrices()
{
    M.setToIdentity();  M.rotate(rotX,{1,0,0}); M.rotate(rotY,{0,1,0});
    V.setToIdentity();  V.translate(0,0,-dist);
    float n=0.1f,f=100.f,fov=60.f, ar=float(CANVAS_W)/CANVAS_H;
    float t=n*std::tan(fov*M_PI/360.0), r=t*ar;
    P.setToIdentity();  P.frustum(-r,r,-t,t,n,f);
    MVP = P*V*M;
}

/* ========= projection helpers ==================================== */
Frag CylinderWidget::projectVertex(const Vtx& v) const
{
    QVector4D hp = MVP * QVector4D(v.pos,1);
    float invW = 1.f/hp.w();
    return { ( hp.x()*invW*0.5f +0.5f)*(CANVAS_W-1),
            (-hp.y()*invW*0.5f +0.5f)*(CANVAS_H-1),
            invW,
            v.uv.x()*invW,
            v.uv.y()*invW };
}

/* ---------- corrected camera-space back-face test ---------------- */
bool CylinderWidget::backFaceCam(const CamV& a,
                                 const CamV& b,
                                 const CamV& c) const
{
    // Calculate the face normal in camera space using cross product
    // The order matters: this determines which direction is "outward"
    QVector3D ab = b.pos - a.pos;
    QVector3D ac = c.pos - a.pos;
    QVector3D normal = QVector3D::crossProduct(ab, ac);

    // Use the centroid of the triangle as a representative point
    QVector3D triangleCenter = (a.pos + b.pos + c.pos) / 3.0f;

    // In camera space, camera is at origin looking down negative Z
    // Vector from triangle to camera is negative of triangle position
    QVector3D toCamera = -triangleCenter;

    // Key insight: if normal points toward camera, triangle faces camera (visible)
    // If normal points away from camera, triangle faces away (should be culled)
    float dotProduct = QVector3D::dotProduct(normal, toCamera);

    // CORRECTED: Return true if facing AWAY from camera (negative dot product)
    return dotProduct < 0.0f;  // Cull if facing away from camera
}

/* ---------- improved texture sampling with proper wrap ----------- */
uint CylinderWidget::sampleTex(float u, float v) const
{
    // Improved wrapping - handle negative values properly and ensure smooth wrap-around
    u = u - std::floor(u);  // This handles both positive and negative values correctly
    v = v - std::floor(v);

    // Ensure we're in valid range [0,1)
    if (u < 0.0f) u += 1.0f;
    if (v < 0.0f) v += 1.0f;
    if (u >= 1.0f) u -= 1.0f;
    if (v >= 1.0f) v -= 1.0f;

    // Convert to texture coordinates
    int x = int(u * (texture.width() - 1));   // Use width-1 to prevent overflow
    int y = int(v * (texture.height() - 1));  // Use height-1 to prevent overflow

    // Clamp to valid bounds as safety measure
    x = qBound(0, x, texture.width() - 1);
    y = qBound(0, y, texture.height() - 1);

    return texture.pixel(x, y);
}

/* ========= triangle fill / wire ================================== */
void CylinderWidget::rasterTriangle(QImage& buf,const Frag& A,const Frag& B,const Frag& C)
{
    if (drawWire) {                       /* just three antialiased edges */
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

            w0/=area; w1/=area; w2/=area;

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

    /* 1. project + keep eye-space copies */
    QVector<Frag> frag(vbo.size());
    QVector<CamV> cam (vbo.size());
    QMatrix4x4 MV = V*M;                              // eye-space transform

    for(int i=0;i<vbo.size();++i){
        frag[i] = projectVertex(vbo[i]);
        cam [i].pos = (MV * QVector4D(vbo[i].pos,1)).toVector3D();
    }

    /* 2. triangles with proper backface culling */
    for(const Tri& t : ibo){
        // Apply backface culling if enabled
        if(enableCulling && backFaceCam(cam[t.a],cam[t.b],cam[t.c])) {
            continue;  // Skip back-facing triangles
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
