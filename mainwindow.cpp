#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QColor>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

// Slot: Load Image
void MainWindow::on_btnLoad_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Image"), "",
                                                    tr("Image Files (*.png *.jpg *.bmp)"));
    if (fileName.isEmpty()) return;

    // Load the image
    if (!originalImage.load(fileName)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not load image."));
        return;
    }

    // Display the original image in the label
    ui->labelOriginal->setPixmap(QPixmap::fromImage(originalImage).scaled(
        ui->labelOriginal->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // Clear filtered image display
    ui->labelFiltered->clear();
}

// Slot: Apply Inversion Filter
void MainWindow::on_btnInvert_clicked() {
    if (originalImage.isNull()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please load an image first."));
        return;
    }

    // Apply the inversion filter
    filteredImage = invertImage(originalImage);

    // Display the filtered image in the label
    ui->labelFiltered->setPixmap(QPixmap::fromImage(filteredImage).scaled(
        ui->labelFiltered->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

// Helper function: Invert image colors
QImage MainWindow::invertImage(const QImage &image) {
    QImage result = image.convertToFormat(QImage::Format_RGB32);
    for (int y = 0; y < result.height(); ++y) {
        for (int x = 0; x < result.width(); ++x) {
            QRgb pixel = result.pixel(x, y);
            int red = 255 - qRed(pixel);
            int green = 255 - qGreen(pixel);
            int blue = 255 - qBlue(pixel);
            result.setPixel(x, y, qRgb(red, green, blue));
        }
    }
    return result;
}
