#ifndef FUNCTIONEDITORCANVAS_H
#define FUNCTIONEDITORCANVAS_H

#include <QPoint>
#include <QVector>
#include <QWidget>

/**
 * @brief The FunctionEditorCanvas class
 *
 * A custom widget that provides a 256x256 editable area for defining a
 * piecewise-linear function. The function maps input values [0,255] to
 * output values [0,255] (with 0 at the bottom and 255 at the top).
 *
 * Users can add, drag, and remove control points by mouse. The widget
 * displays a grid and the polyline defined by the control points.
 */
class FunctionEditorCanvas : public QWidget {
  Q_OBJECT
public:
  /**
   * @brief Constructs a FunctionEditorCanvas.
   * @param parent Parent widget (default is nullptr).
   */
  explicit FunctionEditorCanvas(QWidget *parent = nullptr);

  /**
   * @brief Builds a lookup table (LUT) from the current control points.
   * @return A QVector of 256 integers representing the function.
   */
  QVector<int> buildLookupTable() const;

  /**
   * @brief Resets the control points to the identity function.
   */
  void resetPoints();

  /**
   * @brief Sets the control points for a brightness transformation.
   * f(x)= clamp(x + delta, 0, 255).
   * @param delta The brightness offset.
   * @param samplePoints The number of sample points to generate.
   */
  void setCurveForBrightness(int delta, int samplePoints = 3);

  /**
   * @brief Sets the control points for a contrast transformation.
   * f(x)= clamp(128 + (x-128)*factor, 0, 255).
   * @param factor The contrast factor.
   * @param samplePoints The number of sample points to generate.
   */
  void setCurveForContrast(double factor, int samplePoints = 3);

  /**
   * @brief Sets the control points for an invert transformation.
   * f(x)= 255 - x.
   */
  void setCurveForInvert();

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private:
  QVector<QPoint>
      m_points;    ///< Control points in function space (x:0-255, y:0-255).
  int m_dragIndex; ///< Index of the currently dragged point (-1 if none).

  /**
   * @brief Finds a control point near the given function-space position.
   * @param funcPos The position in function space.
   * @param radius The maximum distance to consider (default 6).
   * @return The index of the nearby control point, or -1 if none.
   */
  int findPointNearby(const QPoint &funcPos, int radius = 6) const;

  /// Sorts the control points in increasing order of x.
  void sortPointsByX();

  /**
   * @brief Constrains a control point so that it stays within [0,255].
   * Also fixes the first point to x=0 and the last to x=255.
   * @param index The index of the control point to constrain.
   */
  void constrainPoint(int index);
};

#endif // FUNCTIONEDITORCANVAS_H
