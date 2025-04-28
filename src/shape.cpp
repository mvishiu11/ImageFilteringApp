#include "shape.h"
#include "drawingengine.h"
#include <QtMath>

/* -------- LineShape ---------------------------------------------------- */
void LineShape::draw(QImage &im) const {
  auto drawThin = useAntiAlias ? drawLineWu : drawLineDDA;
  int dx = p1.x() - p0.x();
  int dy = p1.y() - p0.y();
  bool horizontalish = std::abs(dx) >= std::abs(dy);
  int h = lineThickness / 2;

  for (int off = -h; off <= h; ++off) {
    if (horizontalish)
      drawThin(im, p0.x(), p0.y() + off, p1.x(), p1.y() + off, drawingColor);
    else
      drawThin(im, p0.x() + off, p0.y(), p1.x() + off, p1.y(), drawingColor);
  }
}
void LineShape::write(QDataStream &out) const {
  out << p0 << p1 << drawingColor << lineThickness << useAntiAlias;
}
void LineShape::read(QDataStream &in) {
  in >> p0 >> p1 >> drawingColor >> lineThickness >> useAntiAlias;
}

/* -------- CircleShape -------------------------------------------------- */
void CircleShape::draw(QImage &im) const {
  if (useAntiAlias) {
    drawCircleWu(im, center.x(), center.y(), radius, drawingColor);
  } else {
    int h = lineThickness / 2;
    for (int i = -h; i <= h; ++i) {
      int r2 = radius + i;
      if (r2 > 0)
        drawCircleMidpoint(im, center.x(), center.y(), r2, drawingColor);
    }
  }
}

void CircleShape::write(QDataStream &out) const {
  out << center << radius << drawingColor << lineThickness << useAntiAlias;
}

void CircleShape::read(QDataStream &in) {
  in >> center >> radius >> drawingColor >> lineThickness >> useAntiAlias;
}

/* -------- PolygonShape ------------------------------------------------- */
void PolygonShape::draw(QImage &im) const {
  if (vertices.size() < 2)
    return;
  int h = lineThickness / 2;
  for (int i = 0; i < vertices.size(); ++i) {
    QPoint a = vertices[i], b = vertices[(i + 1) % vertices.size()];
    for (int off = -h; off <= h; ++off) {
      if (useAntiAlias)
        drawLineWu(im, a.x(), a.y() + off, b.x(), b.y() + off, drawingColor);
      else
        drawLineDDA(im, a.x(), a.y() + off, b.x(), b.y() + off, drawingColor);
    }
  }
}
void PolygonShape::moveBy(int dx, int dy) {
  for (QPoint &pt : vertices)
    pt += QPoint(dx, dy);
}
void PolygonShape::write(QDataStream &out) const {
  out << drawingColor << lineThickness << useAntiAlias;
  out << quint32(vertices.size());
  for (const QPoint &pt : vertices)
    out << pt;
}
void PolygonShape::read(QDataStream &in) {
  quint32 n;
  in >> drawingColor >> lineThickness >> useAntiAlias >> n;
  vertices.resize(n);
  for (auto &i : vertices)
    in >> i;
}

/* -------- PillShape ------------------------------------------------- */

void PillShape::draw(QImage &im) const {
  auto drawLine = useAntiAlias ? drawLineWu : drawLineDDA;
  auto drawCap = useAntiAlias ? drawHalfCircleWu : drawHalfCircleMidpoint;

  double vx = p1.x() - p0.x(), vy = p1.y() - p0.y();
  double len = sqrt(vx*vx + vy*vy);
  if (len == 0)
    return;
  double nx = -vy / len, ny = vx / len;

  QPoint ofs(int(std::round(nx * radius)), int(std::round(ny * radius)));

  QPoint a0 = p0 + ofs;
  QPoint b0 = p1 + ofs;
  QPoint a1 = p0 - ofs;
  QPoint b1 = p1 - ofs;

  drawLine(im, a0.x(), a0.y(), b0.x(), b0.y(), drawingColor);
  drawLine(im, a1.x(), a1.y(), b1.x(), b1.y(), drawingColor);

  drawCap(im, p0.x(), p0.y(), radius, -vx, -vy, drawingColor);
  drawCap(im, p1.x(), p1.y(), radius, vx, vy, drawingColor);
}

void PillShape::write(QDataStream &out) const {
  out << p0 << p1 << radius << drawingColor << lineThickness << useAntiAlias;
}
void PillShape::read(QDataStream &in) {
  in >> p0 >> p1 >> radius >> drawingColor >> lineThickness >> useAntiAlias;
}
