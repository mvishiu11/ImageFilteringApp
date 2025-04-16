#include "shape.h"
#include "drawingengine.h"
#include <QDataStream>
#include <QtMath>

//--------------------//
//      LineShape     //
//--------------------//

void LineShape::draw(QImage &img) const {
  // For thick lines, draw several parallel lines.
  int half = lineThickness / 2;
  for (int i = -half; i <= half; ++i) {
    if (useAntiAlias)
      drawLineWu(img, p0.x(), p0.y() + i, p1.x(), p1.y() + i, drawingColor);
    else
      drawLineDDA(img, p0.x(), p0.y() + i, p1.x(), p1.y() + i, drawingColor);
  }
}

void LineShape::moveBy(int dx, int dy) {
  p0 += QPoint(dx, dy);
  p1 += QPoint(dx, dy);
}

void LineShape::write(QDataStream &out) const {
  out << p0 << p1 << drawingColor << lineThickness << useAntiAlias;
}

void LineShape::read(QDataStream &in) {
  in >> p0 >> p1 >> drawingColor >> lineThickness >> useAntiAlias;
}

//--------------------//
//     CircleShape    //
//--------------------//

void CircleShape::draw(QImage &img) const {
  if (lineThickness <= 1) {
    // For a single pixel thick circle, just draw normally.
    drawCircleMidpoint(img, center.x(), center.y(), radius, drawingColor);
  } else {
    int half = lineThickness / 2;
    // For thick circles, draw multiple concentric circles.
    for (int i = -half; i <= half; i++) {
      int r = radius + i;
      if (r > 0)
        drawCircleMidpoint(img, center.x(), center.y(), r, drawingColor);
    }
  }
}

void CircleShape::moveBy(int dx, int dy) { center += QPoint(dx, dy); }

void CircleShape::write(QDataStream &out) const {
  out << center << radius << drawingColor << lineThickness << useAntiAlias;
}

void CircleShape::read(QDataStream &in) {
  in >> center >> radius >> drawingColor >> lineThickness >> useAntiAlias;
}

//--------------------//
//    PolygonShape    //
//--------------------//

void PolygonShape::draw(QImage &img) const {
  int n = vertices.size();
  if (n < 2)
    return;
  // For each edge, draw it with the specified thickness.
  int half = lineThickness / 2;
  for (int i = 0; i < n; ++i) {
    QPoint p0 = vertices[i];
    QPoint p1 = vertices[(i + 1) % n]; // wrap around to close polygon
    for (int offset = -half; offset <= half; ++offset) {
      if (useAntiAlias)
        drawLineWu(img, p0.x(), p0.y() + offset, p1.x(), p1.y() + offset,
                   drawingColor);
      else
        drawLineDDA(img, p0.x(), p0.y() + offset, p1.x(), p1.y() + offset,
                    drawingColor);
    }
  }
}

void PolygonShape::moveBy(int dx, int dy) {
  for (int i = 0; i < vertices.size(); ++i) {
    vertices[i] += QPoint(dx, dy);
  }
}

void PolygonShape::write(QDataStream &out) const {
  out << drawingColor << lineThickness << useAntiAlias;
  out << vertices.size();
  for (const QPoint &pt : vertices)
    out << pt;
}

void PolygonShape::read(QDataStream &in) {
  int n = 0;
  in >> drawingColor >> lineThickness >> useAntiAlias;
  in >> n;
  vertices.resize(n);
  for (int i = 0; i < n; ++i)
    in >> vertices[i];
}
