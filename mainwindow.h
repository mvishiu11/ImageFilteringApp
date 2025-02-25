#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // System buttons
    void on_btnLoad_clicked();
    void on_btnSave_clicked();
    void on_btnReset_clicked();

    // Filter buttons
    void on_btnInvert_clicked();
    void on_btnBrightness_clicked();
    void on_btnContrast_clicked();
    void on_btnGamma_clicked();
    void on_btnBlur_clicked();
    void on_btnGauss_clicked();
    void on_btnSharpen_clicked();
    void on_btnEdge_clicked();
    void on_btnEmboss_clicked();

private:
    Ui::MainWindow *ui;
    QImage originalImage;
    QImage filteredImage;

    // Helper to display images
    void displayImages();
};

#endif // MAINWINDOW_H
