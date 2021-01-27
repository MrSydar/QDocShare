#include <iostream>
#include <clientwindow.h>
#include <ui_clientwindow.h>
#include <document.h>

using namespace std;

ClientWindow::ClientWindow(QWidget *parent) : QDialog(parent), ui(new Ui::ClientWindow){
    ui->setupUi(this);
    hasDocument = false;
    ui->textEdit->setReadOnly(true);
    QObject::connect(ui->textEdit, &NetTextEditor::gotChange, this, &ClientWindow::catchKeyboardChange);
}

ClientWindow::~ClientWindow(){
    emit gotDisconnect();
    delete ui;
}

void ClientWindow::setDocument(QString doc){
    this->document = doc.toStdString();
    ui->textEdit->setText(doc);
    hasDocument = true;
    ui->textEdit->setReadOnly(false);
}

void ClientWindow::setChange(QString change){
    if(!hasDocument){
        emit needDoc();
        return;
    }
    string ch = change.toStdString();
    pair<int,string> delta = getChange(ch);
    changeLocalDocument(delta, document);
    int pos = ui->textEdit->textCursor().position() - (delta.second == "\b" ? 1 : 0);
    ui->textEdit->setText(document.c_str());
    for(int i=0; i <= pos; ++i){
        ui->textEdit->moveCursor(QTextCursor::NextCharacter,QTextCursor::MoveAnchor);
    }
}

void ClientWindow::setQuit(){
    this->close();
}

void ClientWindow::catchKeyboardChange(QString change)
{
    if(hasDocument){
        emit gotChange(change);
    }
}

void ClientWindow::onNewNickname(QString nick){
    ui->clientsList->addItem(nick);
}

void ClientWindow::onNicknameList(QStringList nicknames){
    ui->clientsList->clear();
    ui->clientsList->addItems(nicknames);
}

void ClientWindow::on_pushButton_clicked(){
    emit gotDisconnect();
    this->close();
}

void ClientWindow::on_pushButton_2_clicked()
{
    emit needDoc();
}

void ClientWindow::on_ClientWindow_finished(int result)
{
    cout << "Client window closed with code: " << result << endl;
    this->close();
    emit gotDisconnect();
}
