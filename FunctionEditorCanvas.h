#ifndef FUNCTIONEDITORCANVAS_H
#define FUNCTIONEDITORCANVAS_H

#include <QWidget>
#include <QVector>
#include <QPoint>

/**
 * @brief The FunctionEditorCanvas class
 * A 256x256 Qwidget-based canvas that allows adding/moving/deleting control points
 * that define a piecewise linear function from x=0..255 => y=0..255.
 */
class FunctionEditorCanvas : public QWidget
{
    Q_OBJECT
public:
    explicit FunctionEditorCanvas(QWidget *parent = nullptr);

    /**
     * @brief buildLookupTable
     * Constructs a 256-element LUT based on the current polyline of control points.
     * @return QVector<int> of size 256, mapping input [0..255] to output [0..255].
     */
    QVector<int> buildLookupTable() const;

protected:
    /**
     * @brief Handles the painting event to draw the function graph and grid.
     */
    void paintEvent(QPaintEvent *event) override;


    /**
     * @brief Handles mouse press events to select, add, or remove points.
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief Handles mouse movement events to drag control points.
     */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * @brief Handles mouse release events, stopping dragging.
     */
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    // Guaranteed sorted by X
    QVector<QPoint> m_points;

    int m_dragIndex;

    /**
     * @brief Finds a control point near a given position.
     * @param pos The position to check.
     * @param radius The maximum distance to consider.
     * @return Index of the nearby point, or -1 if none found.
     */
    int findPointNearby(const QPoint &pos, int radius = 6) const;

    /**
     * @brief Sorts control points by x-coordinate while maintaining y-order for stability.
     */
    void sortPointsByX();

    // Constrain a point so it doesn't violate the valid function constraints

    /**
 * @brief Constrains a point's position within valid bounds.
 *   - leftmost must be x=0, rightmost x=255
 *   - x is monotonic
 *   - y in [0..255]
 * @param index The index of the point to constrain.
 */
    void constrainPoint(int index);
};

#endif // FUNCTIONEDITORCANVAS_H
