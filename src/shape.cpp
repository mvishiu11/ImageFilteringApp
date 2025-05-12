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
  if (fill != Qt::transparent || hasImage) {
    if (hasImage)
      fillPolygonET(im, vertices, &sample);
    else
      fillPolygonET(im, vertices, fill);
  }
  for (int i = 0; i < vertices.size(); ++i) {
    QPoint a = vertices[i], b = vertices[(i + 1) % vertices.size()];
    for (int off = -h; off <= h; ++off) {
      if (useAntiAlias)
        drawLineWu(im, a.x(), a.y() + off, b.x(), b.y() + off, drawingColor);
      else
        drawLineDDA(im, a.x(), a.y() + off, b.x(), b.y() + off, drawingColor);
    }
  }
  extern QList<RectangleShape *> gClipRects;

  for (RectangleShape *R : gClipRects) {
    QRect rect(R->p1, R->p2);
    for (int i = 0; i < vertices.size(); ++i) {
      QPointF a = vertices[i], b = vertices[(i + 1) % vertices.size()];
      QPointF A, B;
      if (liangBarskyClip(rect, a, b, A, B))
        drawLineDDA(im, int(A.x()), int(A.y()), int(B.x()), int(B.y()),
                    Qt::red);
    }
  }
}

void PolygonShape::moveBy(int dx, int dy) {
  for (QPoint &pt : vertices)
    pt += QPoint(dx, dy);
}

void PolygonShape::write(QDataStream &out) const {
  out << drawingColor << fill << hasImage << imagePath << lineThickness
      << useAntiAlias;

  out << quint32(vertices.size());
  for (const QPoint &pt : vertices)
    out << pt;
}

void PolygonShape::read(QDataStream &in) {
  quint32 nVerts = 0;

  in >> drawingColor >> fill >> hasImage >> imagePath >> lineThickness >>
      useAntiAlias >> nVerts;

  vertices.resize(nVerts);
  for (QPoint &pt : vertices)
    in >> pt;

  if (hasImage)
    sample.load(imagePath);
}

/* -------- RectangleShape ------------------------------------------------- */

void RectangleShape::draw(QImage &im) const {
  // outline
  auto L = useAntiAlias ? drawLineWu : drawLineDDA;

  QPoint a(p1.x(), p1.y()), b(p2.x(), p1.y()), c(p2.x(), p2.y()),
      d(p1.x(), p2.y());

  // fill if requested
  if (fill != Qt::transparent || hasImage) {
    QVector<QPoint> poly{a, b, c, d};
    if (hasImage)
      fillPolygonET(im, poly, &sample);
    else
      fillPolygonET(im, poly, fill);
  }

  L(im, a.x(), a.y(), b.x(), b.y(), drawingColor);
  L(im, b.x(), b.y(), c.x(), c.y(), drawingColor);
  L(im, c.x(), c.y(), d.x(), d.y(), drawingColor);
  L(im, d.x(), d.y(), a.x(), a.y(), drawingColor);
}

void RectangleShape::write(QDataStream &out) const {
  out << p1 << p2 << drawingColor << fill << hasImage << imagePath
      << useAntiAlias;
}
void RectangleShape::read(QDataStream &in) {
  in >> p1 >> p2 >> drawingColor >> fill >> hasImage >> imagePath >>
      useAntiAlias;
  if (hasImage)
    sample.load(imagePath);
}

/* -------- PillShape ------------------------------------------------- */

void PillShape::draw(QImage &im) const {
  auto drawLine = useAntiAlias ? drawLineWu : drawLineDDA;
  auto drawCap = useAntiAlias ? drawHalfCircleWu : drawHalfCircleMidpoint;

  double vx = p1.x() - p0.x(), vy = p1.y() - p0.y();
  double len = sqrt(vx * vx + vy * vy);
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
