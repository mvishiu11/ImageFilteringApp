#include "drawingengine.h"
#include <QtMath>
#include <algorithm>

/* ---------- helpers ----------------------------------------------------- */
static inline void setPixelSafe(QImage &im, int x, int y, const QColor &c) {
  if (x >= 0 && x < im.width() && y >= 0 && y < im.height())
    im.setPixel(x, y, c.rgb());
}
static inline void blendPixel(QImage &im, int x, int y, double a,
                              const QColor &c) {
  if (x < 0 || y < 0 || x >= im.width() || y >= im.height())
    return;
  QRgb bg = im.pixel(x, y);
  int r = int(qRed(bg) * (1 - a) + c.red() * a);
  int g = int(qGreen(bg) * (1 - a) + c.green() * a);
  int b = int(qBlue(bg) * (1 - a) + c.blue() * a);
  im.setPixel(x, y, qRgb(r, g, b));
}
/* ---------- DDA line ---------------------------------------------------- */
void drawLineDDA(QImage &im, int x0, int y0, int x1, int y1, const QColor &c) {
  int dx = x1 - x0, dy = y1 - y0;
  int steps = std::max(std::abs(dx), std::abs(dy));
  double x = x0, y = y0, ix = dx / double(steps), iy = dy / double(steps);
  for (int i = 0; i <= steps; ++i, x += ix, y += iy)
    setPixelSafe(im, int(std::round(x)), int(std::round(y)), c);
}
/* ---------- Xiaolin‑Wu line -------------------------------------------- */
static inline int ipart(double x) { return int(std::floor(x)); }
static inline double fpart(double x) { return x - ipart(x); }
static inline double rfpart(double x) { return 1.0 - fpart(x); }
void drawLineWu(QImage &im, int x0, int y0, int x1, int y1, const QColor &c) {
  bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
  if (steep) {
    std::swap(x0, y0);
    std::swap(x1, y1);
  }
  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }

  double dx = x1 - x0, dy = y1 - y0, grad = dx == 0 ? 0 : dy / dx;

  /* first end */
  double xEnd = std::round(x0), yEnd = y0 + grad * (xEnd - x0);
  double xGap = rfpart(x0 + 0.5);
  int xpxl1 = int(xEnd), ypxl1 = ipart(yEnd);
  if (steep) {
    blendPixel(im, ypxl1, xpxl1, rfpart(yEnd) * xGap, c);
    blendPixel(im, ypxl1 + 1, xpxl1, fpart(yEnd) * xGap, c);
  } else {
    blendPixel(im, xpxl1, ypxl1, rfpart(yEnd) * xGap, c);
    blendPixel(im, xpxl1, ypxl1 + 1, fpart(yEnd) * xGap, c);
  }
  double intery = yEnd + grad;

  /* second end */
  xEnd = std::round(x1);
  yEnd = y1 + grad * (xEnd - x1);
  xGap = fpart(x1 + 0.5);
  int xpxl2 = int(xEnd), ypxl2 = ipart(yEnd);
  if (steep) {
    blendPixel(im, ypxl2, xpxl2, rfpart(yEnd) * xGap, c);
    blendPixel(im, ypxl2 + 1, xpxl2, fpart(yEnd) * xGap, c);
  } else {
    blendPixel(im, xpxl2, ypxl2, rfpart(yEnd) * xGap, c);
    blendPixel(im, xpxl2, ypxl2 + 1, fpart(yEnd) * xGap, c);
  }
  /* main loop */
  for (int x = xpxl1 + 1; x < xpxl2; ++x) {
    if (steep) {
      blendPixel(im, ipart(intery), x, rfpart(intery), c);
      blendPixel(im, ipart(intery) + 1, x, fpart(intery), c);
    } else {
      blendPixel(im, x, ipart(intery), rfpart(intery), c);
      blendPixel(im, x, ipart(intery) + 1, fpart(intery), c);
    }
    intery += grad;
  }
}
/* ---------- Mid‑point circle (aliased) --------------------------------- */
void drawCircleMidpoint(QImage &im, int xc, int yc, int r, const QColor &c) {
  int x = 0, y = r, d = 1 - r;
  while (x <= y) {
    setPixelSafe(im, xc + x, yc + y, c);
    setPixelSafe(im, xc - x, yc + y, c);
    setPixelSafe(im, xc + x, yc - y, c);
    setPixelSafe(im, xc - x, yc - y, c);
    setPixelSafe(im, xc + y, yc + x, c);
    setPixelSafe(im, xc - y, yc + x, c);
    setPixelSafe(im, xc + y, yc - x, c);
    setPixelSafe(im, xc - y, yc - x, c);
    if (d < 0)
      d += 2 * x + 3;
    else {
      d += 2 * (x - y) + 5;
      --y;
    }
    ++x;
  }
}
/* ---------- Gupta‑Sproull anti‑aliased circle -------------------------- */
static void plotAA(QImage &im, int xc, int yc, int x, int y, const QColor &c,
                   double a) {
  blendPixel(im, xc + x, yc + y, a, c);
  blendPixel(im, xc - x, yc + y, a, c);
  blendPixel(im, xc + x, yc - y, a, c);
  blendPixel(im, xc - x, yc - y, a, c);
  blendPixel(im, xc + y, yc + x, a, c);
  blendPixel(im, xc - y, yc + x, a, c);
  blendPixel(im, xc + y, yc - x, a, c);
  blendPixel(im, xc - y, yc - x, a, c);
}
void drawCircleAA(QImage &im, int xc, int yc, int r, const QColor &c) {
  int x = 0, y = r;
  double T = 0.70710678; // 1/√2
  double invR = 1.0 / r;
  int d = 1 - r;
  while (x <= y) {
    double dist = std::fabs(std::sqrt(x * x + y * y) - r);
    double alpha = std::clamp(1.0 - dist, 0.0, 1.0);
    plotAA(im, xc, yc, x, y, c, alpha);
    if (d < 0)
      d += 2 * x + 3;
    else {
      d += 2 * (x - y) + 5;
      --y;
    }
    ++x;
    if (y < r * T)
      break; // octant symmetry optimisation
  }
}
/* ---------- free‑hand helper ------------------------------------------- */
void drawFreehandPen(QImage &im, const QVector<QPoint> &pts, const QColor &c) {
  if (pts.size() < 2)
    return;
  for (int i = 1; i < pts.size(); ++i)
    drawLineDDA(im, pts[i - 1].x(), pts[i - 1].y(), pts[i].x(), pts[i].y(), c);
}
