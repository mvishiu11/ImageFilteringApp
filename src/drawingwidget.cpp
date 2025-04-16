#include "drawingwidget.h"
#include "drawingengine.h"

#include <QColorDialog>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QToolButton>
#include <QVBoxLayout>
#include <cmath>

DrawingWidget::DrawingWidget(QWidget *parent)
    : QWidget(parent), currentMode(DM_Line), lineThickness(1),
      drawingColor(Qt::black), antiAliasEnabled(true), isDrawing(false) {
  // === build toolbar ===
  toolbarWidget = new QWidget(this);
  auto *toolbarLayout = new QHBoxLayout;
  toolbarLayout->setContentsMargins(5, 5, 5, 5);
  toolbarLayout->setSpacing(10);

  modeSelector = new QComboBox; // no need 'this' parent; we reparent below
  modeSelector->addItem("Line", DM_Line);
  modeSelector->addItem("Thick Line", DM_ThickLine);
  modeSelector->addItem("Circle", DM_Circle);
  modeSelector->addItem("Polygon", DM_Polygon);
  modeSelector->addItem("Pen", DM_Pen);
  modeSelector->addItem("Select", DM_Selection);
  toolbarLayout->addWidget(new QLabel("Tool:"));
  toolbarLayout->addWidget(modeSelector);

  thicknessSpin = new QSpinBox;
  thicknessSpin->setRange(1, 20);
  thicknessSpin->setValue(lineThickness);
  toolbarLayout->addWidget(new QLabel("Thickness:"));
  toolbarLayout->addWidget(thicknessSpin);

  colorButton = new QPushButton("Color");
  colorButton->setMinimumWidth(60);
  colorButton->setStyleSheet("background-color: black; color: white;");
  toolbarLayout->addWidget(colorButton);

  antiAliasCheck = new QCheckBox("Anti-Alias");
  antiAliasCheck->setChecked(true);
  toolbarLayout->addWidget(antiAliasCheck);

  clearButton = new QPushButton("Clear");
  toolbarLayout->addWidget(clearButton);

  deleteButton = new QPushButton("Delete");
  toolbarLayout->addWidget(deleteButton);

  QToolButton *loadBtn = new QToolButton;
  loadBtn->setIcon(QIcon(":/icons/folder-open.svg"));
  QToolButton *saveBtn = new QToolButton;
  saveBtn->setIcon(QIcon(":/icons/disk.svg"));
  toolbarLayout->addWidget(loadBtn);
  toolbarLayout->addWidget(saveBtn);

  zoomSlider = new QSlider(Qt::Horizontal);
  zoomSlider->setRange(1, 8);
  zoomSlider->setValue(1);
  toolbarLayout->addWidget(new QLabel("Zoom"));
  toolbarLayout->addWidget(zoomSlider);

  connect(loadBtn, &QToolButton::clicked, this, &DrawingWidget::loadShapes);
  connect(saveBtn, &QToolButton::clicked, this, &DrawingWidget::saveShapes);
  connect(zoomSlider, &QSlider::valueChanged, this, [this](int v) {
    zoomFactor = v;
    update();
  });

  toolbarWidget->setLayout(toolbarLayout);

  // === main layout ===
  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->addWidget(toolbarWidget);
  mainLayout->addStretch(1);
  setLayout(mainLayout);

  // persistent canvas
  canvas = QImage(800, 600, QImage::Format_RGB32);
  canvas.fill(Qt::white);

  // connect signals
  connect(modeSelector, SIGNAL(currentIndexChanged(int)), this,
          SLOT(onModeChanged(int)));
  connect(thicknessSpin, SIGNAL(valueChanged(int)), this,
          SLOT(onThicknessChanged(int)));
  connect(colorButton, &QPushButton::clicked, this,
          &DrawingWidget::onColorButtonClicked);
  connect(antiAliasCheck, SIGNAL(toggled(bool)), this,
          SLOT(onAntiAliasToggled(bool)));
  connect(clearButton, &QPushButton::clicked, this,
          &DrawingWidget::onClearButtonClicked);
  connect(deleteButton, &QPushButton::clicked, this,
          &DrawingWidget::onDeleteButtonClicked);

  setMouseTracking(true);
}

DrawingWidget::~DrawingWidget() { qDeleteAll(shapes); }

// --- toolbar slots ---
void DrawingWidget::onModeChanged(int idx) {
  setMode(static_cast<DrawingMode>(modeSelector->itemData(idx).toInt()));
}

void DrawingWidget::onThicknessChanged(int v) { setLineThickness(v); }

