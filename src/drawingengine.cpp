#include "drawingengine.h"
#include <QVector2D>
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
    double xReal = std::sqrt(r * r - y * y);
    double xCeil = std::ceil(xReal);
    double D = xCeil - xReal;

    circlePlot(im, xc, yc, int(xCeil), int(y), 1.0 - D, col);
    circlePlot(im, xc, yc, int(xCeil) - 1, int(y), D, col);

    ++y;
  }
}

/* ---------- Half-circle --------------------------- */
static inline bool outsideHalf(int x, int y, int xc, int yc, double nx,
                               double ny) {
  return (x - xc) * nx + (y - yc) * ny >= 0;
}

/* Mid-point (aliased) */
void drawHalfCircleMidpoint(QImage &im, int xc, int yc, int r, double nx,
                            double ny, const QColor &col) {
  int x = 0, y = r, d = 1 - r;
  while (x <= y) {
    auto trySet = [&](int px, int py) {
      if (outsideHalf(px, py, xc, yc, nx, ny))
        setPixelSafe(im, px, py, col);
    };
    trySet(xc + x, yc + y);
    trySet(xc - x, yc + y);
    trySet(xc + x, yc - y);
    trySet(xc - x, yc - y);
    trySet(xc + y, yc + x);
    trySet(xc - y, yc + x);
    trySet(xc + y, yc - x);
    trySet(xc - y, yc - x);

    d < 0 ? d += 2 * x + 3 : (d += 2 * (x - y) + 5, --y);
    ++x;
  }
}

/* Wu (antialiased)*/
static inline void halfCirclePlotAA(QImage &im, int xc, int yc, int x, int y,
                                    double a, double nx, double ny,
                                    const QColor &c) {
  auto tryBlend = [&](int px, int py, double alpha) {
    if ((px - xc) * nx + (py - yc) * ny >= 0)
      blend(im, px, py, alpha, c);
  };

  tryBlend(xc + x, yc + y, a);
  tryBlend(xc - x, yc + y, a);
  tryBlend(xc + x, yc - y, a);
  tryBlend(xc - x, yc - y, a);
  tryBlend(xc + y, yc + x, a);
  tryBlend(xc - y, yc + x, a);
  tryBlend(xc + y, yc - x, a);
  tryBlend(xc - y, yc - x, a);
}

void drawHalfCircleWu(QImage &im, int xc, int yc, int r, double nx, double ny,
                      const QColor &col) {
  if (r <= 0)
    return;

  for (int y = 0; y <= r; ++y) {
    /* exactly the same math as the full-circle Wu */
    double xReal = std::sqrt(r * r - y * y);
    int xInt = int(std::floor(xReal));
    double D = xReal - xInt; // fractional part

    /* left pixel (xInt) gets weight 1-D, right one (xInt+1) gets D */
    halfCirclePlotAA(im, xc, yc, xInt + 1, y, D, nx, ny, col);
    halfCirclePlotAA(im, xc, yc, xInt, y, 1.0 - D, nx, ny, col);
  }
}

/* ---------- free‑hand ------------------------------------------------- */
void drawFreehandPen(QImage &im, const QVector<QPoint> &pts,
                     const QColor &col) {
  for (int i = 1; i < pts.size(); ++i)
    drawLineDDA(im, pts[i - 1].x(), pts[i - 1].y(), pts[i].x(), pts[i].y(),
                col);
}

/* ================================================================ */
/* Liang–Barsky clip (rectangle)                               */
/* ================================================================ */
static inline bool clipTest(double p, double q, double &t0, double &t1) {
  if (q < 0.0 && p == 0)
    return false;
  double r = q / p;
  if (p < 0) {
    if (r > t1)
      return false;
    else if (r > t0)
      t0 = r;
  } else if (p > 0) {
    if (r < t0)
      return false;
    else if (r < t1)
      t1 = r;
  } else if (q < 0)
    return false;
  return true;
}

bool liangBarskyClip(const QRect &R, QPointF p0, QPointF p1, QPointF &A,
                     QPointF &B) {
  double t0 = 0.0, t1 = 1.0;
  double dx = p1.x() - p0.x();
  double dy = p1.y() - p0.y();

  if (clipTest(-dx, p0.x() - R.left(), t0, t1) &&
      clipTest(dx, R.right() - p0.x(), t0, t1) &&
      clipTest(-dy, p0.y() - R.top(), t0, t1) &&
      clipTest(dy, R.bottom() - p0.y(), t0, t1)) {
    A = {p0.x() + t0 * dx, p0.y() + t0 * dy};
    B = {p0.x() + t1 * dx, p0.y() + t1 * dy};
    return true;
  }
  return false;
}

/* ================================================================ */
/* Edge-Table Scan-line fill                                   */
/* ================================================================ */
struct EdgeRec {
  int yMax;
  double x, invSlope;
};

static void bucketSortEdges(const QVector<QPoint> &P,
                            QVector<QVector<EdgeRec>> &ET, int &yMin,
                            int &yMax) {
  int n = P.size();
  yMin = INT_MAX;
  yMax = INT_MIN;
  for (int i = 0; i < n; ++i) {
    QPoint a = P[i], b = P[(i + 1) % n];
    if (a.y() == b.y())
      continue;
    if (a.y() > b.y())
      std::swap(a, b);

    EdgeRec e;
    e.yMax = b.y();
    e.x = a.x();
    e.invSlope = double(b.x() - a.x()) / double(b.y() - a.y());

    int k = a.y();
    if (k < yMin)
      yMin = k;
    if (e.yMax > yMax)
      yMax = e.yMax;
    if (k >= 0 && k < ET.size())
      ET[k].append(e);
  }
}

static void scanlineFill(QImage &img, const QVector<QPoint> &P,
                         const QColor *colour, const QImage *pattern) {
  if (P.size() < 3)
    return;
  int H = img.height();
  QVector<QVector<EdgeRec>> ET(H);
  int yMin, yMax;
  bucketSortEdges(P, ET, yMin, yMax);

  QVector<EdgeRec> AET;

  for (int y = yMin; y <= yMax; ++y) {
    AET += ET[y];

    for (int i = AET.size() - 1; i >= 0; --i)
      if (AET[i].yMax == y)
        AET.removeAt(i);

    std::sort(AET.begin(), AET.end(),
              [](const EdgeRec &a, const EdgeRec &b) { return a.x < b.x; });

    for (int i = 0; i + 1 < AET.size(); i += 2) {
      int x1 = int(std::ceil(AET[i].x));
      int x2 = int(std::floor(AET[i + 1].x));
      for (int x = x1; x <= x2; ++x) {
        if (x < 0 || x >= img.width() || y < 0 || y >= H)
          continue;
        if (colour)
          img.setPixel(x, y, colour->rgb());
        else {
          QRgb p = pattern->pixel(x % pattern->width(), y % pattern->height());
          img.setPixel(x, y, p);
        }
      }
    }

    for (EdgeRec &e : AET)
      e.x += e.invSlope;
  }
}

void fillPolygonET(QImage &img, const QVector<QPoint> &P,
                   const QColor &colour) {
  scanlineFill(img, P, &colour, nullptr);
}

void fillPolygonET(QImage &img, const QVector<QPoint> &P,
                   const QImage *pattern) {
  scanlineFill(img, P, nullptr, pattern);
}
