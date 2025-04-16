#ifndef DRAWINGENGINE_H
#define DRAWINGENGINE_H

#include <QColor>
#include <QImage>
#include <QPoint>
#include <QVector>

void drawLineDDA(QImage &img, int x0, int y0, int x1, int y1,
                 const QColor &color);
void drawLineWu(QImage &img, int x0, int y0, int x1, int y1,
                const QColor &color);

void drawCircleMidpoint(QImage &img, int xc, int yc, int r, const QColor &c);

void drawCircleAA(QImage &img, int xc, int yc, int r, const QColor &c);

void drawFreehandPen(QImage &img, const QVector<QPoint> &points,
                     const QColor &color);
#endif