void DrawingWidget::onColorButtonClicked() {
  QColor c = QColorDialog::getColor(drawingColor, this, "Select Color");
  if (!c.isValid())
    return;
  drawingColor = c;
  colorButton->setStyleSheet(
      QString("background-color:%1; color:white;").arg(c.name()));
}

void DrawingWidget::onAntiAliasToggled(bool chk) {
  setAntiAliasingEnabled(chk);
  // redraw *all* committed shapes with new AA flag:
  canvas.fill(Qt::white);
  for (auto *s : shapes) {
    s->useAntiAlias = chk;
    s->draw(canvas);
  }
  update();
}

void DrawingWidget::onClearButtonClicked() { clearCanvas(); }

void DrawingWidget::onDeleteButtonClicked() { deleteSelectedShape(); }

// --- public slots ---
void DrawingWidget::clearCanvas() {
  canvas.fill(Qt::white);
  qDeleteAll(shapes);
  shapes.clear();
  selectedShape = nullptr;
  update();
}

void DrawingWidget::loadShapes() {
  QString fn =
      QFileDialog::getOpenFileName(this, "Load Shapes", "", "Shape (*.shp)");
  if (fn.isEmpty())
    return;
  QFile f(fn);
  if (!f.open(QIODevice::ReadOnly))
    return;
  QDataStream in(&f);
  in.setVersion(QDataStream::Qt_6_0);
  char hdr[4];
  if (in.readRawData(hdr, 4) != 4 || strncmp(hdr, "VECT", 4) != 0)
    return;
  quint32 count;
  in >> count;
  clearCanvas(); // wipe current
  for (quint32 i = 0; i < count; ++i) {
    quint8 t;
    in >> t;
    Shape *s = nullptr;
    if (t == ST_Line)
      s = new LineShape;
    if (t == ST_Circle)
      s = new CircleShape;
    if (t == ST_Polygon)
      s = new PolygonShape;
    if (!s) {
      in.skipRawData(1);
      continue;
    }
    s->read(in);
    shapes.append(s);
    s->draw(canvas);
  }
  update();
}

void DrawingWidget::saveShapes() {
  QString fn =
      QFileDialog::getSaveFileName(this, "Save Shapes", "", "Shape (*.shp)");
  if (fn.isEmpty())
    return;
  QFile f(fn);
  if (!f.open(QIODevice::WriteOnly))
    return;
  QDataStream out(&f);
  out.setVersion(QDataStream::Qt_6_0);
  out.writeRawData("VECT", 4);
  out << quint32(shapes.size());
  for (auto *s : shapes) {
    if (auto *L = dynamic_cast<LineShape *>(s)) {
      out << quint8(ST_Line);
      L->write(out);
    }
    if (auto *C = dynamic_cast<CircleShape *>(s)) {
      out << quint8(ST_Circle);
      C->write(out);
    }
    if (auto *P = dynamic_cast<PolygonShape *>(s)) {
      out << quint8(ST_Polygon);
      P->write(out);
    }
  }
}

void DrawingWidget::deleteSelectedShape() {
  if (!selectedShape)
    return;
  shapes.removeOne(selectedShape);
  delete selectedShape;
  selectedShape = nullptr;
  // re-render all shapes:
  canvas.fill(Qt::white);
  for (auto *s : shapes)
    s->draw(canvas);
  update();
}

// --- setters ---
void DrawingWidget::setMode(DrawingMode m) {
  currentMode = m;
  isDrawing = false;
  currentPoints.clear();
  update();
}
void DrawingWidget::setLineThickness(int t) { lineThickness = t; }
void DrawingWidget::setDrawingColor(const QColor &c) { drawingColor = c; }
void DrawingWidget::setAntiAliasingEnabled(bool e) { antiAliasEnabled = e; }

// --- painting ---
void DrawingWidget::paintEvent(QPaintEvent *) {
  QPainter p(this);
  int ox = (width() - canvas.width()) / 2;
  int oy = toolbarWidget->height();

  p.translate(ox, oy);
  p.scale(zoomFactor, zoomFactor);
  p.drawImage(0, 0, canvas);
  p.resetTransform();

  // selection highlight
  if (currentMode == DM_Selection && selectedShape) {
    QPen pen(Qt::red);
    pen.setWidth(2);
    p.setPen(pen);
    if (auto *L = dynamic_cast<LineShape *>(selectedShape)) {
      QRect r(L->p0, L->p1);
      p.drawRect(r.normalized().adjusted(-5, -5, 5, 5));
    }
    // extend for circles/polygons...
  }

  // draw the live preview on top
  drawPreview(p);
}

