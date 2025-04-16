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

static inline void blendPixel(QImage &im, int x, int y, double c,
                              const QColor &col) {
  if (x < 0 || y < 0 || x >= im.width() || y >= im.height())
    return;
  QRgb src = im.pixel(x, y);
  auto sr = qRed(src), sg = qGreen(src), sb = qBlue(src);
  auto dr = col.red(), dg = col.green(), db = col.blue();
  auto rr = int(sr * (1 - c) + dr * c);
  auto gg = int(sg * (1 - c) + dg * c);
  auto bb = int(sb * (1 - c) + db * c);
  im.setPixel(x, y, qRgb(rr, gg, bb));
}

static inline int ipart(double x) { return int(std::floor(x)); }
static inline double fpart(double x) { return x - std::floor(x); }
static inline double rfpart(double x) { return 1.0 - fpart(x); }

/* True Xiaolin‑Wu */
void drawLineWu(QImage &im, int x0, int y0, int x1, int y1, const QColor &col) {
  bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
  if (steep) {
    std::swap(x0, y0);
    std::swap(x1, y1);
  }
  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }

  double dx = x1 - x0, dy = y1 - y0;
  double grad = dx == 0 ? 0 : dy / dx;

  // first end‑point
  double xend = std::round(x0);
  double yend = y0 + grad * (xend - x0);
  double xgap = rfpart(x0 + 0.5);
  int ix = int(xend);
  int iy = ipart(yend);
  if (steep) {
    blendPixel(im, iy, ix, rfpart(yend) * xgap, col);
    blendPixel(im, iy + 1, ix, fpart(yend) * xgap, col);
  } else {
    blendPixel(im, ix, iy, rfpart(yend) * xgap, col);
    blendPixel(im, ix, iy + 1, fpart(yend) * xgap, col);
  }
  double intery = yend + grad;

  // second end‑point
  xend = std::round(x1);
  yend = y1 + grad * (xend - x1);
  xgap = fpart(x1 + 0.5);
  int ix2 = int(xend);
  iy = ipart(yend);
  if (steep) {
    blendPixel(im, iy, ix2, rfpart(yend) * xgap, col);
    blendPixel(im, iy + 1, ix2, fpart(yend) * xgap, col);
  } else {
    blendPixel(im, ix2, iy, rfpart(yend) * xgap, col);
    blendPixel(im, ix2, iy + 1, fpart(yend) * xgap, col);
  }

  // main loop
  for (int x = ix + 1; x < ix2; ++x) {
    if (steep) {
      blendPixel(im, ipart(intery), x, rfpart(intery), col);
      blendPixel(im, ipart(intery) + 1, x, fpart(intery), col);
    } else {
      blendPixel(im, x, ipart(intery), rfpart(intery), col);
      blendPixel(im, x, ipart(intery) + 1, fpart(intery), col);
    }
    intery += grad;
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
