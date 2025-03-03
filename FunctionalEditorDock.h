#ifndef FUNCTIONALEDITORDOCK_H
#define FUNCTIONALEDITORDOCK_H

#include <QDockWidget>
#include <QVector>

class FunctionEditorCanvas;

/**
 * @brief The FunctionalEditorDock class
 * A dock widget that hosts the FunctionEditorCanvas (the 256x256 area)
 * plus an "Apply" button to emit a signal with the generated LUT.
 */
class FunctionalEditorDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit FunctionalEditorDock(QWidget *parent = nullptr);

    void setInitialBrightnessCurve(int delta, int samplePoints = 3);
    void setInitialContrastCurve(double factor, int samplePoints = 3);
    void setInitialInvertCurve();

signals:
    /**
     * @brief functionApplied
     * Emitted when the user clicks "Apply" in the dock,
     * passing a 256-sized LUT with the mapping [0..255] -> [0..255].
     */
    void functionApplied(const QVector<int> &lut);

private slots:
    void onApplyClicked();
    void onResetClicked();

private:
    /**
     * @brief generateLUT
     * Uses the canvas's control points to create a lookup table of size 256.
     */
    QVector<int> generateLUT() const;

private:
    FunctionEditorCanvas *m_canvas;
};

#endif // FUNCTIONALEDITORDOCK_H
