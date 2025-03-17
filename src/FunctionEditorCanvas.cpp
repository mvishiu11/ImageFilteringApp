#include "FunctionEditorCanvas.h"
#include <QMouseEvent>
#include <QPainter>
#include <QtMath>
#include <algorithm>

FunctionEditorCanvas::FunctionEditorCanvas(QWidget *parent)
    : QWidget(parent), m_dragIndex(-1) {
  // Set a fixed size of 256x256 pixels.
  setFixedSize(256, 256);
  // Initialize with the identity function: f(x)= x (0 maps to 0, 255 maps to
  // 255).
  m_points.clear();
  m_points.append(QPoint(0, 0));
  m_points.append(QPoint(255, 255));
}

QVector<int> FunctionEditorCanvas::buildLookupTable() const {
  QVector<int> lut(256);
  // Assume m_points is sorted by x.
  for (int x = 0; x < 256; ++x) {
    if (x <= m_points.first().x()) {
      lut[x] = m_points.first().y();
      continue;
    }
    if (x >= m_points.last().x()) {
      lut[x] = m_points.last().y();
      continue;
    }
    for (int i = 0; i < m_points.size() - 1; ++i) {
      QPoint p1 = m_points[i];
      QPoint p2 = m_points[i + 1];
      if (x >= p1.x() && x <= p2.x()) {
        double t = double(x - p1.x()) / double(p2.x() - p1.x());
        int y = p1.y() + int(t * (p2.y() - p1.y()));
        lut[x] = y;
        break;
      }
    }
  }
  for (int &val : lut) {
    val = qBound(0, val, 255);
  }
  return lut;
}

void FunctionEditorCanvas::resetPoints() {
  m_points.clear();
  m_points.append(QPoint(0, 0));
  m_points.append(QPoint(255, 255));
  update();
}

void FunctionEditorCanvas::setCurveForBrightness(int delta, int samplePoints) {
  QVector<QPoint> pts;
  if (samplePoints < 2)
    samplePoints = 2;
  // f(x)= clamp(x + delta, 0, 255)
  for (int i = 0; i < samplePoints; ++i) {
    int x = i * 255 / (samplePoints - 1);
    int y = qBound(0, x + delta, 255);
    pts.append(QPoint(x, y));
  }
  m_points = pts;
  update();
}

void FunctionEditorCanvas::setCurveForContrast(double factor,
                                               int samplePoints) {
  QVector<QPoint> pts;
  if (samplePoints < 2)
    samplePoints = 2;
  // f(x)= clamp(128 + (x-128)*factor, 0, 255)
  for (int i = 0; i < samplePoints; ++i) {
    int x = i * 255 / (samplePoints - 1);
    int y = qBound(0, 128 + int((x - 128) * factor), 255);
    pts.append(QPoint(x, y));
  }
  m_points = pts;
  update();
}

void FunctionEditorCanvas::setCurveForInvert() {
  m_points.clear();
  m_points.append(QPoint(0, 255));
  m_points.append(QPoint(255, 0));
  update();
}

void FunctionEditorCanvas::paintEvent(QPaintEvent * /*event*/) {
  QPainter painter(this);
  // Fill background.
  painter.fillRect(rect(), Qt::white);

  // Draw grid lines.
  painter.setPen(QColor(220, 220, 220));
  for (int i = 0; i <= 256; i += 64) {
    painter.drawLine(i, 0, i, height());
    painter.drawLine(0, i, width(), i);
  }

  // Draw the polyline connecting control points.
  painter.setPen(Qt::black);
  for (int i = 0; i < m_points.size() - 1; ++i) {
    QPoint p1(m_points[i].x(), height() - m_points[i].y());
    QPoint p2(m_points[i + 1].x(), height() - m_points[i + 1].y());
    painter.drawLine(p1, p2);
  }

  // Draw control points as red circles.
  painter.setBrush(Qt::red);
  for (const QPoint &pt : m_points) {
    QPoint widgetPt(pt.x(), height() - pt.y());
    painter.drawEllipse(widgetPt, 4, 4);
  }
}

void FunctionEditorCanvas::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    // Convert the widget coordinate to function space.
    QPoint funcPos(event->pos().x(), height() - event->pos().y());
    int idx = findPointNearby(funcPos);
    if (idx >= 0) {
      m_dragIndex = idx;
    } else if (event->modifiers() & Qt::ShiftModifier) {
      m_points.append(funcPos);
      sortPointsByX();
    }
  } else if (event->button() == Qt::RightButton) {
    QPoint funcPos(event->pos().x(), height() - event->pos().y());
    int idx = findPointNearby(funcPos);
    if (idx > 0 && idx < m_points.size() - 1) {
      m_points.remove(idx);
    }
  }
  update();
}

void FunctionEditorCanvas::mouseMoveEvent(QMouseEvent *event) {
  if (m_dragIndex >= 0) {
    QPoint funcPos(event->pos().x(), height() - event->pos().y());
    m_points[m_dragIndex] = funcPos;
    constrainPoint(m_dragIndex);
    sortPointsByX();
    update();
  }
}

void FunctionEditorCanvas::mouseReleaseEvent(QMouseEvent *event) {
  Q_UNUSED(event);
  m_dragIndex = -1;
}

int FunctionEditorCanvas::findPointNearby(const QPoint &funcPos,
                                          int radius) const {
  for (int i = 0; i < m_points.size(); ++i) {
    if ((m_points[i] - funcPos).manhattanLength() <= radius)
      return i;
  }
  return -1;
}

void FunctionEditorCanvas::sortPointsByX() {
  std::sort(m_points.begin(), m_points.end(),
            [](const QPoint &a, const QPoint &b) { return a.x() < b.x(); });
}

void FunctionEditorCanvas::constrainPoint(int index) {
  if (index < 0 || index >= m_points.size())
    return;
  QPoint &pt = m_points[index];
  int x = qBound(0, pt.x(), 255);
  int y = qBound(0, pt.y(), 255);
  // Fix first and last points.
  if (index == 0)
    x = 0;
  if (index == m_points.size() - 1)
    x = 255;
  pt.setX(x);
  pt.setY(y);
}
