#include "drawingengine.h"
#include <QtMath>
#include <algorithm>

/* ---------- low‑level helpers ---------------------------------------- */
static inline void setPixelSafe(QImage &im, int x, int y, const QColor &c) {
  if (x >= 0 && x < im.width() && y >= 0 && y < im.height())
    im.setPixel(x, y, c.rgb());
}
static inline void blend(QImage &im, int x, int y, double a, const QColor &c) {
  if (x < 0 || y < 0 || x >= im.width() || y >= im.height())
    return;
  QRgb bg = im.pixel(x, y);
  int r = int(qRed(bg) * (1 - a) + c.red() * a);
  int g = int(qGreen(bg) * (1 - a) + c.green() * a);
  int b = int(qBlue(bg) * (1 - a) + c.blue() * a);
  im.setPixel(x, y, qRgb(r, g, b));
}
/* integer part helpers */
static inline int iPart(double x) { return int(std::floor(x)); }
static inline double fPart(double x) { return x - std::floor(x); }
static inline double rfPart(double x) { return 1.0 - fPart(x); }

/* ---------- DDA line -------------------------------------------------- */
void drawLineDDA(QImage &im, int x0, int y0, int x1, int y1, const QColor &c) {
  int dx = x1 - x0, dy = y1 - y0, steps = std::max(std::abs(dx), std::abs(dy));
  if (!steps) {
    setPixelSafe(im, x0, y0, c);
    return;
  }
  double x = x0, y = y0, ix = dx / double(steps), iy = dy / double(steps);
  for (int i = 0; i <= steps; ++i, x += ix, y += iy)
    setPixelSafe(im, int(std::round(x)), int(std::round(y)), c);
}

/* ---------- Xiaolin‑Wu line ------------------------------------------ */
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

  double dx = x1 - x0, dy = y1 - y0, grad = dx ? dy / dx : 0.0;

  auto plot = [&](int x, int y, double a) {
    if (steep)
      blend(im, y, x, a, c);
    else
      blend(im, x, y, a, c);
  };

  /* first end */
  double xEnd = std::round(x0), yEnd = y0 + grad * (xEnd - x0);
  double xGap = rfPart(x0 + 0.5);
  int ix = int(xEnd), iy = iPart(yEnd);
  plot(ix, iy, rfPart(yEnd) * xGap);
  plot(ix, iy + 1, fPart(yEnd) * xGap);

  double intery = yEnd + grad;

  /* second end */
  xEnd = std::round(x1);
  yEnd = y1 + grad * (xEnd - x1);
  xGap = fPart(x1 + 0.5);
  int ix2 = int(xEnd), iy2 = iPart(yEnd);
  plot(ix2, iy2, rfPart(yEnd) * xGap);
  plot(ix2, iy2 + 1, fPart(yEnd) * xGap);

  /* main loop */
  for (int x = ix + 1; x < ix2; ++x) {
    plot(x, iPart(intery), rfPart(intery));
    plot(x, iPart(intery) + 1, fPart(intery));
    intery += grad;
  }
}

/* ---------- Mid‑point circle (aliased) ------------------------------- */
void drawCircleMidpoint(QImage &im, int xc, int yc, int r, const QColor &col) {
  int x = 0, y = r, d = 1 - r;
  while (x <= y) {
    setPixelSafe(im, xc + x, yc + y, col);
    setPixelSafe(im, xc - x, yc + y, col);
    setPixelSafe(im, xc + x, yc - y, col);
    setPixelSafe(im, xc - x, yc - y, col);
    setPixelSafe(im, xc + y, yc + x, col);
    setPixelSafe(im, xc - y, yc + x, col);
    setPixelSafe(im, xc + y, yc - x, col);
    setPixelSafe(im, xc - y, yc - x, col);
    if (d < 0)
      d += 2 * x + 3;
    else {
      d += 2 * (x - y) + 5;
      --y;
    }
    ++x;
  }
}

/* ---------- Xiaolin‑Wu anti‑aliased circle --------------------------- */
static inline void circlePlot(QImage &im, int xc, int yc, int x, int y,
                              double a, const QColor &c) {
  blend(im, xc + x, yc + y, a, c);
  blend(im, xc - x, yc + y, a, c);
  blend(im, xc + x, yc - y, a, c);
  blend(im, xc - x, yc - y, a, c);
  blend(im, xc + y, yc + x, a, c);
  blend(im, xc - y, yc + x, a, c);
  blend(im, xc + y, yc - x, a, c);
  blend(im, xc - y, yc - x, a, c);
}
void drawCircleWu(QImage &im, int xc, int yc, int r, const QColor &col) {
  if (r <= 0)
    return;
  double x = r, y = 0.0;
  double err = 0.0;

  while (x >= y) {
    /* intensity = fractional part of error between circle equation and pixel */
    double gap = std::abs(err);
    circlePlot(im, xc, yc, int(std::round(x)), int(std::round(y)), rfPart(gap),
               col);
    circlePlot(im, xc, yc, int(std::round(x)), int(std::round(y)) + 1,
               fPart(gap), col);

    y += 1.0;
    err += 2 * y + 1;
    if (err > 0) {
      x -= 1.0;
      err -= 2 * x + 1;
    }
  }
}

/* ---------- free‑hand ------------------------------------------------- */
void drawFreehandPen(QImage &im, const QVector<QPoint> &pts,
                     const QColor &col) {
  for (int i = 1; i < pts.size(); ++i)
    drawLineDDA(im, pts[i - 1].x(), pts[i - 1].y(), pts[i].x(), pts[i].y(),
                col);
}
