#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "filters.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QColor>
#include <QDebug>
#include <QResizeEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Create and add the functional editor dock
    functionalDock = new FunctionalEditorDock(this);
    addDockWidget(Qt::RightDockWidgetArea, functionalDock);

    // Connect the signal that sends us the LUT
    connect(functionalDock, &FunctionalEditorDock::functionApplied,
            this, &MainWindow::onDockFunctionApplied);

    // Add a menu item to show/hide the dock
    auto viewMenu = menuBar()->addMenu("View");
    viewMenu->addAction(functionalDock->toggleViewAction());
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_btnLoad_clicked() {
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Open Image"), "",
        tr("Image Files (*.png *.jpg *.bmp)"));
    if (fileName.isEmpty()) return;

    // Load the image
    if (!originalImage.load(fileName)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not load image."));
        return;
    }

    filteredImage = originalImage; // Start with same as original
    displayImages();
}

void MainWindow::on_btnSave_clicked() {
    if (filteredImage.isNull()) {
        QMessageBox::warning(this, tr("Warning"), tr("No filtered image to save."));
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Save Image"), "",
        tr("PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)"));
    if (fileName.isEmpty()) return;

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

void MainWindow::onDockFunctionApplied(const QVector<int> &lut)
{
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

void MainWindow::on_btnInvert_clicked() {
    if (filteredImage.isNull()) {
        QMessageBox::warning(this, tr("Warning"), tr("No image to filter."));
        return;
    }
    filteredImage = Filters::invert(filteredImage);
    displayImages();
}

void MainWindow::on_sliderBrightness_valueChanged(int value)
{
    if (originalImage.isNull()) {
        return;
    }

    filteredImage = Filters::adjustBrightness(originalImage, value);

    displayImages();
}

void MainWindow::on_sliderContrast_valueChanged(int value)
{
    if (originalImage.isNull()) {
        return;
    }

    filteredImage = Filters::adjustContrast(originalImage, value / 100.0);

    displayImages();
}

void MainWindow::on_sliderGamma_valueChanged(int value)
{
    if (originalImage.isNull()) {
        return;
    }

    filteredImage = Filters::adjustGamma(originalImage, value / 100.0);

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

// Helper to display images in labels
void MainWindow::displayImages() {
    ui->labelOriginal->setPixmap(QPixmap::fromImage(originalImage)
                                     .scaled(ui->labelOriginal->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    ui->labelFiltered->setPixmap(QPixmap::fromImage(filteredImage)
                                     .scaled(ui->labelFiltered->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    displayImages();
}
