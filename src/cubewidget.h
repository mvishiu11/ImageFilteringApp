#pragma once
#include <QWidget>
#include <QTimer>
#include <QSlider>
#include <QCheckBox>

struct Vec3 { double x{}, y{}, z{}; };

class CubeCanvas;

class CubeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CubeWidget(QWidget *parent = nullptr);

private slots:
    void updateParams();
    void toggleAuto(bool);
    void stepAuto();

private:
    /* UI elements */
    CubeCanvas *canvas{};
    QSlider *sx{}, *sy{}, *sd{};
    QCheckBox *autoBox{};
    QTimer tick;

    /* parameters */
    double angleX{20.0}, angleY{30.0};
    double dist{5.0};
    int    focal{400};
    bool   autoRotate{false};

    friend class CubeCanvas;

    /* cube */
    static const Vec3 verts[8];
    static const int  edges[12][2];
};


class CubeCanvas : public QWidget
{
    Q_OBJECT
public:
    explicit CubeCanvas(CubeWidget *parent);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    CubeWidget *ctrl;
    Vec3    rotate(const Vec3&)   const;
    QPointF project(const Vec3&)  const;
    void    drawEdge(QImage&,const Vec3&,const Vec3&) const;
};
