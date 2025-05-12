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

void drawCircleWu(QImage &img, int xc, int yc, int r, const QColor &col);

void drawHalfCircleMidpoint(QImage &img, int xc, int yc, int r, double nx,
                            double ny, const QColor &col);

void drawHalfCircleWu(QImage &img, int xc, int yc, int r, double nx, double ny,
                      const QColor &col);

void drawFreehandPen(QImage &img, const QVector<QPoint> &points,
                     const QColor &color);

/* ---------- clipping & filling -------------------------------------- */
bool liangBarskyClip(const QRect &rect, QPointF p0, QPointF p1,
                     QPointF &clippedA, QPointF &clippedB);

void fillPolygonET(QImage &img, const QVector<QPoint> &P, const QColor &colour);

void fillPolygonET(QImage &img, const QVector<QPoint> &P,
                   const QImage *pattern);

#endif
