#include "udp_socket.hpp"

#include <string.h>     /* for memset() */
#include <functional>   /* for std::function */


UdpSocket::UdpSocket(int port) {
    if ((sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("failed to create socket");
    }
    int broadcastPermission = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission, 
        sizeof(broadcastPermission)) < 0) {
        perror("failed to enable broadcast");
    }
    UdpSocket::port = port;
}

int UdpSocket::sendto(string ip, const char *msg, size_t len, int flags) {
    struct sockaddr_in destAddr;

    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    destAddr.sin_port = htons(port);

    if (::sendto(sockfd, msg, len, flags, (struct sockaddr *) 
        &destAddr, sizeof(destAddr)) != len)
        perror("sendto() sent a different number of bytes than expected");
}


int UdpSocket::recvfrom(string &ip, char *buf, size_t len, int flags) {
    int n;
    socklen_t l;
    struct sockaddr_in srcAddr;
    
    memset(&srcAddr, 0, sizeof(srcAddr));
    srcAddr.sin_family = AF_INET;
    srcAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    srcAddr.sin_port = htons(port);

    memset(buf, 0, len);
    n = ::recvfrom(sockfd, buf, len, flags, (struct sockaddr *)&srcAddr, &l);
    buf[n] = '\0';

    ip = string(inet_ntoa(srcAddr.sin_addr));

    return n;
}

int UdpSocket::recvfromMultiple(string &ip, char *buf, size_t len, std::function<void(string srcIP, char *buf, size_t len)> handler, int flags) {
    int r = 0;
    int delay = 1000000; // microseconds
    bool run = true;
    int responses = 0;
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = delay;

    while(run) {
        fd_set rfds;

        FD_ZERO(&rfds);
        FD_SET(sockfd, &rfds);
        int recVal = select(sockfd + 1, &rfds, NULL, NULL, &tv);
        switch (recVal) {
            case 0: //Timeout
                run = false;
                break;
            case -1: //Error
                run = false;
                break;
            default:
                int n = recvfrom(ip, buf, len);
                if(n < 0) { //Failed to Recieve Data
                    run = false;
                    r = -1;
                }
                else { //Recieved Data!!
                    handler(ip, buf, n);
                    tv.tv_sec = 0;
                    tv.tv_usec = (int)(delay / (2 ^ responses++));
                }
                break;
        }
    }
    return r;
}

int UdpSocket::close() {
    ::close(sockfd);
}