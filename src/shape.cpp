#include "shape.h"
#include "drawingengine.h"
#include <QtMath>

/* -------- LineShape ---------------------------------------------------- */
void LineShape::draw(QImage &im) const {
  int h = lineThickness / 2;
  for (int off = -h; off <= h; ++off) {
    if (useAntiAlias)
      drawLineWu(im, p0.x(), p0.y() + off, p1.x(), p1.y() + off, drawingColor);
    else
      drawLineDDA(im, p0.x(), p0.y() + off, p1.x(), p1.y() + off, drawingColor);
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
