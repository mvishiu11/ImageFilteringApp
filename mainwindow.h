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
    void on_btnLoad_clicked();
    void on_btnInvert_clicked();

private:
    Ui::MainWindow *ui;
    QImage originalImage;
    QImage filteredImage;

    QImage invertImage(const QImage &image);
};

#endif // MAINWINDOW_H
