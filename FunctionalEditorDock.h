#ifndef FUNCTIONALEDITORDOCK_H
#define FUNCTIONALEDITORDOCK_H

#include <QDockWidget>
#include <QVector>

/**
 * @brief The FunctionalEditorDock class
 *
 * This dock widget hosts a FunctionEditorCanvas (a 256x256 editable area)
 * along with "Apply" and "Reset" buttons. It is used to create and edit a
 * piecewise-linear lookup table (LUT) representing a functional filter.
 *
 * The widget provides methods to initialize the curve for brightness,
 * contrast, or inversion. When the user clicks "Apply", the generated LUT
 * is emitted via the functionApplied signal.
 */
class FunctionalEditorDock : public QDockWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a FunctionalEditorDock widget.
     * @param parent Parent widget (default is nullptr).
     */
    explicit FunctionalEditorDock(QWidget *parent = nullptr);

    /**
     * @brief Initializes the editor with a brightness curve.
     * @param delta The brightness offset (added to x).
     * @param samplePoints The number of sample points to generate.
     */
    void setInitialBrightnessCurve(int delta, int samplePoints = 3);

    /**
     * @brief Initializes the editor with a contrast curve.
     * @param factor The contrast factor (applied to (x-128)).
     * @param samplePoints The number of sample points to generate.
     */
    void setInitialContrastCurve(double factor, int samplePoints = 3);

    /**
     * @brief Initializes the editor with an inversion curve (f(x)=255-x).
     */
    void setInitialInvertCurve();

signals:
    /**
     * @brief functionApplied
     * Emitted when the user clicks "Apply", passing a 256-element LUT.
     * Each element maps an input value (0..255) to an output value (0..255).
     */
    void functionApplied(const QVector<int> &lut);

private slots:
    void onApplyClicked();
    void onResetClicked();

private:
    /**
     * @brief generateLUT
     * Uses the canvas's control points to build a 256-element lookup table.
     * @return The generated LUT.
     */
    QVector<int> generateLUT() const;

    class FunctionEditorCanvas *m_canvas; ///< The custom drawing canvas.
};

#endif // FUNCTIONALEDITORDOCK_H
