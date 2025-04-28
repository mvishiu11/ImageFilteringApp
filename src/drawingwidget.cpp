#include "drawingwidget.h"
#include "drawingengine.h"
#include <QColorDialog>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QToolButton>
#include <cmath>

DrawingWidget::DrawingWidget(QWidget *parent) : QWidget(parent) {
  /* toolbar ---------------------------------------------------------- */
  toolbarWidget = new QWidget(this);
  auto *tl = new QHBoxLayout(toolbarWidget);
  tl->setContentsMargins(5, 5, 5, 5);
  tl->setSpacing(10);

  modeSelector = new QComboBox;
  tl->addWidget(new QLabel("Tool:"));
  tl->addWidget(modeSelector);
  modeSelector->addItem("Line", DM_Line);
  modeSelector->addItem("Thick Line", DM_ThickLine);
  modeSelector->addItem("Circle", DM_Circle);
  modeSelector->addItem("Polygon", DM_Polygon);
  modeSelector->addItem("Pen", DM_Pen);
  modeSelector->addItem("Pill", DM_Pill);
  modeSelector->addItem("Select", DM_Selection);

  thicknessSpin = new QSpinBox;
  thicknessSpin->setRange(1, 20);
  tl->addWidget(new QLabel("Thickness:"));
  tl->addWidget(thicknessSpin);

  colorButton = new QPushButton("Color");
  colorButton->setMinimumWidth(60);
  colorButton->setStyleSheet("background-color:black; color:white;");
  tl->addWidget(colorButton);

  antiAliasCheck = new QCheckBox("Anti‑Alias");
  antiAliasCheck->setChecked(true);
  tl->addWidget(antiAliasCheck);

  clearButton = new QPushButton("Clear");
  tl->addWidget(clearButton);
  deleteButton = new QPushButton("Delete");
  tl->addWidget(deleteButton);

  QToolButton *loadBtn = new QToolButton;
  loadBtn->setIcon(QIcon(":/icons/icons/folder-open.svg"));
  QToolButton *saveBtn = new QToolButton;
  saveBtn->setIcon(QIcon(":/icons/icons/disk.svg"));
  tl->addWidget(loadBtn);
  tl->addWidget(saveBtn);

  zoomSlider = new QSlider(Qt::Horizontal);
  zoomSlider->setRange(1, 8);
  zoomSlider->setValue(1);
  tl->addWidget(new QLabel("Zoom"));
  tl->addWidget(zoomSlider);

  /* layout ----------------------------------------------------------- */
  auto *ml = new QVBoxLayout(this);
  ml->addWidget(toolbarWidget);
  ml->addStretch(1);
  setLayout(ml);

  /* canvas ----------------------------------------------------------- */
  canvas = QImage(1000, 800, QImage::Format_RGB32);
  canvas.fill(Qt::white);

  /* connections ------------------------------------------------------ */
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
  connect(loadBtn, &QToolButton::clicked, this, &DrawingWidget::loadShapes);
  connect(saveBtn, &QToolButton::clicked, this, &DrawingWidget::saveShapes);
  connect(zoomSlider, &QSlider::valueChanged, this, [this](int v) {
    zoomFactor = v;
    update();
  });

  setMouseTracking(true);
}

DrawingWidget::~DrawingWidget() { qDeleteAll(shapes); }

/* ---- toolbar slots --------------------------------------------------- */
void DrawingWidget::onModeChanged(int i) {
  currentMode = DrawingMode(modeSelector->itemData(i).toInt());
  isDrawing = false;
  currentPoints.clear();
  update();
}

void DrawingWidget::onThicknessChanged(int v) { lineThickness = v; }

void DrawingWidget::onColorButtonClicked() {
  QColor c = QColorDialog::getColor(drawingColor, this);
  if (c.isValid()) {
    drawingColor = c;
    colorButton->setStyleSheet(
        QString("background-color:%1; color:white;").arg(c.name()));
  }
}

void DrawingWidget::onAntiAliasToggled(bool b) {
  antiAliasEnabled = b;
  for (auto *s : shapes)
    s->useAntiAlias = b;
  redrawAllShapes();
  update();
}

void DrawingWidget::onClearButtonClicked() { clearCanvas(); }
void DrawingWidget::onDeleteButtonClicked() { deleteSelectedShape(); }

