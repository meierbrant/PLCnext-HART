#include "udp_socket.hpp"

#include <string.h>     /* for memset() */


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
    char tmpAddr[INET_ADDRSTRLEN];
    
    memset(&srcAddr, 0, sizeof(srcAddr));
    srcAddr.sin_family = AF_INET;
    srcAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    srcAddr.sin_port = htons(port);

    // cout << "\ts_addr = " << srcAddr.sin_addr.s_addr << "\tip = " << ip << endl;
    memset(buf, 0, len);
    n = ::recvfrom(sockfd, buf, len, flags, (struct sockaddr *)&srcAddr, &l);
    buf[n] = '\0';

    ip = string(inet_ntoa(srcAddr.sin_addr));
    // cout << "\ts_addr = " << srcAddr.sin_addr.s_addr << "\tip = " << ip << endl;
    // cout << "response from: " << string(tmpAddr) << endl;

    return n;
}

int UdpSocket::close() {
    ::close(sockfd);
}