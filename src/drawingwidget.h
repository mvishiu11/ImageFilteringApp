#ifndef DRAWINGWIDGET_H
#define DRAWINGWIDGET_H

#include "shape.h"
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QFile>
#include <QList>
#include <QPoint>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
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

public slots:
  void clearCanvas();
  void loadShapes();
  void saveShapes();
  void deleteSelectedShape();

protected:
  void paintEvent(QPaintEvent *) override;
  void resizeEvent(QResizeEvent *) override;
  void mousePressEvent(QMouseEvent *) override;
  void mouseMoveEvent(QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;
  void mouseDoubleClickEvent(QMouseEvent *) override;

private slots:
  void onModeChanged(int);
  void onThicknessChanged(int);
  void onColorButtonClicked();
  void onAntiAliasToggled(bool);
  void onClearButtonClicked();
  void onDeleteButtonClicked();

private:
  /* gui widgets */
  QWidget *toolbarWidget;
  QComboBox *modeSelector;
  QSpinBox *thicknessSpin;
  QPushButton *colorButton;
  QCheckBox *antiAliasCheck;
  QPushButton *clearButton, *deleteButton;
  QSlider *zoomSlider;

  /* persistent canvas + shapes */
  QImage canvas;
  QList<Shape *> shapes;
  Shape *selectedShape = nullptr;

  /* state */
  DrawingMode currentMode = DM_Line;
  int lineThickness = 1;
  QColor drawingColor = Qt::black;
  bool antiAliasEnabled = true;
  int zoomFactor = 1;

  bool isDrawing = false;
  QVector<QPoint> currentPoints;
  QPoint lastMousePos;

  /* hit‑test info */
  enum HitType {
    None,
    LineP0,
    LineP1,
    LineBody,
    CircCenter,
    CircEdge,
    PolyVertex,
    PolyBody
  };
  HitType hit = None;
  int hitIndex = -1;

  /* helpers */
  QPoint mapToCanvas(const QPoint &p) const;
  void drawPreview(QPainter &);
  void commitCurrentShape();
  void selectShapeAt(const QPoint &);
  void moveSelectedShape(const QPoint &);
  void redrawAllShapes();
};
#endif
