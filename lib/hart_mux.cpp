#include "hart_mux.hpp"

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

uint24_t HartMux::getUniqueAddr() { // cmd 0 by short address for gateway
    cout << "getUniqAddr()" << endl;
    uint8_t data[5];
    memset(data, 0, sizeof(data));
    /* PDU Frame */
    // delimiter
    data[0] = 0x02; // STX frame
    // address
    data[1] = 0x80;
    // expansion bytes (normally zero)
    // command
    data[2] = 0;
    // byte count
    data[3] = 0;
    // data
    // check byte
    data[4] = data[0] ^ data[1] ^ data[2] ^ data[3];

    // data[0] = 0xb0;
    // data[1] = 0x13;
    // data[2] = 0xdd;
    // data[3] = 0x49;
    // data[4] = 0x14;
    // data[8] = 0x02; // delimeter: Frame Type STX 0x02(Indicates Slave or Burst Mode device)
	// 	                    // ACK  0x06
	// 	                    // BACK 0x01
    // data[9] = 0x80;
    // data[10] = 0x00; // command
    // data[11] = 0x00; // byte count; no data bytes

    // uint8_t checkByte = data[8] ^ data[9] ^ data[10] ^ data[11];
    // data[12] = checkByte;

    request.header.version = 1;
    request.header.msgType = 0; // 0: Request
    request.header.msgId = 3;   // 3: Token-Passing PDU
    request.header.status = 0;  // 0: init to 0 by client
    request.header.byteCount = sizeof(hart_ip_hdr_t) + sizeof(data); // 13 bytes of the whole frame for telegram

    cout << request.header.byteCount << endl;
    sendData(data, sizeof(data));

    uint8_t buf[37];
    memset(buf, 0, 37);
    int dataLen = recvData(buf);
    if (dataLen < 0) return -1;
    uint24_t deviceId = fromBytes<uint24_t>(&buf[9], 3);
    cout << "deviceId = ";
    printBytes(deviceId);

    return deviceId;
}

int HartMux::sendCmd(unsigned char cmd) {
    request.header.version = 1;
    request.header.msgType = 0; // 0: Request
    request.header.msgId = 3;   // 3: Token-Passing PDU
    request.header.status = 0;  // 0: init to 0 by client
    request.header.byteCount = 8; // 13 bytes of whole frame for telegram

    // PDU frame
    uint8_t delimiter = 2; // Frame Type STX 0x02(Indicates Slave or Burst Mode device)
                        // ACK 0x06
                        // BACK 0x01
    uint8_t byteCnt = 0;

    uint8_t data[32];
    data[0] = delimiter;
    data[1] = 0x80;
    data[2] = cmd;
    data[3] = byteCnt;
    data[4] = delimiter ^ 0x80 ^ cmd ^ byteCnt; // check byte


    // TODO: send packet to MUX

    // TODO: get response from MUX

    //serverInfo = fromBytes<muxPacketHeader>(request.header);
    /*
    response.body[0] // delimiter
    response.body[1:5] // server address (short)
    response.body[6] // server cmd
    response.body[7] // server byteCnt
    */
    addressUniqGW[0] = response.body[7]; // Device type
    addressUniqGW[1] = response.body[8];
    addressUniqGW[2] = response.body[9]; // Device ID
    addressUniqGW[3] = response.body[10];
    addressUniqGW[4] = response.body[11];

    if ((response.header.seqNo != request.header.seqNo) || response.header.status > 0) {
        // TODO: more detailed error
        err = true;
        return 1;
    } else {
        if (request.header.seqNo > 0xfffe) {
            request.header.seqNo = 1;
        } else {
            request.header.seqNo++;
        }
        
        longAddressGW = "";
        // for (int i=0; i<sizeof(addressUniqGW); i++) {
        //     longAddressGW = longAddressGW << addressUniqGW[i]);
        // }
        
    }

    return 0;
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
    serializeHartIpHdr(request.header, bytes);
    send(bytes, request.header.byteCount, 0);
}

int HartMux::sendData(uint8_t *data, size_t len) {
    // send packet to MUX
    request.header.byteCount = len + sizeof(hart_ip_hdr_t);
    uint8_t bytes[request.header.byteCount];
    memset(bytes, 0, sizeof(bytes));
    serializeHartIpHdr(request.header, bytes);
    
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
        
        longAddressGW = "";
        
    }
    
    // get rest of response from MUX
    uint8_t bdy_buf[response.header.byteCount - sizeof(hart_ip_hdr_t)];
    memset(buf, 0, sizeof(bdy_buf));
    int r = 0;
    if (sizeof(bdy_buf) > 0) {
        cout << "TODO: handle receiving more data than buf allocated for" << endl;
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