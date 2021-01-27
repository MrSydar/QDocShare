#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <clientrunner.h>
#include <serverrunner.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_connectButton_clicked();
    void on_sessionButton_clicked();

private:
    ServerRunner sr;
    ClientRunner cr;
    bool isServerRunning;
    Ui::MainWindow *ui;

    void setUIActive(bool active);
};

#endif // MAINWINDOW_H
