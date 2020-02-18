#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

using std::string;
using std::cout;
using std::endl;

class UdpSocket {
    private:
    int sockfd;
    unsigned short port;

    public:
    UdpSocket(int port=80);
    int sendto(string ip, const char *msg, size_t len, int flags=0);
    int recvfrom(string &ip, char *buf, size_t len, int flags=0);
    int close();
};