/* ---- public slots ---------------------------------------------------- */
void DrawingWidget::clearCanvas() {
  canvas.fill(Qt::white);
  qDeleteAll(shapes);
  shapes.clear();
  selectedShape = nullptr;
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
  out.writeRawData("VECT", 4);
  out << quint32(shapes.size());
  for (auto *s : shapes) {
    if (dynamic_cast<LineShape *>(s)) {
      out << quint8(1);
      s->write(out);
    } else if (dynamic_cast<CircleShape *>(s)) {
      out << quint8(2);
      s->write(out);
    } else if (dynamic_cast<PolygonShape *>(s)) {
      out << quint8(3);
      s->write(out);
    } if (dynamic_cast<PillShape *>(s)) {
        out << quint8(4); s->write(out);
    }
  }
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
  char hdr[4];
  if (in.readRawData(hdr, 4) != 4 || strncmp(hdr, "VECT", 4))
    return;
  quint32 count;
  in >> count;
  clearCanvas();
  for (quint32 i = 0; i < count; ++i) {
    quint8 t;
    in >> t;
    Shape *s = nullptr;
    if (t == 1)
      s = new LineShape;
    if (t == 2)
      s = new CircleShape;
    if (t == 3)
      s = new PolygonShape;
    if (t == 4)
      s = new PillShape;
    if (!s) {
      f.close();
      return;
    }
    s->read(in);
    shapes.append(s);
    s->draw(canvas);
  }
  update();
}

void DrawingWidget::deleteSelectedShape() {
  if (!selectedShape)
    return;
  shapes.removeOne(selectedShape);
  delete selectedShape;
  selectedShape = nullptr;
  redrawAllShapes();
  update();
}

/* ---- helpers --------------------------------------------------------- */
QPoint DrawingWidget::mapToCanvas(const QPoint &p) const {
  int ox = (width() - canvas.width() * zoomFactor) / 2;
  int oy = toolbarWidget->height();
  return QPoint((p.x() - ox) / zoomFactor, (p.y() - oy) / zoomFactor);
}

void DrawingWidget::redrawAllShapes() {
  canvas.fill(Qt::white);
  for (auto *s : shapes)
    s->draw(canvas);
}

/* ---- paintEvent ------------------------------------------------------- */
static void drawCenterMarker(QPainter &p, const QPoint &c, int zoom) {
  const int r = 4;
  QPen pen(Qt::red, 1);
  pen.setCosmetic(true);
  p.setPen(pen);
  QPoint zc(c * zoom);
  p.drawLine(zc + QPoint(-r, 0), zc + QPoint(r, 0));
  p.drawLine(zc + QPoint(0, -r), zc + QPoint(0, r));
}

void DrawingWidget::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  int ox = (width() - canvas.width() * zoomFactor) / 2;
  int oy = toolbarWidget->height();
  painter.translate(ox, oy);
  painter.scale(zoomFactor, zoomFactor);
  painter.drawImage(0, 0, canvas);
  painter.resetTransform();
  if (currentMode == DM_Selection && selectedShape) {
    QPen pen(Qt::red, 2);
    pen.setCosmetic(true);
    painter.setPen(pen);
    if (auto *L = dynamic_cast<LineShape *>(selectedShape)) {
      QRect r(L->p0, L->p1);
      painter.drawRect(r.normalized().adjusted(-5, -5, 5, 5));
    } else if (auto *C = dynamic_cast<CircleShape *>(selectedShape)) {
      painter.translate(ox, oy);
      painter.scale(zoomFactor, zoomFactor);
      painter.drawEllipse(C->center, C->radius, C->radius);
      drawCenterMarker(painter, C->center, 1 /*already in zoom‑space*/);
      painter.resetTransform();
    } else if (auto *P = dynamic_cast<PolygonShape *>(selectedShape)) {
      for (const QPoint &pt : P->vertices)
        painter.drawEllipse(pt, 3, 3);
    }
  }
  drawPreview(painter);
}

