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
    void resizeEvent(QResizeEvent *event) override;

private slots:
    // System buttons
    void on_btnLoad_clicked();
    void on_btnSave_clicked();
    void on_btnReset_clicked();

    // Filter buttons
    void on_btnInvert_clicked();
    void on_sliderBrightness_valueChanged(int value);
    void on_sliderContrast_valueChanged(int value);
    void on_sliderGamma_valueChanged(int value);
    void on_btnBlur_clicked();
    void on_btnGauss_clicked();
    void on_btnSharpen_clicked();
    void on_btnEdge_clicked();
    void on_btnEmboss_clicked();

    // Functional editor custom function
    void onDockFunctionApplied(const QVector<int> &lut);

private:
    Ui::MainWindow *ui;
    QImage originalImage;
    QImage filteredImage;

    FunctionalEditorDock *functionalDock;

    // Helper to display images
    void displayImages();
};

#endif // MAINWINDOW_H
