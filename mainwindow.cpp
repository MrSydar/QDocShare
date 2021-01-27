#include <clientwindow.h>
#include <errordialog.h>
#include <iostream>
#include <mainwindow.h>
#include <ui_mainwindow.h>

using namespace std;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);
    isServerRunning = false;
}

MainWindow::~MainWindow(){
    delete ui;
    cr.~ClientRunner();
    sr.~ServerRunner();
}

void MainWindow::on_connectButton_clicked(){
    string address = ui->ipConnectTextEdit->toPlainText().toStdString();
    int port = ui->portConnectTextEdit->toPlainText().toInt();
    if(cr.runClient(address, port) < 0){
        ErrorDialog ed(this, "Can't connect");
        ed.setModal(true);
        ed.exec();
        return;
    }

    ClientWindow cw;
    QObject::connect(&cw, &ClientWindow::gotChange, &cr, &ClientRunner::sendMessage);
    QObject::connect(&cw, &ClientWindow::gotDisconnect, &cr, &ClientRunner::setQuit);
    QObject::connect(&cw, &ClientWindow::needDoc, &cr, &ClientRunner::askForDoc);
    QObject::connect(&cr, &ClientRunner::gotDocument, &cw, &ClientWindow::setDocument);
    QObject::connect(&cr, &ClientRunner::gotChange, &cw, &ClientWindow::setChange);
    QObject::connect(&cr, &ClientRunner::connectionClosed, &cw, &ClientWindow::setQuit);
    QObject::connect(&cr, &ClientRunner::gotNewNickname, &cw, &ClientWindow::onNewNickname);
    QObject::connect(&cr, &ClientRunner::gotClientsNicknames, &cw, &ClientWindow::onNicknameList);

    cw.setModal(true);
    cw.exec();

    cr.stopClient();
}

void MainWindow::on_sessionButton_clicked()
{
    if(isServerRunning){
        sr.closeServer();
        isServerRunning = false;
        ui->sessionButton->setText("START SESSION");
        ui->connectButton->setText("CONNECT");
        setUIActive(true);
    }
    else{
        int status = sr.startServer(ui->portConnectTextEdit->toPlainText().toInt(), ui->numClientsSessionTextEdit->toPlainText().toInt());
        if(status < 0){
            QString errType;

            if(status == -1) errType = "Socket error";
            else if(status == -2) errType = "SetSockOpt error";
            else if(status == -3) errType = "Binding error";
            else errType = "Listen error";

            ErrorDialog er(this, errType);
            er.setModal(true);
            er.exec();
            return;
        }
        isServerRunning = true;
        ui->sessionButton->setText("STOP SESSION");
        ui->connectButton->setText("BLOCKED");
        setUIActive(false);
    }
}

void MainWindow::setUIActive(bool active)
{
    ui->ipConnectTextEdit->setReadOnly(!active);
    ui->portConnectTextEdit->setReadOnly(!active);
    ui->connectButton->setEnabled(active);
    ui->ipSessionTextEdit->setReadOnly(!active);
    ui->numClientsSessionTextEdit->setReadOnly(!active);
}