void DrawingWidget::drawPreview(QPainter &painter) {
  if (!isDrawing || currentPoints.isEmpty())
    return;

  QImage tmp = canvas;
  QPainter p(&tmp);
  p.setPen(drawingColor);

  // LINE & THICK LINE
  if ((currentMode == DM_Line || currentMode == DM_ThickLine) &&
      currentPoints.size() >= 2) {
    int x0 = currentPoints[0].x(), y0 = currentPoints[0].y();
    int x1 = currentPoints[1].x(), y1 = currentPoints[1].y();
    if (currentMode == DM_ThickLine) {
      int h = lineThickness / 2;
      for (int off = -h; off <= h; ++off) {
        if (antiAliasEnabled)
          drawLineWu(tmp, x0, y0 + off, x1, y1 + off, drawingColor);
        else
          drawLineDDA(tmp, x0, y0 + off, x1, y1 + off, drawingColor);
      }
    } else {
      if (antiAliasEnabled)
        drawLineWu(tmp, x0, y0, x1, y1, drawingColor);
      else
        drawLineDDA(tmp, x0, y0, x1, y1, drawingColor);
    }
  }
  // CIRCLE
  else if (currentMode == DM_Circle && currentPoints.size() >= 2) {
    int dx = currentPoints[1].x() - currentPoints[0].x();
    int dy = currentPoints[1].y() - currentPoints[0].y();
    int r = int(std::sqrt(dx * dx + dy * dy));
    drawCircleMidpoint(tmp, currentPoints[0].x(), currentPoints[0].y(), r,
                       drawingColor);
  }
  // PEN
  else if (currentMode == DM_Pen) {
    drawFreehandPen(tmp, currentPoints, drawingColor);
  }
  // POLYGON
  if (currentMode == DM_Polygon && currentPoints.size() >= 2) {
    // draw all edges
    for (int i = 0; i + 1 < currentPoints.size(); ++i) {
      p.drawLine(currentPoints[i], currentPoints[i + 1]);
    }
    // draw each vertex
    for (const QPoint &pt : currentPoints) {
      p.drawEllipse(pt, 3, 3);
    }
    // preview closure if we're within threshold
    if ((currentPoints.last() - currentPoints.first()).manhattanLength() < 15) {
      p.drawLine(currentPoints.last(), currentPoints.first());
    }
  }

  int ox = (width() - canvas.width()) / 2;
  int oy = toolbarWidget->height();
  painter.drawImage(ox, oy, tmp);
}

void DrawingWidget::redrawAllShapes() {
  canvas.fill(Qt::white);
  for (auto *s : shapes)
    s->draw(canvas);
}

// --- mouse handling ---
void DrawingWidget::mousePressEvent(QMouseEvent *event) {
  QPoint pos = mapToCanvas(event->pos());
  // Bounds check omitted for brevity…

  if (currentMode == DM_Polygon) {
    if (!isDrawing) {
      // first click: start a new polygon
      isDrawing = true;
      currentPoints.clear();
      currentPoints.append(pos);
    }
    // otherwise do nothing here
  } else {
    // Line/Circle/Pen/etc: old behavior
    isDrawing = true;
    currentPoints.clear();
    currentPoints.append(pos);
  }
  update();
}

void DrawingWidget::mouseMoveEvent(QMouseEvent *ev) {
  QPoint pos = mapToCanvas(ev->pos());
  if (currentMode == DM_Selection && selectedShape &&
      (ev->buttons() & Qt::LeftButton)) {
    QPoint delta = pos - lastMousePos;
    if (auto *L = dynamic_cast<LineShape *>(selectedShape)) {
      if (hit == LineP0)
        L->p0 += delta;
      else if (hit == LineP1)
        L->p1 += delta;
    } else if (auto *C = dynamic_cast<CircleShape *>(selectedShape)) {
      if (hit == CircCenter)
        C->center += delta;
      else if (hit == CircEdge)
        C->radius =
            int(std::hypot(pos.x() - C->center.x(), pos.y() - C->center.y()));
    }
    // polygon vertex drag similar
    lastMousePos = pos;
    redrawAllShapes();
    update();
    return;
  } else if (isDrawing) {
    if (currentMode == DM_Polygon) {
      if (!currentPoints.isEmpty())
        currentPoints.last() = pos;
      else
        currentPoints.append(pos);
    } else if (currentMode == DM_Pen) {
      currentPoints.append(pos);
    } else {
      if (currentPoints.size() < 2)
        currentPoints.append(pos);
      else
        currentPoints[1] = pos;
    }
  }
  update();
}

