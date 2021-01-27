#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QDialog>
#include <QTextEdit>
#include <iostream>
#include <nettexteditor.h>

namespace Ui {
class ClientWindow;
}

class ClientWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();
    NetTextEditor *getEditor();


public slots:
    void setDocument(QString document);
    void setChange(QString change);
    void setQuit();
    void catchKeyboardChange(QString change);
    void onNewNickname(QString nick);
    void onNicknameList(QStringList nicknames);

signals:
    void gotDisconnect();
    void needDoc();
    void gotChange(QString change);

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

    void on_ClientWindow_finished(int result);


private:
    std::string document;
    Ui::ClientWindow *ui;
    bool hasDocument = false;
};

#endif // CLIENTWINDOW_H
