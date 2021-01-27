#ifndef SERVERRUNNER_H
#define SERVERRUNNER_H

#include <QObject>
#include <QFuture>
#include <string>
#include <quitInfo.h>

class ServerRunner : public QObject
{
    Q_OBJECT

public:
    explicit ServerRunner(QObject *parent = nullptr);
    ~ServerRunner();

    int startServer(int port, int maxClients);
    void closeServer();

private:
    int run();
    QFuture<int> runner;
    int openServer(int port, int maxClients);
    void closeConnections();
    quitCode qCode;

    int serverfd, maxClients, *clientfds;
    std::string document = "Hello world!\n";

signals:

};

#endif // SERVERRUNNER_H