/* ---- drawPreview ------------------------------------------------------ */
void DrawingWidget::drawPreview(QPainter &p) {
  if (!isDrawing || currentPoints.isEmpty())
    return;
  QImage tmp = canvas; // local copy
  if ((currentMode == DM_Line || currentMode == DM_ThickLine) &&
      currentPoints.size() >= 2) {
      auto drawThin = antiAliasEnabled ? drawLineWu : drawLineDDA;
      int dx = currentPoints[1].x() - currentPoints[0].x();
      int dy = currentPoints[1].y() - currentPoints[0].y();
      bool horizontalish = std::abs(dx) >= std::abs(dy);
      int h = lineThickness/2;

      for (int off = -h; off <= h; ++off) {
          if (horizontalish)
              drawThin(tmp,
                       currentPoints[0].x(),
                       currentPoints[0].y() + off,
                       currentPoints[1].x(),
                       currentPoints[1].y() + off,
                       drawingColor);
          else
              drawThin(tmp,
                       currentPoints[0].x() + off,
                       currentPoints[0].y(),
                       currentPoints[1].x() + off,
                       currentPoints[1].y(),
                       drawingColor);
      }
  } else if (currentMode == DM_Circle && currentPoints.size() >= 2) {
    int dx = currentPoints[1].x() - currentPoints[0].x();
    int dy = currentPoints[1].y() - currentPoints[0].y();
    int r = int(std::sqrt(dx * dx + dy * dy));
    drawCircleMidpoint(tmp, currentPoints[0].x(), currentPoints[0].y(), r,
                       drawingColor);
  } else if (currentMode == DM_Pen) {
    drawFreehandPen(tmp, currentPoints, drawingColor);
  } else if (currentMode == DM_Pill && currentPoints.size() >= 2) {
      int rad = std::max(1, thicknessSpin->value());
      PillShape tmpShape(currentPoints[0], currentPoints[1],
                         rad, drawingColor, lineThickness, antiAliasEnabled);
      tmpShape.draw(tmp);                     // draw on the temporary image
  }
  if (currentMode == DM_Polygon && currentPoints.size() >= 2) {
    QPainter tp(&tmp);
    tp.setPen(drawingColor);
    for (int i = 0; i + 1 < currentPoints.size(); ++i)
      tp.drawLine(currentPoints[i], currentPoints[i + 1]);
    for (const QPoint &pt : currentPoints)
      tp.drawEllipse(pt, 3, 3);
    if ((currentPoints.last() - currentPoints.first()).manhattanLength() < 15)
      tp.drawLine(currentPoints.last(), currentPoints.first());
  }
  int ox = (width() - canvas.width() * zoomFactor) / 2;
  int oy = toolbarWidget->height();
  p.translate(ox, oy);
  p.scale(zoomFactor, zoomFactor);
  p.drawImage(0, 0, tmp);
}

/* ---- mouse events ----------------------------------------------------- */
void DrawingWidget::mousePressEvent(QMouseEvent *ev) {
  QPoint pos = mapToCanvas(ev->pos());
  if (currentMode == DM_Selection) {
    selectShapeAt(pos);
    lastMousePos = pos;
    update();
    return;
  }
  if (currentMode == DM_Polygon) {
    if (!isDrawing) {
      isDrawing = true;
      currentPoints.clear();
      currentPoints.append(pos);
    }
  } else {
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
    lastMousePos = pos;

    if (auto *L = dynamic_cast<LineShape *>(selectedShape)) {
      if (hit == LineP0)
        L->p0 += delta;
      else if (hit == LineP1)
        L->p1 += delta;
      else if (hit == LineBody) {
        L->p0 += delta;
        L->p1 += delta;
      }
    } else if (auto *C = dynamic_cast<CircleShape *>(selectedShape)) {
      if (hit == CircCenter)
        C->center += delta;
      else if (hit == CircEdge)
        C->radius =
            int(std::hypot(pos.x() - C->center.x(), pos.y() - C->center.y()));
    } else if (auto *P = dynamic_cast<PolygonShape *>(selectedShape)) {
      if (hit == PolyVertex && hitIndex >= 0)
        P->vertices[hitIndex] += delta;
      else if (hit == PolyBody)
        P->moveBy(delta.x(), delta.y());
    } else if (auto *Pill = dynamic_cast<PillShape *>(selectedShape)) {
        if (hit == PillP0)
            Pill->p0 += delta;                    // stretch / shrink
        else if (hit == PillP1)
            Pill->p1 += delta;
        else if (hit == PolyBody)
            Pill->moveBy(delta.x(), delta.y());   // move whole pill
    }
    redrawAllShapes();
    update();
    return;
  }
  if (!isDrawing)
    return;
  if (currentMode == DM_Polygon) {
    currentPoints.last() = pos;
  } else if (currentMode == DM_Pen) {
    currentPoints.append(pos);
  } else {
    if (currentPoints.size() < 2)
      currentPoints.append(pos);
    else
      currentPoints[1] = pos;
  }
  update();
}

