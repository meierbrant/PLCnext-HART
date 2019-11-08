#pragma once

#include "data_types.hpp"
#include "socket.hpp"
#include <iostream>

class HartMux {
    public:
    bool err;
    std::string errMsg;
    uint32_t inactivityTimeout;
    Socket sock;

    HartMux(std::string ip) : sock(ip, "5094"), ipAddress(ip), inactivityTimeout(60000) {};
    int initSession();
    int getUniqueAddr(); // cmd 0
    int sendCmd(unsigned char cmd);

private:
    unsigned int clientUdtInactivity;

    /* HART General */
    hart_ip_pkt_t request;
    hart_ip_pkt_t response;
    uint8_t rcvPacket[512]; // TODO: might be array of 512 words
    uint8_t addressUniqGW[5]; // utdAddressUnique; don't know what this type is
    uint8_t addressUniqCH[5]; // utdAddressUnique; don't know what this type is

    hart_ip_hdr_t clientInfo;
    hart_ip_hdr_t serverInfo;

    int send(uint8_t *bytes, size_t len, int flags);
    int recv(uint8_t *buf, size_t len, int flags);
    int sendData(uint8_t *data, size_t len);
    int recvData(uint8_t *buf);

    /* Inputs */
    bool activate = false;
    //bool poll = false;
    //long pollTime;
    std::string ipAddress;
    unsigned int IpPort = 5094;
    uint8_t slotNo = 0;
    uint8_t channelNo = 0;
    uint8_t cmd = 0;
    //char *sReqBytes;

    /* Outputs */
    int stepOut;
    bool active = false;
    bool busy = false;
    bool done = false;
    bool error = false;
    uint16_t diag = 0;
    uint16_t addDiag = 0;
    std::string longAddressGW;
    std::string longAddressCH;
    int dataOut = 0;
    int arrDataOut[256];

};