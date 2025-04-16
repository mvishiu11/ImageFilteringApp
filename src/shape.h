#ifndef SHAPE_H
#define SHAPE_H

#include <QColor>
#include <QDataStream>
#include <QImage>
#include <QPoint>
#include <QVector>

/**
 * @brief Abstract base class representing a drawable vector shape.
 */
class Shape {
public:
  Shape(const QColor &color = Qt::black, int thickness = 1,
        bool antiAlias = true)
      : drawingColor(color), lineThickness(thickness), useAntiAlias(antiAlias) {
  }
  virtual ~Shape() {}

  virtual void draw(QImage &img) const = 0;
  virtual void moveBy(int dx, int dy) = 0;

  // Serialization – write shape data to a stream.
  virtual void write(QDataStream &out) const = 0;
  // Deserialization – read shape data from a stream.
  virtual void read(QDataStream &in) = 0;

  QColor drawingColor;
  int lineThickness;
  bool useAntiAlias;
};

/**
 * @brief Line shape drawn between two points.
 */
class LineShape : public Shape {
public:
  LineShape() {}
  LineShape(const QPoint &start, const QPoint &end,
            const QColor &color = Qt::black, int thickness = 1, bool aa = true)
      : Shape(color, thickness, aa), p0(start), p1(end) {}
  void draw(QImage &img) const override;
  void moveBy(int dx, int dy) override;
  void write(QDataStream &out) const override;
  void read(QDataStream &in) override;

  QPoint p0, p1;
};

/**
 * @brief Circle shape defined by center and radius.
 */
class CircleShape : public Shape {
public:
  CircleShape() {}
  CircleShape(const QPoint &center, int radius, const QColor &color = Qt::black,
              int thickness = 1, bool aa = true)
      : Shape(color, thickness, aa), center(center), radius(radius) {}
  void draw(QImage &img) const override;
  void moveBy(int dx, int dy) override;
  void write(QDataStream &out) const override;
  void read(QDataStream &in) override;

  QPoint center;
  int radius;
};

/**
 * @brief Polygon shape composed of vertices.
 */
class PolygonShape : public Shape {
public:
  PolygonShape() {}
  PolygonShape(const QVector<QPoint> &vertices, const QColor &color = Qt::black,
               int thickness = 1, bool aa = true)
      : Shape(color, thickness, aa), vertices(vertices) {}
  void draw(QImage &img) const override;
  void moveBy(int dx, int dy) override;
  void write(QDataStream &out) const override;
  void read(QDataStream &in) override;

  QVector<QPoint> vertices;
};

#endif // SHAPE_H
