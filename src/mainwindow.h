#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ConvolutionEditorWidget.h"
#include "DitheringAndQuantizationWidget.h"
#include "FunctionalEditorDock.h"
#include "ditheringandquantization.h"
#include <QImage>
#include <QMainWindow>
#include <QTabWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
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

signals:
  void imageLoaded();

private slots:
  // System actions
  void on_btnLoad_clicked();  ///< Loads an image from disk.
  void on_btnSave_clicked();  ///< Saves the processed image to disk.
  void on_btnReset_clicked(); ///< Resets to the original image.
  void on_btnGray_clicked();  ///< Converts image to grayscale

  // Filter actions
  void on_btnInvert_clicked();             ///< Applies inversion filter.
  void on_btnGenerateInvert_clicked();     ///< Generate invert functional.
  void on_btnBrightness_clicked();         ///< Apply brightness correction.
  void on_btnGenerateBrightness_clicked(); ///< Generate brightness functional.
  void on_btnContrast_clicked();           ///< Apply contrast correction.
  void on_btnGenerateContrast_clicked();   ///< Generate contrast functional.
  void on_btnGamma_clicked();              ///< Apply gamma correction.
  void on_btnBlur_clicked();               ///< Applies a blur filter.
  void on_btnGauss_clicked();              ///< Applies a Gaussian blur filter.
  void on_btnSharpen_clicked();            ///< Applies a sharpening filter.
  void on_btnEdge_clicked();     ///< Applies an edge detection filter.
  void on_btnEmboss_clicked();   ///< Applies an emboss filter.
  void on_btnMedian_clicked();   ///< Applies median filter.
  void on_btnErosion_clicked();  ///< Applies morhpological erosion.
  void on_btnDilation_clicked(); ///< Applies morphological dilation.

  /**
   * @brief Applies a custom function to the image using a lookup table (LUT).
   * @param lut The lookup table used for transformation.
   */
  void onDockFunctionApplied(const QVector<int> &lut);

  void onApplyConvolutionFilter();

  void onApplyOrderedDithering(int thresholdMapSize, int levelsPerChannel);
  void onApplyOrderedDitheringYCbCr(int thresholdMapSize, int levelsPerChannel);
  void onApplyPopularityQuantization(int numColors);

private:
  Ui::MainWindow *ui;   ///< The UI instance.
  QImage originalImage; ///< Stores the original loaded image.
  QImage filteredImage; ///< Stores the processed image.

  QTabWidget *filterEditorTabs;
  FunctionalEditorDock *functionalEditor;
  ConvolutionEditorWidget *convolutionEditor;

  /**
   * @brief Displays images in the appropriate UI labels.
   */
  void displayImages();
};

#endif // MAINWINDOW_H
