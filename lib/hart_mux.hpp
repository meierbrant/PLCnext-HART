#pragma once

#include "data_types.hpp"
#include "socket.hpp"
#include "hart_device.hpp"
#include <iostream>

using std::string;

class HartMux : public HartDevice {
    public:
    bool err;
    string errMsg;
    uint32_t inactivityTimeout;
    Socket sock;
    hart_io_capabilities ioCapabilities;

    HartMux(string ip) : sock(ip, "5094"), ipAddress(ip), inactivityTimeout(60000) {};
    int initSession();
    int closeSession();
    uint8_t* getUniqueAddr(); // cmd 0
    void readIOSystemCapabilities(); // cmd 74
    HartDevice readSubDeviceSummary(uint16_t index); // cmd 84
    int sendCmd(unsigned char cmd, uint8_t pollAddr);
    int sendCmd(unsigned char cmd, uint8_t *uniqueAddr, uint8_t *reqData=NULL, size_t reqDataCnt=0);

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
    string ipAddress;
    unsigned int ipPort = 5094;
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
    int dataOut = 0;
    int arrDataOut[256];

};