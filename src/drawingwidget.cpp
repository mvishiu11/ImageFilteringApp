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

QList<RectangleShape *> gClipRects;

namespace {
static constexpr const char *kClipEnabled =
    "QPushButton{ background:#37b24d; color:white; font-weight:bold; }";

static constexpr const char *kClipDisabled =
    "QPushButton{ background:#adb5bd; color:#eeeeee; }";
} // namespace

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
  modeSelector->addItem("Rectangle", DM_Rectangle);
  modeSelector->addItem("Fill", DM_Fill);

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

  /* clipping --------------------------------------------------------- */

  QPushButton *fillBtn = new QPushButton("Fill-Colour");
  QPushButton *imgBtn = new QPushButton("Load Fill Img");
  clipBtn = new QPushButton("Clip");
  tl->addWidget(fillBtn);
  tl->addWidget(imgBtn);
  tl->addWidget(clipBtn);

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
  connect(fillBtn, &QPushButton::clicked, this, [&] {
    if (!selectedShape)
      return;
    QColor c = QColorDialog::getColor(Qt::yellow, this);
    if (!c.isValid())
      return;
    if (auto *P = dynamic_cast<PolygonShape *>(selectedShape)) {
      P->fill = c;
      P->hasImage = false;
      redrawAllShapes();
      update();
    } else if (auto *R = dynamic_cast<RectangleShape *>(selectedShape)) {
      R->fill = c;
      R->hasImage = false;
      redrawAllShapes();
      update();
    }
  });

  connect(imgBtn, &QPushButton::clicked, this, [&] {
    if (selectedShape) {
      QString fn =
          QFileDialog::getOpenFileName(this, "Image", "", "*.png *.jpg");
      if (fn.isEmpty())
        return;
      QImage img(fn);
      if (img.isNull())
        return;
      if (auto *P = dynamic_cast<PolygonShape *>(selectedShape)) {
        P->sample = img;
        P->hasImage = true;
        P->imagePath = fn;
      } else if (auto *R = dynamic_cast<RectangleShape *>(selectedShape)) {
        R->sample = img;
        R->hasImage = true;
        R->imagePath = fn;
      }
      redrawAllShapes();
      update();
    } else {
      /* no shape selected  → choose global seed-fill pattern */
      QString fn = QFileDialog::getOpenFileName(this, "Seed-fill pattern", "",
                                                "*.png *.jpg *.bmp");
      if (fn.isEmpty())
        return;

      QImage pat(fn);
      if (pat.isNull())
        return;
      seedPattern = pat;
    }
  });

  connect(clipBtn, &QPushButton::clicked, this, [&] {
    gClipRects.clear();
    for (Shape *s : shapes)
      if (auto *R = dynamic_cast<RectangleShape *>(s))
        gClipRects.append(R);
    redrawAllShapes();
    update();
  });

  setMouseTracking(true);
  updateClipButton();
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
    seedPattern = QImage();
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
  updateClipButton();
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
    }
    if (dynamic_cast<PillShape *>(s)) {
      out << quint8(4);
      s->write(out);
    }
    if (dynamic_cast<RectangleShape *>(s)) {
      out << quint8(5);
      s->write(out);
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
    if (t == 5)
      s = new RectangleShape;
    if (!s) {
      f.close();
      return;
    }
    s->read(in);
    shapes.append(s);
    s->draw(canvas);
  }
  update();
  updateClipButton();
}

