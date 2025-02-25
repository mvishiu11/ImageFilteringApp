#include "FunctionEditorCanvas.h"
#include <QPainter>
#include <QMouseEvent>
#include <QtMath>
#include <algorithm>

FunctionEditorCanvas::FunctionEditorCanvas(QWidget *parent)
    : QWidget(parent),
    m_dragIndex(-1)
{
    setFixedSize(256, 256);

    m_points.append(QPoint(0, 255));
    m_points.append(QPoint(255, 0));
    m_points.clear();
    m_points.append(QPoint(0, 255));
    m_points.append(QPoint(255, 0));
}

QVector<int> FunctionEditorCanvas::buildLookupTable() const
{
    QVector<int> lut(256);

    for (int x = 0; x < 256; ++x) {
        // If x is exactly at or below the first point
        if (x <= m_points[0].x()) {
            lut[x] = m_points[0].y();
            continue;
        }
        // If x is exactly at or above the last point
        if (x >= m_points.last().x()) {
            lut[x] = m_points.last().y();
            continue;
        }

        // Otherwise, find the segment in m_points that covers x
        for (int i = 0; i < m_points.size() - 1; ++i) {
            QPoint p1 = m_points[i];
            QPoint p2 = m_points[i+1];
            if (x >= p1.x() && x <= p2.x()) {
                double t = double(x - p1.x()) / double(p2.x() - p1.x());
                int y = p1.y() + int(t * (p2.y() - p1.y()));
                lut[x] = y;
                break;
            }
        }
    }

    // Bound each LUT entry to [0..255]
    for (int &val : lut) {
        if (val < 0) val = 0;
        if (val > 255) val = 255;
    }

    return lut;
}

void FunctionEditorCanvas::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);

    // Draw a grid
    painter.setPen(QColor(220,220,220));
    for (int i = 0; i <= 256; i += 64) {
        painter.drawLine(i, 0, i, 256);
        painter.drawLine(0, i, 256, i);
    }

    // Draw the polyline
    painter.setPen(Qt::black);
    for (int i = 0; i < m_points.size() - 1; ++i) {
        painter.drawLine(m_points[i], m_points[i+1]);
    }

    // Draw points
    painter.setBrush(Qt::red);
    for (int i = 0; i < m_points.size(); ++i) {
        QPoint pt = m_points[i];
        painter.drawEllipse(pt, 4, 4);
    }
}

void FunctionEditorCanvas::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int idx = findPointNearby(event->pos());
        if (idx >= 0) {
            m_dragIndex = idx;  // Start dragging
        } else {
            if (event->modifiers() & Qt::ShiftModifier) {
                m_points.append(event->pos());
                sortPointsByX();
            }
        }
    }
    else if (event->button() == Qt::RightButton) {
        int idx = findPointNearby(event->pos());
        if (idx > 0 && idx < m_points.size()-1) {
            m_points.remove(idx);
        }
    }

    update();
}

void FunctionEditorCanvas::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragIndex >= 0) {
        // Move the point
        QPoint newPos = event->pos();
        m_points[m_dragIndex] = newPos;
        constrainPoint(m_dragIndex);
        sortPointsByX();

        update();
    }
}

void FunctionEditorCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_dragIndex = -1;
}

int FunctionEditorCanvas::findPointNearby(const QPoint &pos, int radius) const
{
    for (int i = 0; i < m_points.size(); ++i) {
        QPoint diff = m_points[i] - pos;
        if (diff.manhattanLength() <= radius) {
            return i;
        }
    }
    return -1;
}

void FunctionEditorCanvas::sortPointsByX()
{
    // Sort by x, if tie, sort by y to keep stable
    std::sort(m_points.begin(), m_points.end(), [](const QPoint &a, const QPoint &b){
        if (a.x() == b.x()) return a.y() < b.y();
        return a.x() < b.x();
    });
}

void FunctionEditorCanvas::constrainPoint(int index)
{
    if (index < 0 || index >= m_points.size()) return;

    // clamp Y to [0..255], clamp X to [0..255]
    // but also leftmost must be x=0, rightmost must be x=255
    QPoint &pt = m_points[index];
    int x = pt.x();
    int y = pt.y();

    // clamp to range
    x = std::max(0, std::min(x, 255));
    y = std::max(0, std::min(y, 255));

    if (index == 0) {
        // leftmost point
        x = 0; // force x=0
    }
    if (index == m_points.size() - 1) {
        // rightmost point
        x = 255; // force x=255
    }
    pt.setX(x);
    pt.setY(y);
}
