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

class Socket {
    private:
    string target;
    string ipstr;
    string port;
    struct addrinfo *servinfo;
    int sockfd;

    public:
    Socket() {};
    Socket(string target);
    Socket(string target, string port);
    int connect();
    int bind(string port);
    int send(const uint8_t *msg, size_t len, int flags);
    int recv(uint8_t *buf, size_t len, int flags);
    int close();
};