#include "hart_mux.hpp"
#include "hart_devices.hpp"

#include "nettools.hpp"
#include <string.h>

#define DEBUG

// Constructor defined in header

int HartMux::initSession() {
    cout << "initSession()" << endl;
    sock.connect();
    memset(&request, 0, sizeof(hart_ip_pkt_t));

    /* Send Request */
    request.header.version = 1;
    request.header.msgType = 0; // 0: request
    request.header.msgId = 0;   // 0: session init
    request.header.status = 0;  // 0: init to 0 by client
    request.header.seqNo = 1;   // sequence init start value
    request.header.byteCount = 13; // 13 bytes of whole frame for init session telegram

    uint8_t body[request.header.byteCount-sizeof(request.header)];
    memset(body, 0, sizeof(body));
    request.body = body;
    uint8_t initSessionMaster = 1; // 1: primary master
    body[0] = initSessionMaster;
    serialize<uint32_t>(inactivityTimeout, &body[1]);

    sendData(body, sizeof(body));

    uint8_t data[13];
    recvData(data);

    return 0;
}

int HartMux::closeSession() {
    cout << "closeSession()" << endl;
    request.header.version = 1;
    request.header.msgType = 0; // 0: request
    request.header.msgId = 1;   // 0: session close
    request.header.status = 0;  // 0: init to 0 by client
    request.header.byteCount = 8;

    sendHeader();

    uint8_t data[8];
    recvData(data);

    if (response.header.status == 0) sock.close();

    return response.header.status;
}

uint8_t* HartMux::getUniqueAddr() { // cmd 0 by short address for gateway
    cout << "getUniqAddr()" << endl;

    sendCmd(0, (uint8_t)0);
    // PDU frame up to data bytes should be 6 bytes
    uint16_t deviceTypeCode = (response.body[7] << 8) | response.body[8];
    setTypeInfo(deviceTypeCode);
    addrUniq[0] = response.body[7];
    addrUniq[1] = response.body[8];
    memcpy(&addrUniq[2], &response.body[15], 3);

    print();

    return (uint8_t *)addrUniq;
}

void HartMux::readIOSystemCapabilities() { // cmd 74
    cout << "readIOSystemCapabilities()" << endl;

    sendCmd(74, addrUniq);
    ioCapabilities.maxIoCards = response.body[10];
    ioCapabilities.maxChannels = response.body[11];
    ioCapabilities.subDevicesPerCh = response.body[12];
    ioCapabilities.numConnectedDevices = (response.body[13] << 8) | response.body[14];
    ioCapabilities.numDelayedResp = response.body[15];
    ioCapabilities.masterMode = response.body[16];
    ioCapabilities.retryCnt = response.body[17];
}

HartDevice HartMux::readSubDeviceSummary(uint16_t index) {
    uint8_t data[2];
    serialize(index, data);
    sendCmd(84, addrUniq, data, 2);

    HartDevice d;
    size_t pduHdrSize = 10;
    memcpy(d.addrUniq, &response.body[pduHdrSize+6], 5);
    d.ioCard = response.body[pduHdrSize+2];
    d.channel = response.body[pduHdrSize+3];
    d.manufacturerId = (response.body[pduHdrSize+4] << 8) | response.body[pduHdrSize+5];
    d.cmdRevLvl = response.body[pduHdrSize+11];
    char longTag[33];
    memcpy(longTag, &response.body[pduHdrSize+12], 32);
    longTag[32] = 0x00;
    d.longTag = string(longTag);
    d.revision = response.body[pduHdrSize+44];
    d.profile = response.body[pduHdrSize+45];
    d.pvtLabelDistCode = (response.body[pduHdrSize+46] << 8) | response.body[pduHdrSize+47];
    return d;
}


/**
 * pollAddr is zero by default since drop networks are not supported yet
 */
int HartMux::sendCmd(unsigned char cmd, uint8_t pollAddr) {
    size_t byteCount = 0;
    uint8_t data[512];
    memset(data, 0, sizeof(data));
    if (cmd == 0) {
        hart_pdu_frame f;
        hart_pdu_delimiter d;
        d.frameType = hart_pdu_delimiter::STX;
        d.physicalLayerType = hart_pdu_delimiter::ASYNC;
        d.numExpansionBytes = 0;
        d.addressType = hart_pdu_delimiter::POLLING;
        f.delimiter = d;
        uint8_t del;
        memcpy(&del, &d, 1);
        cout << hex << "delimiter=" << (uint32_t)del << dec << endl;
        f.addr = addrUniq;
        f.cmd = cmd;
        f.byteCnt = 0;
        byteCount += serialize(f, data);
    } else {
        cout << "Error: short address only allowed for cmd 0" << endl;
        return -1;
    }

    request.header.version = 1;
    request.header.msgType = 0; // 0: Request
    request.header.msgId = 3;   // 3: Token-Passing PDU
    request.header.status = 0;  // 0: init to 0 by client
    
    sendData(data, byteCount);

    uint8_t buf[512];
    memset(buf, 0, 512);
    
    return recvData(buf);
}

