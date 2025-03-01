#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include "FunctionalEditorDock.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

protected:
    /**
     * @brief Handles window resize events to update image display.
     * @param event The resize event.
     */
    void resizeEvent(QResizeEvent *event) override;

private slots:
    // System actions
    void on_btnLoad_clicked();  ///< Loads an image from disk.
    void on_btnSave_clicked();  ///< Saves the processed image to disk.
    void on_btnReset_clicked(); ///< Resets to the original image.

    // Filter actions
    void on_btnInvert_clicked();                      ///< Applies inversion filter.
    void on_sliderBrightness_valueChanged(int value); ///< Adjusts brightness.
    void on_sliderContrast_valueChanged(int value);   ///< Adjusts contrast.
    void on_sliderGamma_valueChanged(int value);      ///< Adjusts gamma.
    void on_btnBlur_clicked();                        ///< Applies a blur filter.
    void on_btnGauss_clicked();                       ///< Applies a Gaussian blur filter.
    void on_btnSharpen_clicked();                     ///< Applies a sharpening filter.
    void on_btnEdge_clicked();                        ///< Applies an edge detection filter.
    void on_btnEmboss_clicked();                      ///< Applies an emboss filter.

    /**
     * @brief Applies a custom function to the image using a lookup table (LUT).
     * @param lut The lookup table used for transformation.
     */
    void onDockFunctionApplied(const QVector<int> &lut);

private:
    Ui::MainWindow *ui;   ///< The UI instance.
    QImage originalImage; ///< Stores the original loaded image.
    QImage filteredImage; ///< Stores the processed image.

    FunctionalEditorDock *functionalDock; ///< Dock for function-based transformations.

    /**
     * @brief Displays images in the appropriate UI labels.
     */
    void displayImages();
};

#endif // MAINWINDOW_H
