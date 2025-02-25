#ifndef FUNCTIONEDITORCANVAS_H
#define FUNCTIONEDITORCANVAS_H

#include <QWidget>
#include <QVector>
#include <QPoint>

/**
 * @brief The FunctionEditorCanvas class
 * A 256x256 widget that allows adding/moving/deleting control points
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
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    // Guaranteed sorted by X
    QVector<QPoint> m_points;

    int m_dragIndex;

    int findPointNearby(const QPoint &pos, int radius = 6) const;
    void sortPointsByX();

    // Constrain a point so it doesn't violate the valid function constraints
    //  - leftmost must be x=0, rightmost x=255
    //  - x is monotonic
    //  - y in [0..255]
    void constrainPoint(int index);
};

#endif // FUNCTIONEDITORCANVAS_H