void DrawingWidget::mouseReleaseEvent(QMouseEvent *event) {
  QPoint pos = mapToCanvas(event->pos());
  // Bounds check omitted…

  if (currentMode == DM_Polygon && isDrawing) {
    if (currentPoints.size() >= 3 &&
        (pos - currentPoints.first()).manhattanLength() < 15) {
      // close & commit polygon
      currentPoints.append(currentPoints.first()); // ensure closure
      commitCurrentShape();
      isDrawing = false;
    } else {
      // add a new vertex
      currentPoints.append(pos);
    }
  } else if (currentMode != DM_Polygon && isDrawing) {
    // existing behavior for lines/circles/pen
    if (currentPoints.size() < 2)
      currentPoints.append(pos);
    else
      currentPoints[1] = pos;
    commitCurrentShape();
    isDrawing = false;
  }
  update();
}

void DrawingWidget::mouseDoubleClickEvent(QMouseEvent *ev) {
  if (currentMode == DM_Polygon && isDrawing && currentPoints.size() >= 3) {
    currentPoints.append(currentPoints.first());
    commitCurrentShape();
    isDrawing = false;
    update();
    return;
  }
  if (currentMode == DM_Selection) {
    QPoint pos = mapToCanvas(ev->pos());
    selectShapeAt(pos);
    if (selectedShape) {
      shapes.removeOne(selectedShape);
      delete selectedShape;
      selectedShape = nullptr;
      // re‑draw all
      canvas.fill(Qt::white);
      for (auto *s : shapes)
        s->draw(canvas);
      update();
    }
  }
}

void DrawingWidget::resizeEvent(QResizeEvent *e) {
  QWidget::resizeEvent(e);
  update();
}

// --- helpers ---
void DrawingWidget::commitCurrentShape() {
  Shape *ns = nullptr;
  switch (currentMode) {
  case DM_Line:
  case DM_ThickLine:
    if (currentPoints.size() < 2)
      return;
    ns = new LineShape(currentPoints[0], currentPoints[1], drawingColor,
                       lineThickness, antiAliasEnabled);
    break;
  case DM_Circle:
    if (currentPoints.size() < 2)
      return;
    {
      int dx = currentPoints[1].x() - currentPoints[0].x();
      int dy = currentPoints[1].y() - currentPoints[0].y();
      int r = int(std::sqrt(dx * dx + dy * dy));
      ns = new CircleShape(currentPoints[0], r, drawingColor, lineThickness,
                           antiAliasEnabled);
    }
    break;
  case DM_Pen:
    if (currentPoints.isEmpty())
      return;
    ns = new PolygonShape(currentPoints, drawingColor, lineThickness,
                          antiAliasEnabled);
    break;
  case DM_Polygon:
    if (currentPoints.size() < 3)
      return;
    ns = new PolygonShape(currentPoints, drawingColor, lineThickness,
                          antiAliasEnabled);
    break;
  default:
    return;
  }
  // draw *once* onto the persistent canvas
  ns->draw(canvas);
  shapes.append(ns);
}

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

void DrawingWidget::selectShapeAt(const QPoint &pos) {
  const int T = 7;
  selectedShape = nullptr;
  hit = None;
  for (auto *s : shapes) {
    if (auto *L = dynamic_cast<LineShape *>(s)) {
      if ((pos - L->p0).manhattanLength() < T) {
        selectedShape = L;
        hit = LineP0;
        return;
      }
      if ((pos - L->p1).manhattanLength() < T) {
        selectedShape = L;
        hit = LineP1;
        return;
      }
    }
    if (auto *C = dynamic_cast<CircleShape *>(s)) {
      if ((pos - C->center).manhattanLength() < T) {
        selectedShape = C;
        hit = CircCenter;
        return;
      }
      int d = int(std::hypot(pos.x() - C->center.x(), pos.y() - C->center.y()));
      if (std::abs(d - C->radius) < T) {
        selectedShape = C;
        hit = CircEdge;
        return;
      }
    }
    if (auto *P = dynamic_cast<PolygonShape *>(s)) {
      for (auto &pt : P->vertices) {
        if ((pos - pt).manhattanLength() < T) {
          selectedShape = P;
          break;
        }
      }
      if (selectedShape)
        break;
    }
  }
}

void DrawingWidget::moveSelectedShape(const QPoint &d) {
  if (selectedShape) {
    selectedShape->moveBy(d.x(), d.y());
    // redraw all:
    canvas.fill(Qt::white);
    for (auto *s : shapes)
      s->draw(canvas);
  }
}
