#include "FunctionEditorCanvas.h"
#include <QPainter>
#include <QMouseEvent>
#include <QtMath>
#include <algorithm>

FunctionEditorCanvas::FunctionEditorCanvas(QWidget *parent)
    : QWidget(parent), m_dragIndex(-1)
{
    setFixedSize(256, 256);

    // Default function: Inverted line from (0,255) to (255,0)
    m_points.append(QPoint(0, 255));
    m_points.append(QPoint(255, 0));
}

QVector<int> FunctionEditorCanvas::buildLookupTable() const
{
    QVector<int> lut(256);

    for (int x = 0; x < 256; ++x) {
        if (x <= m_points[0].x()) {
            lut[x] = m_points[0].y();
            continue;
        }
        if (x >= m_points.last().x()) {
            lut[x] = m_points.last().y();
            continue;
        }

        // Linear interpolation
        for (int i = 0; i < m_points.size() - 1; ++i) {
            QPoint p1 = m_points[i];
            QPoint p2 = m_points[i + 1];
            if (x >= p1.x() && x <= p2.x()) {
                double t = double(x - p1.x()) / double(p2.x() - p1.x());
                lut[x] = p1.y() + int(t * (p2.y() - p1.y()));
                break;
            }
        }
    }

    // Clamp values to [0, 255]
    for (int &val : lut) {
        val = std::clamp(val, 0, 255);
    }

    return lut;
}

void FunctionEditorCanvas::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);

    // Draw grid
    painter.setPen(QColor(220, 220, 220));
    for (int i = 0; i <= 256; i += 64) {
        painter.drawLine(i, 0, i, 256);
        painter.drawLine(0, i, 256, i);
    }

    // Draw function curve
    painter.setPen(Qt::black);
    for (int i = 0; i < m_points.size() - 1; ++i) {
        painter.drawLine(m_points[i], m_points[i + 1]);
    }

    // Draw control points
    painter.setBrush(Qt::red);
    for (const auto &pt : m_points) {
        painter.drawEllipse(pt, 4, 4);
    }
}

void FunctionEditorCanvas::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int idx = findPointNearby(event->pos());
        if (idx >= 0) {
            m_dragIndex = idx;  // Start dragging
        } else if (event->modifiers() & Qt::ShiftModifier) {
            m_points.append(event->pos());
            sortPointsByX();
        }
    } else if (event->button() == Qt::RightButton) {
        int idx = findPointNearby(event->pos());
        if (idx > 0 && idx < m_points.size() - 1) {
            m_points.remove(idx);  // Remove point (except endpoints)
        }
    }
    update();
}

void FunctionEditorCanvas::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragIndex >= 0) {
        m_points[m_dragIndex] = event->pos();
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
        if ((m_points[i] - pos).manhattanLength() <= radius) {
            return i;
        }
    }
    return -1;
}

void FunctionEditorCanvas::sortPointsByX()
{
    std::sort(m_points.begin(), m_points.end(), [](const QPoint &a, const QPoint &b) {
        return a.x() == b.x() ? a.y() < b.y() : a.x() < b.x();
    });
}

void FunctionEditorCanvas::constrainPoint(int index)
{
    if (index < 0 || index >= m_points.size()) return;

    QPoint &pt = m_points[index];
    pt.setX(std::clamp(pt.x(), 0, 255));
    pt.setY(std::clamp(pt.y(), 0, 255));

    if (index == 0) pt.setX(0);                     // First point fixed at x=0
    if (index == m_points.size() - 1) pt.setX(255); // Last point fixed at x=255
}