void DrawingWidget::deleteSelectedShape() {
  if (!selectedShape)
    return;
  shapes.removeOne(selectedShape);
  delete selectedShape;
  selectedShape = nullptr;
  redrawAllShapes();
  update();
  updateClipButton();
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

void DrawingWidget::updateClipButton() {
  if (!clipBtn)
    return;
  int nWin = 0, nPoly = 0;
  for (Shape *s : this->shapes) {
    if (dynamic_cast<RectangleShape *>(s))
      ++nWin;
    else if (dynamic_cast<PolygonShape *>(s))
      ++nPoly;
  }
  bool ok = (nWin && nPoly);
  clipBtn->setEnabled(ok);
  clipBtn->setStyleSheet(ok ? kClipEnabled : kClipDisabled);
  clipBtn->setText(ok ? "Clip" : "No window / polygon");
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
    } else if (auto *R = dynamic_cast<RectangleShape *>(selectedShape)) {
      painter.drawRect(QRect(R->p1, R->p2).normalized());
      painter.drawEllipse(R->p1, 3, 3);
      painter.drawEllipse(R->p2, 3, 3);
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
    int h = lineThickness / 2;

    for (int off = -h; off <= h; ++off) {
      if (horizontalish)
        drawThin(tmp, currentPoints[0].x(), currentPoints[0].y() + off,
                 currentPoints[1].x(), currentPoints[1].y() + off,
                 drawingColor);
      else
        drawThin(tmp, currentPoints[0].x() + off, currentPoints[0].y(),
                 currentPoints[1].x() + off, currentPoints[1].y(),
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
    PillShape tmpShape(currentPoints[0], currentPoints[1], rad, drawingColor,
                       lineThickness, antiAliasEnabled);
    tmpShape.draw(tmp); // draw on the temporary image
  } else if (currentMode == DM_Rectangle && currentPoints.size() >= 2) {
    RectangleShape tmpShape(currentPoints[0], currentPoints[1], drawingColor,
                            antiAliasEnabled);
    tmpShape.draw(tmp);
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
  if (currentMode == DM_Fill) {
    const QPoint c = mapToCanvas(ev->pos());

    if (seedPattern.isNull())
      fillSeedScanline(canvas, c.x(), c.y(), &drawingColor, nullptr);
    else
      fillSeedScanline(canvas, c.x(), c.y(), nullptr, &seedPattern);

    for (Shape *s : shapes)
      s->draw(canvas);
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
  const int T = 7;
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
        Pill->p0 += delta;
      else if (hit == PillP1)
        Pill->p1 += delta;
      else if (hit == PolyBody)
        Pill->moveBy(delta.x(), delta.y());
    } else if (auto *R = dynamic_cast<RectangleShape *>(selectedShape)) {
      if (hit == RectP1)
        R->p1 += delta;
      else if (hit == RectP2)
        R->p2 += delta;
      else if (hit == RectEdge) {
        QRect box(R->p1, R->p2);
        box = box.normalized();
        bool grabTop = qAbs(lastMousePos.y() - box.top()) < T;
        bool grabBot = qAbs(lastMousePos.y() - box.bottom()) < T;
        bool grabLeft = qAbs(lastMousePos.x() - box.left()) < T;
        bool grabRight = qAbs(lastMousePos.x() - box.right()) < T;

        if (grabTop || grabBot) {
          R->p1.ry() += delta.y();
        }
        if (grabLeft || grabRight) {
          R->p1.rx() += delta.x();
        }
      } else if (hit == RectBody)
        R->moveBy(delta.x(), delta.y());
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
      updateClipButton();
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
    if (currentPoints.size() < 2)
      return;
    int rad = std::max(1, thicknessSpin->value());
    ns = new PillShape(currentPoints[0], currentPoints[1], rad, drawingColor,
                       lineThickness, antiAliasEnabled);
  } else if (currentMode == DM_Rectangle) {
    if (currentPoints.size() < 2)
      return;
    ns = new RectangleShape(currentPoints[0], currentPoints[1], drawingColor,
                            antiAliasEnabled);
  }
  if (!ns)
    return;
  ns->draw(canvas);
  shapes.append(ns);
  updateClipButton();
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
        selectedShape = Pill;
        hit = PillP0;
        return;
      }
      if ((pos - Pill->p1).manhattanLength() < T) {
        selectedShape = Pill;
        hit = PillP1;
        return;
      }
      double d = distToSegment(Pill->p0, Pill->p1);
      if (d < T) {
        selectedShape = Pill;
        hit = PolyBody;
        return;
      }
    } else if (auto *R = dynamic_cast<RectangleShape *>(s)) {
      QPoint tl = R->p1, br = R->p2; // not guaranteed ordering
      QRect box(tl, br);
      box = box.normalized();
      QPoint tr(box.right(), box.top());
      QPoint bl(box.left(), box.bottom());

      /* 2.1 corners (handles) ------------------------------------ */
      if ((pos - box.topLeft()).manhattanLength() < T) {
        selectedShape = R;
        hit = RectP1;
        return;
      }
      if ((pos - box.bottomRight()).manhattanLength() < T) {
        selectedShape = R;
        hit = RectP2;
        return;
      }
      /* 2.2 edges ------------------------------------------------- */
      auto dSeg = [&](QPoint a, QPoint b) { return distToSegment(a, b); };
      if (dSeg(box.topLeft(), tr) < T || dSeg(tr, box.bottomRight()) < T ||
          dSeg(box.bottomRight(), bl) < T || dSeg(bl, box.topLeft()) < T) {
        selectedShape = R;
        hit = RectEdge;
        return;
      }
      /* 2.3 interior --------------------------------------------- */
      if (box.adjusted(-T, -T, T, T).contains(pos)) {
        selectedShape = R;
        hit = RectBody;
        return;
      }
    }
  }
}
