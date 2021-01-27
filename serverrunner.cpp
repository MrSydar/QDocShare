#include <QtConcurrent>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <mutex>
#include <csignal>
#include <semaphore.h>
#include <queue>
#include <algorithm>
#include <map>
#include <unistd.h>
#include <iostream>

#include <quitInfo.h>
#include <communication.h>
#include <document.h>
#include <serverrunner.h>

using namespace std;

void emitNicknamesList(int *clientsfds, int maxClients, map<int, string> nicknames){
    string message = "l ";
    map<int, string>::iterator it = nicknames.begin();
    while(it != nicknames.end()){
        message += it->second + ',';
        ++it;
    }
    message.pop_back();

    for(int i=0; i < maxClients; ++i){
        if(clientsfds[i] != 0){
            sendMessage(clientsfds[i], message);
        }
    }
}

int disconnectClient(vector<pollfd> *readfds, map<int, string> &nicknames, int *clientsfds, int sd, int maxClients){
    for(int i=0; i < (int) readfds->size(); ++i){
        if(readfds->at(i).fd == sd){
            readfds->erase(readfds->begin() + i);
            nicknames.erase(sd);
            for(int i=0; i < maxClients; ++i){
                if(clientsfds[i] == sd){
                    clientsfds[i] = 0;

                    close(sd);

                    emitNicknamesList(clientsfds, maxClients, nicknames);

                    cout << "Disconnected succesfully socket: " << sd << endl;

                    return 0;
                }
            }
        }
    }

    cout << "Disconnect failed (already disconnected or not found). Socket: " << sd << endl;
    return -1;
};

int outputManager(vector<pollfd> &readfds, map<int, string> &nicknames, int *clientfds, int maxClients, mutex &cLock, mutex &chLock, sem_t &sem, string &document, queue<string> &chQueue, quitCode &qCode){
    pair<int, string> delta;
    string pureDelta;

    while (true){
        chLock.lock();

        if(!qCode.isNoQuit()){
            return 0;
        }

        if(chQueue.size() == 0){
            chLock.unlock();
            sem_wait(&sem);
        }
        else {
            delta = getChange(chQueue.front().substr(2));
            pureDelta = chQueue.front();
            chQueue.pop();
            chLock.unlock();
            cLock.lock();

            for(int i=0; i < maxClients; ++i){
                if(clientfds[i] != 0){
                    if(sendMessage(clientfds[i], pureDelta) == -1){
                        printf("Cant send message to %d socket, disconnected by server\n", clientfds[i]);
                        disconnectClient(&readfds, nicknames, clientfds, clientfds[i], maxClients);
                    }
                }
            }

            changeLocalDocument(delta, ref(document));
            printf("New local document state: \n%s\n", document.c_str());

            cLock.unlock();
        }
    }
}

