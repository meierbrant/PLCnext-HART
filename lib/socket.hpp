#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

using std::cout;
using std::endl;

class Socket {
    private:
    std::string target;
    std::string ipstr;
    std::string port;
    struct addrinfo *servinfo;
    int sockfd;

    public:
    Socket(std::string target);
    Socket(std::string target, std::string port);
    int connect();
    int bind();
    int send(const uint8_t *msg, size_t len, int flags);
    int recv(uint8_t *buf, size_t len, int flags);
    int close();
};