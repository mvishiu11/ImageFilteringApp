#ifndef DRAWINGENGINE_H
#define DRAWINGENGINE_H

#include <QColor>
#include <QImage>
#include <QPoint>
#include <QVector>

/**
 * @brief Draws a line using the Digital Differential Analyzer (DDA) algorithm.
 * Sets pixels on the image with the given color.
 */
void drawLineDDA(QImage &img, int x0, int y0, int x1, int y1,
                 const QColor &color);

/**
 * @brief Draws an anti-aliased line using Xiaolin Wu's algorithm.
 */
void drawLineWu(QImage &img, int x0, int y0, int x1, int y1,
                const QColor &color);

/**
 * @brief Draws a circle using the Midpoint Circle algorithm.
 */
void drawCircleMidpoint(QImage &img, int xc, int yc, int radius,
                        const QColor &color);

/**
 * @brief Draws freehand strokes by connecting a sequence of points using DDA.
 */
void drawFreehandPen(QImage &img, const QVector<QPoint> &points,
                     const QColor &color);

#endif // DRAWINGENGINE_H