void DrawingWidget::mouseReleaseEvent(QMouseEvent *ev) {
  QPoint pos = mapToCanvas(ev->pos());
  if (currentMode == DM_Polygon && isDrawing) {
    if (currentPoints.size() >= 3 &&
        (pos - currentPoints.first()).manhattanLength() < 15) {
      commitCurrentShape();
      isDrawing = false;
    } else
      currentPoints.append(pos);
  } else if (currentMode != DM_Polygon && isDrawing) {
    if (currentPoints.size() < 2)
      currentPoints.append(pos);
    else
      currentPoints[1] = pos;
    if (currentMode == DM_Pen && currentPoints.size() < 2) {
      isDrawing = false;
      return;
    }
    commitCurrentShape();
    isDrawing = false;
  }
  update();
}

void DrawingWidget::mouseDoubleClickEvent(QMouseEvent *ev) {
  if (currentMode == DM_Polygon && isDrawing && currentPoints.size() >= 3) {
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
      redrawAllShapes();
      update();
    }
  }
}

void DrawingWidget::resizeEvent(QResizeEvent *e) {
  QWidget::resizeEvent(e);
  update();
}

/* ---- commitCurrentShape ---------------------------------------------- */
void DrawingWidget::commitCurrentShape() {
  Shape *ns = nullptr;
  if (currentMode == DM_Line || currentMode == DM_ThickLine) {
    if (currentPoints.size() < 2)
      return;
    ns = new LineShape(currentPoints[0], currentPoints[1], drawingColor,
                       lineThickness, antiAliasEnabled);
  } else if (currentMode == DM_Circle) {
    if (currentPoints.size() < 2)
      return;
    int dx = currentPoints[1].x() - currentPoints[0].x();
    int dy = currentPoints[1].y() - currentPoints[0].y();
    int r = int(std::sqrt(dx * dx + dy * dy));
    ns = new CircleShape(currentPoints[0], r, drawingColor, lineThickness,
                         antiAliasEnabled);
  } else if (currentMode == DM_Pen || currentMode == DM_Polygon) {
    if (currentPoints.size() < 2)
      return;
    ns = new PolygonShape(currentPoints, drawingColor, lineThickness,
                          antiAliasEnabled);
  } else if (currentMode == DM_Pill) {
      if (currentPoints.size() < 2) return;
      int rad = std::max(1, thicknessSpin->value());
      ns = new PillShape(currentPoints[0], currentPoints[1],
                         rad, drawingColor, lineThickness, antiAliasEnabled);
  }
  if (!ns)
    return;
  ns->draw(canvas);
  shapes.append(ns);
}

/* ---- selection helpers ----------------------------------------------- */
void DrawingWidget::selectShapeAt(const QPoint &pos) {
  const int T = 7;
  selectedShape = nullptr;
  hit = None;
  hitIndex = -1;

  auto distToSegment = [&](QPoint a, QPoint b) {
    double A = pos.x() - a.x(), B = pos.y() - a.y();
    double C = b.x() - a.x(), D = b.y() - a.y();
    double dot = A * C + B * D, len2 = C * C + D * D;
    double t = (len2 ? dot / len2 : -1);
    t = qBound(0.0, t, 1.0);
    double rx = a.x() + t * C, ry = a.y() + t * D;
    return std::hypot(pos.x() - rx, pos.y() - ry);
  };

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
      if (distToSegment(L->p0, L->p1) < T) {
        selectedShape = L;
        hit = LineBody;
        return;
      }
    } else if (auto *C = dynamic_cast<CircleShape *>(s)) {
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
    } else if (auto *P = dynamic_cast<PolygonShape *>(s)) {
      for (int i = 0; i < P->vertices.size(); ++i) {
        if ((pos - P->vertices[i]).manhattanLength() < T) {
          selectedShape = P;
          hit = PolyVertex;
          hitIndex = i;
          return;
        }
      }
      QRect bb(P->vertices.first(), P->vertices.first());
      for (const QPoint &pt : P->vertices)
        bb |= QRect(pt, pt);
      if (bb.adjusted(-T, -T, T, T).contains(pos)) {
        selectedShape = P;
        hit = PolyBody;
        return;
      }
    } else if (auto *Pill = dynamic_cast<PillShape *>(s)) {
        if ((pos - Pill->p0).manhattanLength() < T) {
            selectedShape = Pill;  hit = PillP0;  return;
        }
        if ((pos - Pill->p1).manhattanLength() < T) {
            selectedShape = Pill;  hit = PillP1;  return;
        }
        double d = distToSegment(Pill->p0, Pill->p1);
        if (d < T) {
            selectedShape = Pill;  hit = PolyBody;  return;
        }
    }
  }
}
