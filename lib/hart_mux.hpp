#pragma once

#include "data_types.hpp"
#include "socket.hpp"
#include "hart_device.hpp"
#include <iostream>

class HartMux : public HartDevice {
    public:
    bool err;
    std::string errMsg;
    uint32_t inactivityTimeout;
    Socket sock;

    HartMux(std::string ip) : sock(ip, "5094"), ipAddress(ip), inactivityTimeout(60000) {};
    int initSession();
    int closeSession();
    uint8_t* getUniqueAddr(); // cmd 0
    void readIOSystemCapabilities(); // cmd 74
    int sendCmd(unsigned char cmd, uint8_t pollAddr);
    int sendCmd(unsigned char cmd, uint8_t *uniqueAddr);

private:
    HartDevice devices[32];

    /* HART General */
    hart_ip_pkt_t request;
    hart_ip_pkt_t response;
    
    int send(uint8_t *bytes, size_t len, int flags);
    int recv(uint8_t *buf, size_t len, int flags);
    int sendHeader();
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