#include "cylinderwidget.h"
#include "cylindermesh.h"
#include "drawingengine.h"
#include <QPainter>
#include <QWheelEvent>
#include <QFileDialog>
#include <cmath>
#include <algorithm>
#include <QtMath>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

/* =============== ctor & UI ========================================= */
CylinderWidget::CylinderWidget(QWidget *parent)
    : QWidget(parent)
{
    /* ----- mesh ---------------------------------------------------- */
    buildCylinder(40);

    /* ----- texture placeholder ------------------------------------ */
    texture = QImage(256,256,QImage::Format_RGB32);
    texture.fill(Qt::gray);

    /* ----- control panel ------------------------------------------ */
    sx=new QSlider(Qt::Horizontal); sx->setRange(0,360); sx->setValue(int(rotX));
    sy=new QSlider(Qt::Horizontal); sy->setRange(0,360); sy->setValue(int(rotY));
    sd=new QSlider(Qt::Horizontal); sd->setRange(3,15);  sd->setValue(int(dist));
    autoBox=new QCheckBox(tr("auto")); autoBox->setChecked(false);

    connect(sx,&QSlider::valueChanged,this,&CylinderWidget::updateParams);
    connect(sy,&QSlider::valueChanged,this,&CylinderWidget::updateParams);
    connect(sd,&QSlider::valueChanged,this,&CylinderWidget::updateParams);
    connect(autoBox,&QCheckBox::toggled ,this,&CylinderWidget::toggleAuto);

    tick.setInterval(30); connect(&tick,&QTimer::timeout,this,&CylinderWidget::stepAuto);

    auto *lay=new QVBoxLayout; lay->setContentsMargins(6,6,6,6);
    lay->addWidget(new QLabel("rot X")); lay->addWidget(sx);
    lay->addWidget(new QLabel("rot Y")); lay->addWidget(sy);
    lay->addWidget(new QLabel("dist"));  lay->addWidget(sd);
    lay->addWidget(autoBox); lay->addStretch(1);

    panel=new QWidget; panel->setLayout(lay); panel->setFixedWidth(120);

    auto *main=new QHBoxLayout(this);
    main->setContentsMargins(0,0,0,0);
    main->addWidget(panel);
    main->addStretch(1);
}

/* =============== mesh ============================================== */
void CylinderWidget::buildCylinder(int slices)
{
    makeCylinderMesh(vbo,ibo,slices);
}

/* =============== parameter slots =================================== */
void CylinderWidget::updateParams()
{
    rotX = sx->value();
    rotY = sy->value();
    dist = sd->value();
    update();
}
void CylinderWidget::toggleAuto(bool on){ on ? tick.start():tick.stop(); }
void CylinderWidget::stepAuto(){ sy->setValue((sy->value()+1)%360); }

/* =============== matrices ========================================== */
void CylinderWidget::fillMatrices()
{
    M.setToIdentity();
    M.rotate(rotX,{1,0,0});
    M.rotate(rotY,{0,1,0});

    V.setToIdentity();
    V.translate(0,0,-dist);

    P.setToIdentity();
    float n=0.1f, f=100.f, fov=60.f, ar=float(CANVAS_W)/CANVAS_H;
    float t=n*std::tan(fov*M_PI/360.0);
    float r=t*ar;
    P.frustum(-r,r,-t,t,n,f);

    MVP = P*V*M;
}

/* =============== texture loader ==================================== */
bool CylinderWidget::loadTexture(const QString& fn)
{
    QImage img(fn);
    if(img.isNull()) return false;
    texture = img.convertToFormat(QImage::Format_RGB32);
    update();
    return true;
}

