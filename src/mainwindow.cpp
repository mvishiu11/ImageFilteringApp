#include "mainwindow.h"
#include "ditheringandquantization.h"
#include "filters.h"
#include "ui_mainwindow.h"

#include <QColor>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QToolBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  // Create a stacked widget to switch between filter mode and drawing mode.
  modeStack = new QStackedWidget(this);

  // Create filtering page: Wrap existing filtering UI.
  // Assume ui->centralWidget contains your original filter UI (image labels,
  // sliders, etc.)
  filteringPage = ui->centralwidget;
  modeStack->addWidget(filteringPage);

  // Create the drawing page.
  drawingPage = new DrawingWidget(this);
  modeStack->addWidget(drawingPage);

  cubePage = new CubeWidget(this);
  modeStack->addWidget(cubePage);

  cylinderPage = new CylinderWidget(this);
  modeStack->addWidget(cylinderPage);

  // Set the stacked widget as the new central widget.
  setCentralWidget(modeStack);

  // Create a toolbar with mode-switching actions.
  QToolBar *modeToolBar = addToolBar(tr("Mode"));
  QAction *filterModeAction = modeToolBar->addAction(tr("Filter Mode"));
  QAction *drawModeAction = modeToolBar->addAction(tr("Draw Mode"));
  QAction *cubeModeAction = modeToolBar->addAction(tr("3D Cube"));
  QAction *cylinderModeAction = modeToolBar->addAction(tr("3D Textured Cylinder"));

  connect(filterModeAction, &QAction::triggered, this,
          &MainWindow::switchToFilterMode);
  connect(drawModeAction, &QAction::triggered, this,
          &MainWindow::switchToDrawMode);
  connect(cubeModeAction, &QAction::triggered, this,
          &MainWindow::switchToCubeMode);
  connect(cylinderModeAction, &QAction::triggered, this,
          &MainWindow::switchToCylinderMode);


  // Re-add your docks for filtering as before:

  filterEditorTabs = new QTabWidget(this);
  // Create the functional editor
  functionalEditor = new FunctionalEditorDock(this);
  // Create the convolution editor
  convolutionEditor = new ConvolutionEditorWidget(this);

  functionalEditor->setFeatures(functionalEditor->features() &
                                ~QDockWidget::DockWidgetFloatable &
                                ~QDockWidget::DockWidgetClosable);
  convolutionEditor->setFeatures(convolutionEditor->features() &
                                 ~QDockWidget::DockWidgetFloatable &
                                 ~QDockWidget::DockWidgetClosable);

  filterEditorTabs->addTab(functionalEditor, tr("Functional Editor"));
  filterEditorTabs->addTab(convolutionEditor, tr("Convolution Editor"));

  QDockWidget *filterDock = new QDockWidget(tr("Filter Editor"), this);
  filterDock->setWidget(filterEditorTabs);
  addDockWidget(Qt::RightDockWidgetArea, filterDock);

  dqWidget = new DitheringQuantizationWidget(this);
  addDockWidget(Qt::RightDockWidgetArea, dqWidget);
  connect(dqWidget,
          &DitheringQuantizationWidget::applyOrderedDitheringRequested, this,
          &MainWindow::onApplyOrderedDithering);
  connect(dqWidget,
          &DitheringQuantizationWidget::applyOrderedDitheringYCbCrRequested,
          this, &MainWindow::onApplyOrderedDitheringYCbCr);
  connect(dqWidget,
          &DitheringQuantizationWidget::applyPopularityQuantizationRequested,
          this, &MainWindow::onApplyPopularityQuantization);

  connect(functionalEditor, &FunctionalEditorDock::functionApplied, this,
          &MainWindow::onDockFunctionApplied);
  connect(convolutionEditor, &ConvolutionEditorWidget::applyConvolutionFilter,
          this, &MainWindow::onApplyConvolutionFilter);

  // Connect menu actions for file operations.
  auto viewMenu = menuBar()->addMenu("View");
  viewMenu->addAction(filterDock->toggleViewAction());
  viewMenu->addAction(dqWidget->toggleViewAction());

  // texture menu actions
  auto textureMenu = menuBar()->addMenu("Textures");
  QAction *textureLoadAction = textureMenu->addAction(tr("Load Texture"));
  connect(textureLoadAction, &QAction::triggered, this,
          &MainWindow::loadTexture);

  connect(ui->actionLoad_Image, &QAction::triggered, this,
          &MainWindow::on_btnLoad_clicked);
  connect(ui->actionClose, &QAction::triggered, this, &MainWindow::close);
  connect(ui->actionSave_Image, &QAction::triggered, this,
          &MainWindow::on_btnSave_clicked);
  connect(ui->actionReset_Image, &QAction::triggered, this,
          &MainWindow::on_btnReset_clicked);

  connect(this, &MainWindow::imageLoaded, this,
          [this]() { statusBar()->showMessage(tr("Image loaded"), 3000); });
}

