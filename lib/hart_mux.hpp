#pragma once

#include "data_types.hpp"
#include "socket.hpp"
#include "hart_device.hpp"
#include "nlohmann/json.hpp"
#include <iostream>
#include <signal.h>
#include <sys/time.h>
#include <thread>
#include <exception>

using std::string;
using std::thread;
using nlohmann::json;

class CmdDefNotFound : public std::exception {};

class HartMux : public HartDevice {
    public:
    bool err;
    string errMsg;
    uint32_t inactivityTimeout;
    Socket sock;
    hart_io_capabilities ioCapabilities;
    bool stopAutodiscovery;
    HartDevice devices[32];
    int maxLogfileSize; // default is 6MB, which is roughly one line per minute for 1 day

    HartMux(string ip) : sock(ip, "5094"), ipAddress(ip), inactivityTimeout(5000), maxLogfileSize(6000000) {};
    int initSession();
    int closeSession();
    uint8_t* getUniqueAddr(); // cmd 0
    void readIOSystemCapabilities(); // cmd 74
    HartDevice readSubDeviceSummary(uint16_t index); // cmd 84
    void autodiscoverSubDevices();
    void listDevices();
    hart_var_set readSubDeviceVars(HartDevice dev);
    hart_var_set getSubDeviceVars(HartDevice dev);
    void logVars(string dir);
    json getLogData(string dir, int ioCard, int channel);
    string getLongTag();
    void setLongTag(string longTag);

    int sendCmd(unsigned char cmd, uint8_t pollAddr);
    int sendCmd(unsigned char cmd, uint8_t *uniqueAddr, uint8_t *reqData=NULL, size_t reqDataCnt=0);
    int sendCmd(unsigned char cmd, uint8_t *uniqueAddr, uint8_t *reqData, size_t reqDataCnt, uint8_t *resData, size_t &resDataCnt);
    void sendSubDeviceCmd(unsigned char cmd, HartDevice dev, uint8_t *reqData, size_t reqDataCnt, uint8_t *resData, size_t &resDataCnt, uint8_t &status);
    json to_json();

private:
    thread autodiscoveryThread;

    /* HART General */
    hart_ip_pkt_t request;
    hart_ip_pkt_t response;
    
    int send(uint8_t *bytes, size_t len, int flags);
    int recv(uint8_t *buf, size_t len, int flags);
    int sendHeader();
    int sendData(uint8_t *data, size_t len);
    int recvData(uint8_t *buf);
    int sendPduFrame(hart_pdu_frame frame);
    int pduTransaction(hart_pdu_frame send_frame, hart_pdu_frame &recv_frame);
    int nestedPduTransaction(hart_pdu_frame send_frame, hart_pdu_frame &recv_frame);
    hart_pdu_frame recvPduFrame();

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

void autodiscoverLoop(HartMux *mux, int seconds);

json cmdRsponseToJson(int cmd, uint8_t *data, int len, uint8_t status);
json parseResponseBytes(uint8_t *bytes, string format, int count);