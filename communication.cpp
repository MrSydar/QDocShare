#include <communication.h>
#include <unistd.h>
#include <netinet/in.h>
#include <thread>

string genNickName(int id){
    string nickname = "";
    while(id != 0){
        switch (id % 10) {
        case 0:
            nickname += "duck";
            break;
        case 1:
            nickname += "007";
            break;
        case 2:
            nickname += "agent";
            break;
        case 3:
            nickname += "boy";
            break;
        case 4:
            nickname += "girl";
            break;
        case 5:
            nickname += "pink";
            break;
        case 6:
            nickname += "police";
            break;
        case 7:
            nickname += "best";
            break;
        case 8:
            nickname += "vscode";
            break;
        case 9:
            nickname += "docsem";
            break;
        }
        id /= 10;
    }
    return nickname;
}

bool isclosed (int sock) {
    char x;
interrupted:
    ssize_t r = ::recv(sock, &x, 1, MSG_DONTWAIT|MSG_PEEK);
    if (r < 0) {
        switch (errno) {
        case EINTR:     goto interrupted;
        case EAGAIN:    break; /* empty rx queue */
        case ETIMEDOUT: break; /* recv timeout */
        case ENOTCONN:  break; /* not connected yet */
        default:        throw(errno);
        }
    }
    return r == 0;
}

int sendInt(int num, int fd){
    int32_t conv = htonl(num);
    char *data = (char*)&conv;
    int left = sizeof(conv);
    int rc;
    do {
        rc = write(fd, data, left);
        if (rc < 0) {
            perror("sendInt error: ");
            return -1;
        }
        else {
            data += rc;
            left -= rc;
        }
    }
    while (left > 0);
    return 0;
}

int receiveInt(int *num, int fd){
    int32_t ret;
    char *data = (char*)&ret;
    int left = sizeof(ret);
    int rc;
    do {
        rc = read(fd, data, left);
        if (rc < 0) {
            perror("receiveInt error: ");
            return -1;
        }
        else {
            data += rc;
            left -= rc;
        }
    }
    while (left > 0);
    *num = ntohl(ret);
    return 0;
}

string receiveMessage(int sockfd){
    char* buffer;
    string message;
    int n = -1;

    if(receiveInt(&n, sockfd) == -1){
        printf("receiveMessage error\n");
        return "e";
    }

    buffer = new char[n];

    n = read(sockfd,buffer,n);
    if (n < 0) {
        perror("ERROR reading from socket");
        return "e";
    }

    message = string(buffer, n);

    printf("Received %d characters from socket %d, message: \"%s\"\n", n, sockfd, message.c_str());

    return string(buffer, n);
}

int sendMessage(int sockfd, string message){
    int n = -1;

    if(sendInt(message.size(), sockfd) < 0){
        printf("sendMessage error\n");
        return -1;
    };

    n = write(sockfd, message.c_str(), message.size());

    if (n <= 0) {
        perror("sendMessage write error: ");
        return -1;
    }

    printf("Sent %d characters to socket %d, message: \"%s\"\n", n, sockfd, message.c_str());

    return 0;
}
