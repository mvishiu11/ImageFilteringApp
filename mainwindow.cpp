#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "filters.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QColor>
#include <QResizeEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
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
    displayImages();
}

// Invert Filter
void MainWindow::on_btnInvert_clicked() {
    if (filteredImage.isNull()) {
        QMessageBox::warning(this, tr("Warning"), tr("No image to filter."));
        return;
    }
    filteredImage = Filters::invert(filteredImage);
    displayImages();
}

void MainWindow::on_btnBrightness_clicked() {
    if (filteredImage.isNull()) {
        QMessageBox::warning(this, tr("Warning"), tr("No image to filter."));
        return;
    }

    filteredImage = Filters::adjustBrightness(filteredImage, 30);
    displayImages();
}

void MainWindow::on_btnContrast_clicked() {
    if (filteredImage.isNull()) {
        QMessageBox::warning(this, tr("Warning"), tr("No image to filter."));
        return;
    }

    filteredImage = Filters::adjustContrast(filteredImage, 1.2);
    displayImages();
}

void MainWindow::on_btnGamma_clicked() {
    if (filteredImage.isNull()) {
        QMessageBox::warning(this, tr("Warning"), tr("No image to filter."));
        return;
    }

    filteredImage = Filters::adjustGamma(filteredImage, 1.5);
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

// Helper: Display images in labels
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
