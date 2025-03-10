#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Load style sheet
    QFile file(":/styles/style.qss");  // The :/ prefix stands for resources
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        a.setStyleSheet(styleSheet);
        file.close();
    } else {
        qDebug() << "Could not load style.qss";
    }
    
    MainWindow w;
    w.show();
    return a.exec();
}
