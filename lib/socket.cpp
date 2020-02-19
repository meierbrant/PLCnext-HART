#include "socket.hpp"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

//#define DEBUG

/**
 * designed based on this network programming guide:
 * https://beej.us/guide/bgnet/pdf/bgnet_usl_c_1.pdf
 */

Socket::Socket(string target) {
    #ifdef DEBUG
    std::cout << "new Socket(" << target << ":80)" << std::endl;
    #endif
    Socket::target = target;
    Socket::port = "80";
}

Socket::Socket(string target, string port) {
    #ifdef DEBUG
    std::cout << "new Socket(" << target << ":" << port << ")" << std::endl;
    #endif
    Socket::target = target;
    Socket::port = port;
}

int Socket::connect() {
    struct addrinfo hints, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo(target.c_str(), port.c_str(), &hints, &servinfo);
    
    // check each returned result until one connects sucessfully
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (::connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
            continue;

        break;
    }
    servinfo = p;
    if (servinfo == NULL) {
        fprintf(stderr, "failed to connect to socket\n");
        exit(2);
    }

    char ipStr[INET_ADDRSTRLEN];
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)servinfo->ai_addr;
    inet_ntop(AF_INET, &(ipv4->sin_addr), ipStr, INET_ADDRSTRLEN);
    string ipstr(ipStr);
    #ifdef DEBUG
    std::cout << "\t" << ipstr << "\tport=" << ipv4->sin_port << "\tprotocol=" << servinfo->ai_protocol << std::endl;
    #endif

    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    #ifdef DEBUG
    std::cout << "\tsockdf=" << sockfd << "\tai_addr=" << servinfo->ai_addr << std::endl;
    #endif
    int r = ::connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
    if (r < 0) {
        perror("Error connecting socket");
    }
    return r;
}

int Socket::bind(string port) {
    this->port = port;
    struct addrinfo hints, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, port.c_str(), &hints, &servinfo);
    
    // check each returned result until one connects sucessfully
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (::bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
            continue;

        break;
    }
    servinfo = p;
    if (servinfo == NULL) {
        fprintf(stderr, "failed to connect to socket\n");
        exit(2);
    }

    char ipStr[INET_ADDRSTRLEN];
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)servinfo->ai_addr;
    inet_ntop(AF_INET, &(ipv4->sin_addr), ipStr, INET_ADDRSTRLEN);
    string ipstr(ipStr);
    #ifdef DEBUG
    std::cout << "\t" << ipstr << "\tport=" << ipv4->sin_port << "\tprotocol=" << servinfo->ai_protocol << std::endl;
    #endif

    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    #ifdef DEBUG
    std::cout << "\tsockdf=" << sockfd << "\tai_addr=" << servinfo->ai_addr << std::endl;
    #endif

    if (::listen(sockfd, 5) == -1) { // connection queue size limit = 5
        perror("failed to listen");
    }
    return 0;
}

// TODO: support sending void* messages
int Socket::send(const uint8_t *msg, size_t len, int flags) {
    int bytes_sent = 0;
    while (bytes_sent < len) {
        int result = ::send(sockfd, &msg[bytes_sent], len-bytes_sent, flags);
        if (result < 0) return result;
        bytes_sent += result;
        //cout << bytes_sent << "/" << len << " sent..." << endl;
    }
    return bytes_sent;
}

int Socket::recv(uint8_t *buf, size_t len, int flags) {
    int bytes_recvd = 0;
    while (bytes_recvd < len) {
        int result = ::recv(sockfd, &buf[bytes_recvd], len-bytes_recvd, flags);
        if (result <= 0) return result;
        bytes_recvd += result;
    }
    return bytes_recvd;
}

int Socket::close() {
    freeaddrinfo(servinfo);
    return ::close(sockfd);
}