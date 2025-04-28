#ifndef SHAPE_H
#define SHAPE_H
#include <QColor>
#include <QDataStream>
#include <QImage>
#include <QPoint>
#include <QVector>

class Shape {
public:
  Shape(const QColor &col = Qt::black, int thick = 1, bool aa = true)
      : drawingColor(col), lineThickness(thick), useAntiAlias(aa) {}
  virtual ~Shape() = default;

  virtual void draw(QImage &) const = 0;
  virtual void moveBy(int dx, int dy) = 0;

  virtual void write(QDataStream &) const = 0;
  virtual void read(QDataStream &) = 0;

  QColor drawingColor;
  int lineThickness;
  bool useAntiAlias;
};

class LineShape : public Shape {
public:
  LineShape() = default;
  LineShape(const QPoint &s, const QPoint &e, const QColor &c, int t, bool aa)
      : Shape(c, t, aa), p0(s), p1(e) {}
  void draw(QImage &) const override;
  void moveBy(int dx, int dy) override {
    p0 += QPoint(dx, dy);
    p1 += QPoint(dx, dy);
  }
  void write(QDataStream &) const override;
  void read(QDataStream &) override;
  QPoint p0, p1;
};

class CircleShape : public Shape {
public:
  CircleShape() = default;
  CircleShape(const QPoint &ctr, int r, const QColor &c, int t, bool aa)
      : Shape(c, t, aa), center(ctr), radius(r) {}
  void draw(QImage &) const override;
  void moveBy(int dx, int dy) override { center += QPoint(dx, dy); }
  void write(QDataStream &) const override;
  void read(QDataStream &) override;
  QPoint center;
  int radius;
};

class PolygonShape : public Shape {
public:
  PolygonShape() = default;
  PolygonShape(const QVector<QPoint> &v, const QColor &c, int t, bool aa)
      : Shape(c, t, aa), vertices(v) {}
  void draw(QImage &) const override;
  void moveBy(int dx, int dy) override;
  void write(QDataStream &) const override;
  void read(QDataStream &) override;
  QVector<QPoint> vertices;
};

class PillShape : public Shape {
public:
  PillShape() = default;
  PillShape(const QPoint &c0, const QPoint &c1, int rad, const QColor &col,
            int thick, bool aa)
      : Shape(col, thick, aa), p0(c0), p1(c1), radius(rad) {}

  void draw(QImage &) const override;
  void moveBy(int dx, int dy) override {
    p0 += QPoint(dx, dy);
    p1 += QPoint(dx, dy);
  }
  void write(QDataStream &) const override;
  void read(QDataStream &) override;

  QPoint p0, p1;
  int radius;
};

#endif
