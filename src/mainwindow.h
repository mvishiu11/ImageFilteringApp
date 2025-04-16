#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ConvolutionEditorWidget.h"
#include "DitheringAndQuantizationWidget.h"
#include "FunctionalEditorDock.h"
#include "drawingwidget.h"
#include <QImage>
#include <QMainWindow>
#include <QStackedWidget>
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
  void resizeEvent(QResizeEvent *event) override;

signals:
  void imageLoaded();

private slots:
  // System actions
  void on_btnLoad_clicked();
  void on_btnSave_clicked();
  void on_btnReset_clicked();
  void on_btnGray_clicked();

  // Filter actions
  void on_btnInvert_clicked();
  void on_btnGenerateInvert_clicked();
  void on_btnBrightness_clicked();
  void on_btnGenerateBrightness_clicked();
  void on_btnContrast_clicked();
  void on_btnGenerateContrast_clicked();
  void on_btnGamma_clicked();
  void on_btnBlur_clicked();
  void on_btnGauss_clicked();
  void on_btnSharpen_clicked();
  void on_btnEdge_clicked();
  void on_btnEmboss_clicked();
  void on_btnMedian_clicked();
  void on_btnErosion_clicked();
  void on_btnDilation_clicked();

  void onDockFunctionApplied(const QVector<int> &lut);
  void onApplyConvolutionFilter();
  void onApplyOrderedDithering(int thresholdMapSize, int levelsPerChannel);
  void onApplyOrderedDitheringYCbCr(int thresholdMapSize, int levelsPerChannel);
  void onApplyPopularityQuantization(int numColors);

  // Mode switching actions
  void switchToFilterMode();
  void switchToDrawMode();

private:
  Ui::MainWindow *ui;
  QImage originalImage;
  QImage filteredImage;

  // Existing filtering tools:
  QTabWidget *filterEditorTabs;
  FunctionalEditorDock *functionalEditor;
  ConvolutionEditorWidget *convolutionEditor;
  DitheringQuantizationWidget *dqWidget;

  // New drawing widget and mode switching:
  QStackedWidget *modeStack;  // Central widget that switches modes.
  QWidget *filteringPage;     // Container for filtering UI.
  DrawingWidget *drawingPage; // The drawing widget.

  void displayImages();
};

#endif // MAINWINDOW_H
