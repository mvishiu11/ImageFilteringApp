#include "cubewidget.h"
#include "drawingengine.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QtMath>
#include <QLabel>

const Vec3 CubeWidget::verts[8] = {
    {-1,-1,-1},{ 1,-1,-1},{ 1, 1,-1},{-1, 1,-1},
    {-1,-1, 1},{ 1,-1, 1},{ 1, 1, 1},{-1, 1, 1}
};
const int CubeWidget::edges[12][2] = {
    {0,1},{1,2},{2,3},{3,0},
    {4,5},{5,6},{6,7},{7,4},
    {0,4},{1,5},{2,6},{3,7}
};

CubeWidget::CubeWidget(QWidget *parent): QWidget(parent)
{
    /* --- UI --- */
    canvas  = new CubeCanvas(this);
    canvas->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    sx = new QSlider(Qt::Horizontal);  sx->setRange(0,360); sx->setValue(int(angleX));
    sy = new QSlider(Qt::Horizontal);  sy->setRange(0,360); sy->setValue(int(angleY));
    sd = new QSlider(Qt::Horizontal);  sd->setRange(2,20);  sd->setValue(int(dist));
    autoBox = new QCheckBox(tr("Auto-rotate")); autoBox->setChecked(false);

    auto *ctrlLayout = new QVBoxLayout;
    ctrlLayout->addWidget(sx);
    ctrlLayout->addWidget(sy);
    ctrlLayout->addWidget(sd);
    ctrlLayout->addWidget(autoBox);
    ctrlLayout->addStretch(1);

    auto *panel = new QWidget; panel->setLayout(ctrlLayout);
    panel->setFixedWidth(120);

    auto *hl = new QHBoxLayout(this);
    hl->setContentsMargins(4,4,4,4);
    hl->addWidget(canvas,1);
    hl->addWidget(panel,0);

    /* --- signals --- */
    connect(sx,&QSlider::valueChanged,this,&CubeWidget::updateParams);
    connect(sy,&QSlider::valueChanged,this,&CubeWidget::updateParams);
    connect(sd,&QSlider::valueChanged,this,&CubeWidget::updateParams);
    connect(autoBox,&QCheckBox::toggled,this,&CubeWidget::toggleAuto);

    tick.setInterval(30);
    connect(&tick,&QTimer::timeout,this,&CubeWidget::stepAuto);
}

/* --- slots --- */
void CubeWidget::updateParams()
{
    angleX = sx->value();
    angleY = sy->value();
    dist   = sd->value();
    canvas->update();
}
void CubeWidget::toggleAuto(bool on)
{
    autoRotate = on;
    on ? tick.start() : tick.stop();
}
void CubeWidget::stepAuto()
{
    sy->setValue((sy->value() + 1) % 360);
}


/* --- canvas impl --- */
CubeCanvas::CubeCanvas(CubeWidget *p): QWidget(p), ctrl(p)
{
    setMinimumSize(300,300);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

static inline double d2r(double d){ return d * M_PI / 180.0; }

Vec3 CubeCanvas::rotate(const Vec3 &v) const
{
    double ax = d2r(ctrl->angleX), ay = d2r(ctrl->angleY);
    double c1 = qCos(ax), s1 = qSin(ax);
    Vec3 t{ v.x, v.y * c1 - v.z * s1, v.y * s1 + v.z * c1 };
    double c2 = qCos(ay), s2 = qSin(ay);
    return { t.x * c2 + t.z * s2, t.y, -t.x * s2 + t.z * c2 };
}

QPointF CubeCanvas::project(const Vec3 &v) const
{
    double z = v.z + ctrl->dist;
    double s = (z == 0 ? 1.0 : ctrl->focal / z);
    return { v.x * s, v.y * s };
}

void CubeCanvas::drawEdge(QImage &im,const Vec3&a,const Vec3&b) const
{
    QPointF pa = project(a), pb = project(b);
    pa += QPointF(im.width() / 2.0, im.height() / 2.0);
    pb += QPointF(im.width() / 2.0, im.height() / 2.0);
    drawLineWu(im, int(pa.x()), int(pa.y()), int(pb.x()), int(pb.y()), Qt::black);
}

/* --- paint --- */
void CubeCanvas::paintEvent(QPaintEvent*)
{
    QSize sz = size() * devicePixelRatio();
    QImage img(sz, QImage::Format_RGB32);
    img.setDevicePixelRatio(devicePixelRatio());
    img.fill(Qt::white);

    for(auto &e : CubeWidget::edges)
        drawEdge(img,
                 rotate(CubeWidget::verts[e[0]]),
                 rotate(CubeWidget::verts[e[1]]));

    QPainter p(this);
    p.drawImage(rect(), img);
}
