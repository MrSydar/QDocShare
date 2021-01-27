#include <QString>
#include <clientrunner.h>
#include <communication.h>
#include <document.h>
#include <quitInfo.h>

#include <QtConcurrent>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <poll.h>
#include <sstream>

using namespace std;

ClientRunner::ClientRunner(QObject *parent) : QObject(parent)
{

}
ClientRunner::~ClientRunner(){
    qCode.setQuit();
    runner.waitForFinished();
}

int ClientRunner::receiveManager(int invokefd){
    vector<pollfd> readfds;
    pollfd pfd, invfd;

    pfd.fd = sockfd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    readfds.push_back(pfd);

    invfd.fd = invokefd;
    invfd.events = POLLIN;
    invfd.revents = 0;
    readfds.push_back(invfd);

    while(true){
        int activity = poll(&readfds[0], readfds.size(), -1);
        if (activity < 0){
            perror("Poll error: ");
            qCode.setError();
            return -1;
        }

        if(!qCode.isNoQuit()){
            return 0;
        }

        if(readfds[0].revents & POLLIN){
            string message;

            if(isclosed(sockfd)){
                message = "e";
                qCode.setError();
                return -1;
            }
            else{
                cout << "Incoming message" << endl;
                message = receiveMessage(sockfd);
            }

            if(message[0] == 'c'){
                cout << "Got change message: " << message << endl;
                message = message.substr(2);
                emit gotChange(QString(message.c_str()));
            }
            else if(message[0] == 'l'){
                QStringList nicknames;

                string nick;
                istringstream f(message.substr(2).c_str());
                while (getline(f, nick, ',')) {
                    nicknames.append(nick.c_str());
                }

                emit gotClientsNicknames(nicknames);
            }
            else if(message[0] == 'n'){
                emit gotNewNickname(message.substr(2).c_str());
            }
            else if(message[0] == 'd'){
                cout << "New document: " << message << endl;
                message = message.substr(2);
                emit gotDocument(message.c_str());
            }
            else if(message[0] == 'q' || message[0] == 'e'){
                cout << "Server disconnected this client" << endl;
                close(sockfd);
                qCode.setQuit();
                return 0;
            }
            else{
                cout << "Unidentified message: " << message << endl;
            }
        }

        if(readfds[0].revents != POLLIN && readfds[0].revents & POLLERR){
            cout << "Server error" << endl;
            close(sockfd);
            qCode.setError();
            return -1;
        }
    }

    return 0;
}

void ClientRunner::sendMessage(QString message){
    if(message[0] == 'c'){
        ::sendMessage(sockfd, message.toStdString());
    }
    else {
        cout << "Send manager got unidentified message" << endl;
    }
}

void ClientRunner::setQuit(){
    qCode.setQuit();
}

int ClientRunner::run(){
    QFuture<int> rcvMan;
    int invokeFDS[2];

    if(pipe(invokeFDS) < 0){
        perror("pipe error: ");
        return -1;
    }

    rcvMan = QtConcurrent::run(this, &ClientRunner::receiveManager, invokeFDS[0]);

    qCode.wait();
    cout << "Client disconnected" << endl;

    write(invokeFDS[1], "q", 1);
    cout << "Client receive manager quit with code: " << rcvMan.result() << endl;

    emit connectionClosed();

    close(sockfd);

    return 0;
}

int ClientRunner::runClient(string hostname, int port){
    if(connectToServer(hostname, port) < 0){
        return -1;
    }
    qCode.setNoQuit();
    runner = QtConcurrent::run(this, &ClientRunner::run);

    return 0;
}

void ClientRunner::askForDoc(){
    ::sendMessage(sockfd, "d");
    ::sendMessage(sockfd, "l");
}

int ClientRunner::stopClient(){
    return runner.result();
}

int ClientRunner::connectToServer(string hostname, int port){
    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket setup error: ");
        return -1;
    }

    server = gethostbyname(hostname.c_str());
    if (server == NULL) {
        perror("GetHostByName error: ");
        return -1;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

    serv_addr.sin_port = htons(port);

    if (::connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        perror("Connect error: ");
        return -1;
    }

    return 0;
}