/**
 * param uniqueAddr is 5 bytes
 */
int HartMux::sendCmd(unsigned char cmd, uint8_t *uniqueAddr, uint8_t *reqData, size_t reqDataCnt) {
    size_t byteCount = 0;
    uint8_t data[512];
    memset(data, 0, sizeof(data));

    hart_pdu_frame f;
    hart_pdu_delimiter d;
    d.frameType = hart_pdu_delimiter::STX;
    d.physicalLayerType = hart_pdu_delimiter::ASYNC;
    d.numExpansionBytes = 0;
    d.addressType = hart_pdu_delimiter::UNIQUE;
    f.delimiter = d;
    f.addr = addrUniq;
    f.cmd = cmd;
    f.byteCnt = reqDataCnt;
    f.data = reqData;
    byteCount += serialize(f, data);

    request.header.version = 1;
    request.header.msgType = 0; // 0: Request
    request.header.msgId = 3;   // 3: Token-Passing PDU
    request.header.status = 0;  // 0: init to 0 by client
    sendData(data, byteCount);

    uint8_t buf[512];
    memset(buf, 0, 512);
    
    return recvData(buf);
}

int HartMux::send(uint8_t *bytes, size_t len, int flags) {
    int r = sock.send(bytes, len, flags);
    if (r < 0) {
        perror("Error recieving packet header from HART MUX");
        return -1;
    } else if (r == 0) {
        perror("The HART MUX hung up");
        return -1;
    }
}

int HartMux::recv(uint8_t *buf, size_t len, int flags) {
    int r = sock.recv(buf, len, flags);
    if (r < 0) {
        perror("Error recieving packet header from HART MUX");
    } else if (r == 0) {
        cout << "The HART MUX hung up" << endl;
    }
    return r;
}

int HartMux::sendHeader() {
    uint8_t bytes[request.header.byteCount];
    memset(bytes, 0, sizeof(bytes));
    serialize(request.header, bytes);
    send(bytes, request.header.byteCount, 0);
}

int HartMux::sendData(uint8_t *data, size_t len) {
    // send packet to MUX
    request.header.byteCount = len + sizeof(hart_ip_hdr_t);
    uint8_t bytes[request.header.byteCount];
    memset(bytes, 0, sizeof(bytes));
    serialize(request.header, bytes);
    
    memcpy(&bytes[sizeof(hart_ip_hdr_t)], data, len);
    send(bytes, request.header.byteCount, 0);
    #ifdef DEBUG
    cout << "sent packet:" << endl;
    printBytes(bytes, request.header.byteCount);
    cout << endl;
    #endif
}

int HartMux::recvData(uint8_t *buf) {
    // get response header from MUX
    uint8_t hdr_buf[sizeof(hart_ip_hdr_t)];
    memset(hdr_buf, 0, sizeof(hdr_buf));
    recv(hdr_buf, sizeof(hdr_buf), 0);
    response.header = deserializeHartIpHdr(hdr_buf);
    #ifdef DEBUG
    cout << "received header (network byte order) (expecting " << response.header.byteCount - sizeof(hart_ip_hdr_t) << " more bytes):" << endl;
    printBytes(hdr_buf, sizeof(hart_ip_hdr_t));
    #endif

    if ((response.header.seqNo != request.header.seqNo) || response.header.status > 0) {
        cout << hartStatusCodetoString(response.header.msgId, response.header.status) << endl;
        err = true;
        return -1;
    } else {
        if (request.header.seqNo > 0xfffe) {
            request.header.seqNo = 1;
        } else {
            request.header.seqNo++;
        }        
    }
    
    // get rest of response from MUX
    uint8_t bdy_buf[response.header.byteCount - sizeof(hart_ip_hdr_t)];
    memset(buf, 0, sizeof(bdy_buf));
    int r = 0;
    if (sizeof(bdy_buf) > 0) {
        r = recv(buf, sizeof(bdy_buf), 0);
        response.body = buf;
        //*buf = bdy_buf;
        #ifdef DEBUG
        cout << "received data (network byte order):" << endl;
        printBytes(response.body, sizeof(bdy_buf));
        cout << endl;
        #endif
    }

    return r;
}