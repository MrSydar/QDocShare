#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <string>
using namespace std;

bool isclosed (int sock);

int sendInt(int num, int fd);

int receiveInt(int *num, int fd);

string receiveMessage(int sockfd);

int sendMessage(int sockfd, string message);

string genNickName(int id);

#endif // COMMUNICATION_H
