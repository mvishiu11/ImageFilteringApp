#include "drawingengine.h"
#include <QColor>
#include <QtMath>
#include <algorithm>

// Helper: Set pixel if within bounds.
static inline void setPixelSafe(QImage &img, int x, int y,
                                const QColor &color) {
  if (x >= 0 && x < img.width() && y >= 0 && y < img.height()) {
    img.setPixel(x, y, color.rgb());
  }
}

void drawLineDDA(QImage &img, int x0, int y0, int x1, int y1,
                 const QColor &color) {
  int dx = x1 - x0;
  int dy = y1 - y0;
  int steps = std::max(std::abs(dx), std::abs(dy));
  if (steps == 0) {
    setPixelSafe(img, x0, y0, color);
    return;
  }
  double xInc = dx / double(steps);
  double yInc = dy / double(steps);
  double x = x0, y = y0;
  for (int i = 0; i <= steps; ++i) {
    setPixelSafe(img, int(round(x)), int(round(y)), color);
    x += xInc;
    y += yInc;
  }
}

void drawLineWu(QImage &img, int x0, int y0, int x1, int y1,
                const QColor &color) {
  // Implementation based on Xiaolin Wu algorithm.
  // For brevity, here is a simplified version.
  bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
  if (steep) {
    std::swap(x0, y0);
    std::swap(x1, y1);
  }
  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }
  int dx = x1 - x0;
  int dy = y1 - y0;
  double gradient = dx == 0 ? 1 : double(dy) / dx;
  // For endpoints, we simply use rounding (more detailed implementation would
  // blend edge pixels).
  for (int x = x0; x <= x1; ++x) {
    double y = y0 + gradient * (x - x0);
    int iy = int(round(y));
    if (steep)
      setPixelSafe(img, iy, x, color);
    else
      setPixelSafe(img, x, iy, color);
  }
}

void drawCircleMidpoint(QImage &img, int xc, int yc, int radius,
                        const QColor &color) {
  int x = 0, y = radius, d = 1 - radius;
  while (x <= y) {
    // Draw the eight symmetrical points.
    setPixelSafe(img, xc + x, yc + y, color);
    setPixelSafe(img, xc - x, yc + y, color);
    setPixelSafe(img, xc + x, yc - y, color);
    setPixelSafe(img, xc - x, yc - y, color);
    setPixelSafe(img, xc + y, yc + x, color);
    setPixelSafe(img, xc - y, yc + x, color);
    setPixelSafe(img, xc + y, yc - x, color);
    setPixelSafe(img, xc - y, yc - x, color);
    if (d < 0)
      d += (2 * x + 3);
    else {
      d += (2 * (x - y) + 5);
      y--;
    }
    x++;
  }
}

void drawFreehandPen(QImage &img, const QVector<QPoint> &points,
                     const QColor &color) {
  if (points.isEmpty())
    return;
  QPoint prev = points.first();
  for (int i = 1; i < points.size(); ++i) {
    QPoint curr = points[i];
    drawLineDDA(img, prev.x(), prev.y(), curr.x(), curr.y(), color);
    prev = curr;
  }
}
