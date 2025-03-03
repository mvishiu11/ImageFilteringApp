// FunctionEditorCanvas.h

#ifndef FUNCTIONEDITORCANVAS_H
#define FUNCTIONEDITORCANVAS_H

#include <QWidget>
#include <QVector>
#include <QPoint>

class FunctionEditorCanvas : public QWidget
{
    Q_OBJECT
public:
    explicit FunctionEditorCanvas(QWidget *parent = nullptr);

    QVector<int> buildLookupTable() const;
    void resetPoints();

    void setCurveForBrightness(int delta, int samplePoints = 3);
    void setCurveForContrast(double factor, int samplePoints = 3);
    void setCurveForInvert();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    // Control points
    QVector<QPoint> m_points;

    // Index of the point being dragged (-1 if none)
    int m_dragIndex;

    // Utility methods
    int findPointNearby(const QPoint &funcPos, int radius = 6) const;
    void sortPointsByX();
    void constrainPoint(int index);
};

#endif // FUNCTIONEDITORCANVAS_H
