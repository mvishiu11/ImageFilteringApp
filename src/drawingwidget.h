#ifndef DRAWINGWIDGET_H
#define DRAWINGWIDGET_H

#include "shape.h"
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QList>
#include <QPoint>
#include <QPushButton>
#include <QSpinBox>
#include <QVector>
#include <QWidget>

enum DrawingMode {
  DM_Line,
  DM_ThickLine,
  DM_Circle,
  DM_Polygon,
  DM_Pen,
  DM_Selection
};

class DrawingWidget : public QWidget {
  Q_OBJECT
public:
  explicit DrawingWidget(QWidget *parent = nullptr);
  ~DrawingWidget() override;

  void setMode(DrawingMode mode);
  void setLineThickness(int thickness);
  void setDrawingColor(const QColor &color);
  void setAntiAliasingEnabled(bool enabled);

public slots:
  void clearCanvas();
  void loadShapes();
  void saveShapes();
  void deleteSelectedShape();

protected:
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;

private slots:
  void onModeChanged(int idx);
  void onThicknessChanged(int val);
  void onColorButtonClicked();
  void onAntiAliasToggled(bool checked);
  void onClearButtonClicked();
  void onDeleteButtonClicked();

private:
  // persistent drawing canvas
  QImage canvas;
  // vector shapes
  QList<Shape *> shapes;
  Shape *selectedShape = nullptr;

  // toolbar
  QWidget *toolbarWidget;
  QComboBox *modeSelector;
  QSpinBox *thicknessSpin;
  QPushButton *colorButton;
  QCheckBox *antiAliasCheck;
  QPushButton *clearButton;
  QPushButton *deleteButton;
  QSlider *zoomSlider;

  // drawing state
  DrawingMode currentMode;
  int lineThickness;
  QColor drawingColor;
  bool antiAliasEnabled;
  int zoomFactor{1};

  bool isDrawing;
  QVector<QPoint> currentPoints;
  QPoint lastMousePos;

  // core routines
  void commitCurrentShape();
  void drawPreview(QPainter &p);
  void selectShapeAt(const QPoint &pos);
  void moveSelectedShape(const QPoint &delta);

  enum ShapeType : quint8 { ST_Line = 1, ST_Circle = 2, ST_Polygon = 3 };
  enum HitType {
    None,
    LineP0,
    LineP1,
    CircCenter,
    CircEdge,
    PolyVertex,
    PolyBody
  };
  HitType hit = None;
  int hitIndex = -1;

  QPoint mapToCanvas(const QPoint &p) const {
    int ox = (width() - canvas.width() * zoomFactor) / 2;
    int oy = toolbarWidget->height();
    return QPoint((p.x() - ox) / zoomFactor, (p.y() - oy) / zoomFactor);
  }

  void redrawAllShapes();
};

#endif // DRAWINGWIDGET_H
