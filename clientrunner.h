#ifndef CLIENTRUNNER_H
#define CLIENTRUNNER_H

#include <quitInfo.h>
#include <QObject>
#include <qfuture.h>

class ClientRunner : public QObject
{
    Q_OBJECT
public:
    explicit ClientRunner(QObject *parent = nullptr);
    ~ClientRunner();

    int runClient(std::string hostname, int port);
    int stopClient();

private:
    int sockfd;
    quitCode qCode;
    QFuture<int> runner;

    int receiveManager(int invokefd);
    int connectToServer(std::string hostname, int port);
    int run();

signals:
    void gotChange(QString change);
    void gotDocument(QString document);
    void connectionClosed();
    void gotClientsNicknames(QStringList nicknames);
    void gotNewNickname(QString nickname);

public slots:
    void askForDoc();
    void sendMessage(QString change);
    void setQuit();
};

#endif // CLIENTRUNNER_H
