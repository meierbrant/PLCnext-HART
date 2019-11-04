#include "nettools.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <iostream>

// int nettools::ping(std::string ipv4) {
//     return ping((const char *)&ipv4);
//}

// TODO: fix ping. It always returns 0;
int nettools::ping(const char *ipv4) {
    int s = socket(PF_INET, SOCK_RAW, 1);

    if(s <= 0) {
        perror("Socket Error");
        exit(0);
    }

    struct icmp_hdr_t {
        uint8_t type;
        uint8_t code;
        uint16_t chksum;
        uint32_t data;
    };

    icmp_hdr_t pckt;
    pckt.type = 8;          // The echo request is 8
    pckt.code = 0;          // No need
    pckt.chksum = 0xfff7;   // Fixed checksum since the data is not changing
    pckt.data = 0;          // We don't send anything.

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = inet_addr("8.8.8.8");

    int actionSendResult = sendto(s, &pckt, sizeof(pckt), 0, (struct sockaddr*)&addr, sizeof(addr));

    if(actionSendResult < 0) {
        perror("Ping Error");
        exit(0);
    }

    unsigned int resAddressSize;
    unsigned char res[30] = "";
    struct sockaddr resAddress;

    struct icmp_response_t {
        uint8_t type;
        uint8_t code;
        uint16_t checksum;
        uint16_t identifier;
        uint16_t sequence_number;
    };

    int r = recvfrom(s, res, sizeof(res), 0, &resAddress, &resAddressSize);

    icmp_response_t* echo_response;

    echo_response = (icmp_response_t *)&res[20];

    printf(
        "type: %x, code: %x, checksum: %x, identifier: %x, sequence: %x\n",
        echo_response->type,
        echo_response->code,
        echo_response->checksum,
        echo_response->identifier,
        echo_response->sequence_number
    );

    std::cout << echo_response << std::endl;

    if (1) {
    
    } else {
        perror("Response Error");
        return -1;
    }

    return 0;
    
}