/* =============== projection utils ================================== */
Frag CylinderWidget::projectVertex(const Vtx& v) const
{
    QVector4D hp = MVP * QVector4D(v.pos,1.0f);
    float invW = 1.f / hp.w();
    Frag f;
    f.sx   = ( hp.x()*invW * 0.5f + 0.5f) * (CANVAS_W-1);
    f.sy   = (-hp.y()*invW * 0.5f + 0.5f) * (CANVAS_H-1);
    f.sz_w = invW;
    f.u_w  = v.uv.x()*invW;
    f.v_w  = v.uv.y()*invW;
    return f;
}
bool CylinderWidget::backFace(const Frag& a,
                          const Frag& b,
                          const Frag& c) const
{
    float area = (b.sx - a.sx) * (c.sy - a.sy)
    - (b.sy - a.sy) * (c.sx - a.sx);
    return area <= 0;
}

/* =============== rasteriser ======================================== */
inline uint CylinderWidget::sampleTex(float u,float v) const
{
    int x = qBound(0, int(u*texture.width ()), texture.width ()-1);
    int y = qBound(0, int(v*texture.height()), texture.height()-1);
    return texture.pixel(x,y);
}

void CylinderWidget::rasterTriangle(QImage& buf,const Tri& t,
                                    const Frag& A,const Frag& B,const Frag& C)
{
    /* bounding box */
    int minX = std::floor(std::min({A.sx,B.sx,C.sx}));
    int maxX = std::ceil (std::max({A.sx,B.sx,C.sx}));
    int minY = std::floor(std::min({A.sy,B.sy,C.sy}));
    int maxY = std::ceil (std::max({A.sy,B.sy,C.sy}));
    minX = qMax(minX,0); minY = qMax(minY,0);
    maxX = qMin(maxX,buf.width()-1);
    maxY = qMin(maxY,buf.height()-1);

    /* edge functions */
    auto edge = [](const Frag& p,const Frag& q,float x,float y){
        return (q.sx-p.sx)*(y-p.sy) - (q.sy-p.sy)*(x-p.sx);
    };
    float area = edge(A,B,C.sx,C.sy);
    if(area==0) return;
    for(int y=minY;y<=maxY;++y)
        for(int x=minX;x<=maxX;++x){
            float w0 = edge(B,C,x,y);
            float w1 = edge(C,A,x,y);
            float w2 = edge(A,B,x,y);
            if ((w0 * area) < 0 || (w1 * area) < 0 || (w2 * area) < 0)
                continue;
            /* barycentric + perspective corr */
            w0/=area; w1/=area; w2/=area;
            float invW = w0*A.sz_w + w1*B.sz_w + w2*C.sz_w;
            float u = (w0*A.u_w + w1*B.u_w + w2*C.u_w)/invW;
            float v = (w0*A.v_w + w1*B.v_w + w2*C.v_w)/invW;
            buf.setPixel(x,y, sampleTex(u,v));
        }
}

void CylinderWidget::drawScene(QImage& buf)
{
    fillMatrices();
    /* project all vertices once */
    QVector<Frag> frag(vbo.size());
    for(int i=0;i<vbo.size();++i) frag[i] = projectVertex(vbo[i]);
    /* draw triangles */
    for(const Tri& t : ibo){
        const Frag &A=frag[t.a], &B=frag[t.b], &C=frag[t.c];
        if(backFace(A,B,C)) continue;          /* cull */
        rasterTriangle(buf,t,A,B,C);
    }
}

/* =============== painting ========================================== */
void CylinderWidget::paintEvent(QPaintEvent*)
{
    QImage buf(CANVAS_W,CANVAS_H,QImage::Format_RGB32);
    buf.fill(Qt::white);
    drawScene(buf);

    QPainter p(this);
    /* fit canvas into available space, keep aspect */
    QSize target = buf.size()*zoom;
    QPoint pos( panel->width() + (width()-panel->width()-target.width())/2,
               (height()-target.height())/2 );
    p.drawImage(QRect(pos,target), buf);
}

/* =============== wheel zoom  ======================================= */
void CylinderWidget::wheelEvent(QWheelEvent *e)
{
    int d = e->angleDelta().y();
    zoom = qBound(1, zoom + (d>0?1:-1), 8);
    update();
}
void CylinderWidget::resizeEvent(QResizeEvent *ev){ QWidget::resizeEvent(ev); }

/* =================================================================== */