int inputManager(int invokefd, map<int, string> &nicknames, vector<pollfd> &readfds, int serverfd, int *clientfds, int maxClients, mutex &cLock, mutex &chLock, string &document, queue <string> &chQueue, quitCode &qCode, sem_t &invokeOutputSem){
    int connectedClients;
    string message;

    pollfd pfd, invfd;

    pfd.fd = serverfd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    readfds.push_back(pfd);

    invfd.fd = invokefd;
    invfd.events = POLLIN;
    invfd.revents = 0;
    readfds.push_back(invfd);

    connectedClients = 0;
    printf("Server is running! Waiting for connections..\n");

    while(true){
        int activity = poll(&readfds[0], readfds.size(), -1);
        cout << "incoming message" << endl;
        if(!qCode.isNoQuit()){
            return 0;
        }

        if (activity < 0){
            perror("Poll error: ");
            qCode.setError();
            return -1;
        }

        size_t j = 0;
        while (j < readfds.size()) {
            if (readfds[j].revents == 0) {
                ++j;
                continue;
            }

            int sd = readfds[j].fd;

            if (readfds[j].revents & POLLIN) {
                if (sd == serverfd) {
                    sockaddr_in address;
                    socklen_t addrlen = sizeof(address);
                    int new_socket = accept(serverfd, (struct sockaddr *) &address, &addrlen);
                    if (new_socket < 0) {
                        perror("Accept error: ");
                        qCode.setError();
                        return -1;
                    }

                    cLock.lock();
                    if(connectedClients < maxClients){
                        for (int i = 0; i < maxClients; ++i){
                            if(clientfds[i] == 0 ) {
                                string newNickName = genNickName(new_socket);
                                clientfds[i] = new_socket;
                                nicknames.insert(pair<int,string>(new_socket, newNickName));
                                printf(
                                    "New connection established, socket fd is %d, ip is: %s, port: %d, nickname: %s\n",
                                    new_socket,
                                    inet_ntoa(address.sin_addr),
                                    ntohs(address.sin_port),
                                    newNickName.c_str()
                                );
                                for(int j=0; j < maxClients; ++j){
                                    if(clientfds[j] != 0 && i != j){
                                        sendMessage(clientfds[j],"n " + newNickName);
                                    }
                                }
                                break;
                            }
                        }

                        pfd.fd = new_socket;
                        pfd.events = POLLIN | POLLRDHUP;
                        pfd.revents = 0;
                        readfds.push_back(pfd);

                        ++connectedClients;
                    }
                    else {
                        printf(
                            "New connection rejected, socket fd is %d, ip is : %s, port : %d\n",
                            new_socket,
                            inet_ntoa(address.sin_addr),
                            ntohs(address.sin_port)
                        );
                        sendMessage(new_socket, "q");
                        close(new_socket);
                    }
                    cLock.unlock();
                }
                else {
                    if(isclosed(sd)){
                        message = "q";
                    }
                    else{
                        cout << "Incoming message" << endl;
                        message = receiveMessage(sd);
                    }

                    if (message[0] == 'q'){
                        sockaddr_in address;
                        socklen_t addrlen = sizeof(address);

                        if(getpeername(sd, (struct sockaddr*) &address, &addrlen) < 0){
                            perror("GetPeerName error: ");
                            qCode.setError();
                            return -1;
                        };
                        printf("Disconnected socket %d, ip %s, port %d\n", sd, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                        cLock.lock();
                        if(disconnectClient(&readfds, nicknames, clientfds, sd, maxClients) != -1){
                            --connectedClients;
                        };
                        cLock.unlock();
                    }
                    else if(message[0] == 'c'){
                        printf("Got change '%s' from %d socket\n", message.c_str(), sd);

                        chLock.lock();
                        chQueue.push(message);
                        chLock.unlock();
                        sem_post(&invokeOutputSem);
                    }
                    else if(message[0] == 'e'){
                        printf("Client receive message error: disconnecting socket %d\n", sd);

                        cLock.lock();
                        if(disconnectClient(&readfds, nicknames, clientfds, sd, maxClients) != -1){
                            --connectedClients;
                        };
                        cLock.unlock();
                    }
                    else if(message[0] == 'l'){
                        printf("Clients nicknames list request from %d socket\n", sd);
                        cLock.lock();
                        emitNicknamesList(clientfds, maxClients, nicknames);
                        cLock.unlock();
                    }
                    else if(message[0] == 'd'){
                        printf("Document request from %d socket\n", sd);

                        cLock.lock();
                        if(sendMessage(sd, "d " + document) == -1){
                            printf("Document sending error: disconnecting socket %d\n", sd);
                            if(disconnectClient(&readfds, nicknames, clientfds, sd, maxClients) != -1){
                                --connectedClients;
                            };
                        }
                        cLock.unlock();
                    }
                    else{
                        printf("Got unidentified message '%s' from %d socket\n", message.c_str(), sd);
                    }
                }
            }

            if (readfds[j].revents != POLLIN) {
                if (sd == serverfd) {
                    cout << "Server POLLIN" << endl;
                }
                else {
                    if(readfds[j].revents & POLLERR){
                        printf("Socket %d error\n", sd);
                    }

                    cLock.lock();
                    if(disconnectClient(&readfds, nicknames, clientfds, sd, maxClients) != -1){
                        --connectedClients;
                    };
                    cLock.unlock();

                    continue;
                }
            }

            ++j;
        }
    }
}

int ServerRunner::run(){
    QFuture<int> input, output;
    mutex cLock, chLock;
    queue <string> chQueue;
    vector<pollfd> readfds;
    map <int,string> nicknames;
    int invokeFDS[2];

    if(pipe(invokeFDS) < 0){
        perror("pipe error: ");
        return -1;
    }

    sem_t invokeOutputSem;
    if(sem_init(&invokeOutputSem,0,1) < 0){
        perror("sem_init error: ");
        return -1;
    }

    input = QtConcurrent::run(std::bind(&inputManager, invokeFDS[0], ref(nicknames), ref(readfds), serverfd, ref(clientfds), maxClients, ref(cLock), ref(chLock), ref(document), ref(chQueue), ref(qCode), ref(invokeOutputSem)));
    output = QtConcurrent::run(std::bind(&outputManager, ref(readfds), ref(nicknames), ref(clientfds), maxClients, ref(cLock), ref(chLock), ref(invokeOutputSem), ref(document), ref(chQueue), ref(qCode)));

    qCode.wait();
    printf("Server is shutting down..\n");

    write(invokeFDS[1], "q", 1);
    cout << "Input joined with code: " << input.result() << endl;

    chLock.unlock();
    sem_post(&invokeOutputSem);
    cout << "Output joined with code:" << output.result() << endl;

    for(int i=0; i < maxClients; ++i){
        if(clientfds[i] != 0){
            sendMessage(readfds[i].fd, "q");
        }
    }

    closeConnections();

    if(qCode.isError()){
        return -1;
    }
    else{
        return 0;
    }
}

int ServerRunner::openServer(int port, int maxClients){
    int opt;
    struct sockaddr_in servAddr;

    this->maxClients = maxClients;
    clientfds = new int[maxClients];
    memset(clientfds, 0, sizeof(int) * maxClients);

    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd < 0) {
        perror("ERROR opening socket");
        return -1;
    }

    opt = 1;
    if(setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0){
        perror("Setsockopt error: ");
        return -2;
    }

    bzero((char *) &servAddr, sizeof(servAddr));           //clear structure
    servAddr.sin_family = AF_INET;                         //set address family to internet
    servAddr.sin_addr.s_addr = INADDR_ANY;                 //set IP address to this machine address
    servAddr.sin_port = htons(port);                       //convert to web byte order

    if (bind(serverfd, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0){
        perror("Binding error: ");
        return -3;
    }

    if (listen(serverfd, 3) < 0){
        perror("Listen error: ");
        return -4;
    }

    return 0;
}

int ServerRunner::startServer(int port, int maxClients){
    int status = openServer(port, maxClients);
    if(status < 0){
        return status;
    }

    runner = QtConcurrent::run(this,&ServerRunner::run);
    return 0;
}

void ServerRunner::closeServer(){
    qCode.setQuit();
    cout << "Server closed with code: " << runner.result() << endl;
}

void ServerRunner::closeConnections(){
    for(int i=0; i < maxClients; ++i){
        if(clientfds[i] != 0){
            close(clientfds[i]);
        }
    }
    close(serverfd);
}

ServerRunner::ServerRunner(QObject *parent) : QObject(parent){
    signal(SIGPIPE, SIG_IGN);
    document = "The 5th Shrek Film is an upcoming Shrek film that was announced in July 2016 by DreamWorks Animation as what would be the fifth installment in the Shrek franchise and the sequel to Shrek Forever After. The plot is currently unknown and is scheluded to be released on September 2022. ";
}

ServerRunner::~ServerRunner(){
    closeServer();
}