void MainWindow::switchToFilterMode() {
  modeStack->setCurrentWidget(filteringPage);
}

void MainWindow::switchToDrawMode() {
  modeStack->setCurrentWidget(drawingPage);
}

void MainWindow::switchToCubeMode() {
    modeStack->setCurrentWidget(cubePage);
}

void MainWindow::switchToCylinderMode() {
    modeStack->setCurrentWidget(cylinderPage);
}

void MainWindow::loadTexture() {
    QString fn = QFileDialog::getOpenFileName(this,tr("PNG"),"","PNG (*.png)");
    static_cast<CylinderWidget*>(cylinderPage)->loadTexture(fn);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_btnLoad_clicked() {
  QString fileName = QFileDialog::getOpenFileName(
      this, tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp)"));
  if (fileName.isEmpty())
    return;

  // Load the image
  if (!originalImage.load(fileName)) {
    QMessageBox::critical(this, tr("Error"), tr("Could not load image."));
    return;
  }

  filteredImage = originalImage; // Start with same as original
  displayImages();
  emit imageLoaded();
}

void MainWindow::on_btnSave_clicked() {
  if (filteredImage.isNull()) {
    QMessageBox::warning(this, tr("Warning"), tr("No filtered image to save."));
    return;
  }
  QString fileName = QFileDialog::getSaveFileName(
      this, tr("Save Image"), "",
      tr("PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)"));
  if (fileName.isEmpty())
    return;

  if (!filteredImage.save(fileName)) {
    QMessageBox::critical(this, tr("Error"), tr("Could not save image."));
  }
}

void MainWindow::on_btnReset_clicked() {
  if (originalImage.isNull()) {
    QMessageBox::warning(this, tr("Warning"), tr("No image loaded."));
    return;
  }
  // Revert to the original
  filteredImage = originalImage;
  ui->sliderBrightness->setSliderPosition(0);
  ui->sliderContrast->setSliderPosition(100);
  ui->sliderGamma->setSliderPosition(100);
  displayImages();
}

void MainWindow::on_btnGray_clicked() {
  if (filteredImage.format() != QImage::Format_Grayscale8) {
    int ret =
        QMessageBox::question(this, tr("Convert to Grayscale?"),
                              tr("The image is not in grayscale. Would you "
                                 "like to convert it to grayscale first?"),
                              QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
      filteredImage = filteredImage.convertToFormat(QImage::Format_Grayscale8);
      displayImages();
    } else {
      return;
    }
  }
}

void MainWindow::onDockFunctionApplied(const QVector<int> &lut) {
  if (filteredImage.isNull()) {
    QMessageBox::warning(this, "Warning", "No image to apply function to.");
    return;
  }

  QImage result = filteredImage.convertToFormat(QImage::Format_RGB32);
  for (int y = 0; y < result.height(); ++y) {
    for (int x = 0; x < result.width(); ++x) {
      QRgb pixel = result.pixel(x, y);
      int r = lut[qRed(pixel)];
      int g = lut[qGreen(pixel)];
      int b = lut[qBlue(pixel)];
      result.setPixel(x, y, qRgb(r, g, b));
    }
  }
  filteredImage = result;
  displayImages();
}

void MainWindow::onApplyConvolutionFilter() {
  ConvolutionEditorWidget *convEditor = convolutionEditor;
  QVector<QVector<int>> kernel = convEditor->getKernel();
  int divisor = convEditor->getDivisor();
  int offset = convEditor->getOffset();
  QPair<int, int> anchor = convEditor->getAnchor();

  if (filteredImage.isNull()) {
    QMessageBox::warning(this, tr("Warning"), tr("No image to filter."));
    return;
  }

  filteredImage = Filters::applyConvolution(
      filteredImage, kernel, divisor, offset, anchor.first, anchor.second);
  displayImages();
}

void MainWindow::onApplyOrderedDithering(int thresholdMapSize,
                                         int levelsPerChannel) {
  if (filteredImage.isNull()) {
    QMessageBox::warning(this, tr("Warning"), tr("No image to filter."));
    return;
  }
  filteredImage = DitheringAndQuantization::applyOrderedDithering(
      filteredImage, thresholdMapSize, levelsPerChannel);
  displayImages();
}

void MainWindow::onApplyOrderedDitheringYCbCr(int thresholdMapSize,
                                              int levelsPerChannel) {
  if (filteredImage.isNull()) {
    QMessageBox::warning(this, tr("Warning"), tr("No image to filter."));
    return;
  }
  filteredImage = DitheringAndQuantization::applyOrderedDitheringInYCbCr(
      filteredImage, thresholdMapSize, levelsPerChannel);
  displayImages();
}

void MainWindow::onApplyPopularityQuantization(int numColors) {
  if (filteredImage.isNull()) {
    QMessageBox::warning(this, tr("Warning"), tr("No image to filter."));
    return;
  }
  filteredImage = DitheringAndQuantization::applyPopularityQuantization(
      filteredImage, numColors);
  displayImages();
}

void MainWindow::on_btnInvert_clicked() {
  if (filteredImage.isNull()) {
    QMessageBox::warning(this, tr("Warning"), tr("No image to filter."));
    return;
  }
  filteredImage = Filters::invert(filteredImage);
  displayImages();
}

void MainWindow::on_btnGenerateInvert_clicked() {
  functionalEditor->setInitialInvertCurve();
}

void MainWindow::on_btnBrightness_clicked() {
  if (filteredImage.isNull()) {
    QMessageBox::warning(this, tr("Warning"), tr("No image to filter."));
    return;
  }

  filteredImage =
      Filters::adjustBrightness(filteredImage, ui->sliderBrightness->value());

  displayImages();
}

void MainWindow::on_btnGenerateBrightness_clicked() {
  int delta = ui->sliderBrightness->value();
  functionalEditor->setInitialBrightnessCurve(delta, 6);
}

void MainWindow::on_btnContrast_clicked() {
  if (filteredImage.isNull()) {
    QMessageBox::warning(this, tr("Warning"), tr("No image to filter."));
    return;
  }

  filteredImage = Filters::adjustContrast(filteredImage,
                                          ui->sliderContrast->value() / 100.0);

  displayImages();
}

void MainWindow::on_btnGenerateContrast_clicked() {
  double factor = ui->sliderContrast->value() / 100.0;
  functionalEditor->setInitialContrastCurve(factor, 6);
}

void MainWindow::on_btnGamma_clicked() {
  if (filteredImage.isNull()) {
    QMessageBox::warning(this, tr("Warning"), tr("No image to filter."));
    return;
  }

  filteredImage =
      Filters::adjustGamma(filteredImage, ui->sliderGamma->value() / 100.0);

  displayImages();
}

void MainWindow::on_btnBlur_clicked() {
  if (filteredImage.isNull()) {
    QMessageBox::warning(this, tr("Warning"), tr("No image to filter."));
    return;
  }
  filteredImage = Filters::blur3x3(filteredImage);
  displayImages();
}

void MainWindow::on_btnGauss_clicked() {
  if (filteredImage.isNull()) {
    QMessageBox::warning(this, tr("Warning"), tr("No image to filter."));
    return;
  }
  filteredImage = Filters::gaussianBlur3x3(filteredImage);
  displayImages();
}

void MainWindow::on_btnSharpen_clicked() {
  if (filteredImage.isNull()) {
    QMessageBox::warning(this, tr("Warning"), tr("No image to filter."));
    return;
  }
  filteredImage = Filters::sharpen3x3(filteredImage);
  displayImages();
}

void MainWindow::on_btnEdge_clicked() {
  if (filteredImage.isNull()) {
    QMessageBox::warning(this, tr("Warning"), tr("No image to filter."));
    return;
  }
  filteredImage = Filters::edgeDetect3x3(filteredImage);
  displayImages();
}

void MainWindow::on_btnEmboss_clicked() {
  if (filteredImage.isNull()) {
    QMessageBox::warning(this, tr("Warning"), tr("No image to filter."));
    return;
  }
  filteredImage = Filters::emboss3x3(filteredImage);
  displayImages();
}

void MainWindow::on_btnMedian_clicked() {
  if (filteredImage.isNull()) {
    QMessageBox::warning(this, tr("Warning"), tr("No image to filter."));
    return;
  }

  if (filteredImage.format() != QImage::Format_Grayscale8) {
    int ret =
        QMessageBox::question(this, tr("Convert to Grayscale?"),
                              tr("The image is not in grayscale. Would you "
                                 "like to convert it to grayscale first?"),
                              QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
      filteredImage = filteredImage.convertToFormat(QImage::Format_Grayscale8);
    } else {
      return;
    }
  }

  filteredImage = Filters::applyMedianFilter(filteredImage, 3);
  displayImages();
}

void MainWindow::on_btnErosion_clicked() {
  if (filteredImage.isNull()) {
    QMessageBox::warning(this, tr("Warning"), tr("No image to filter."));
    return;
  }

  filteredImage = Filters::applyErosionFilter(filteredImage, 3);
  displayImages();
}

void MainWindow::on_btnDilation_clicked() {
  if (filteredImage.isNull()) {
    QMessageBox::warning(this, tr("Warning"), tr("No image to filter."));
    return;
  }

  filteredImage = Filters::applyDilationFilter(filteredImage, 3);
  displayImages();
}

void MainWindow::displayImages() {
  if (!originalImage.isNull()) {
    ui->labelOriginal->setPixmap(QPixmap::fromImage(originalImage));
  }
  if (!filteredImage.isNull()) {
    ui->labelFiltered->setPixmap(QPixmap::fromImage(filteredImage));
  }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  QMainWindow::resizeEvent(event);
  displayImages();
}
