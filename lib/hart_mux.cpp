#include "hart_mux.hpp"

#include "nettools.hpp"
#include <string.h>

// Constructor defined in header

int HartMux::initSession() {
    std::cout << "initSession()" << std::endl;
    sock.connect();
    memset(&request, 0, sizeof(hart_ip_pkt_t));

    /* Send Request */
    // Client information
    request.header.version = 1;
    request.header.msgType = 0; // 0: request
    request.header.msgId = 0;   // 0: session init
    request.header.status = 0;  // 0: init to 0 by client
    request.header.seqNo = 1;   // sequence init start value
    request.header.byteCount = 13; // 13 bytes of whole frame for init session telegram

    uint8_t initSessionMaster = 1; // 1: primary master
    request.body[0] = initSessionMaster;

    // Inactivity Close Timer
    serialize<uint32_t>(inactivityTimeout, &request.body[1]);

    // send packet to MUX
    // NOTE: 
    cout << "sending packet..." << endl;
    uint8_t bytes[sizeof(hart_ip_pkt_t)];
    memset(bytes, 0, sizeof(bytes));
    //serialize<hart_ip_pkt_t>(request, bytes);
    //serializeHartIpHdr(request.header, bytes);
    bytes[0] = request.header.version;
    bytes[1] = request.header.msgType;
    bytes[2] = request.header.msgId;
    bytes[3] = request.header.status;
    bytes[4] = request.header.seqNo >> 8;
    bytes[5] = request.header.seqNo;
    bytes[6] = request.header.byteCount >> 8;
    bytes[7] = request.header.byteCount;
    bytes[8] = 1;
    bytes[9] = inactivityTimeout >> 24;
    bytes[10] = inactivityTimeout >> 16;
    bytes[11] = inactivityTimeout >> 8;
    bytes[12] = inactivityTimeout;
    printBytes(bytes, request.header.byteCount);
    int r = sock.send(bytes, request.header.byteCount, 0);
    if (r < 0) {
        perror("Error sending packet to HART MUX");
        return -1;
    } else if (r == 0) {
        perror("The HART MUX hung up");
        return -1;
    }
    cout << "sent " << r << " bytes" << endl;

    // get response from MUX
    cout << "receiving packet..." <<endl;
    uint8_t buf[512];
    r = sock.recv(buf, sizeof(hart_ip_pkt_t), 0);
    if (r < 0) {
        perror("Error recieving packet from HART MUX");
        return -1;
    } else if (r == 0) {
        perror("The HART MUX hung up");
        return -1;
    }
    std::cout << "recieved " << r << " bytes: \t" << buf << std::endl;
    
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
    }

    return 0;
}

int HartMux::getUniqueAddr() { // cmd 0 by short address for gateway
    return sendCmd(0);
}

int HartMux::sendCmd(unsigned char cmd) {
    request.header.version = 1;
    request.header.msgType = 0; // 0: Request
    request.header.msgId = 3;   // 3: Token-Passing PDU
    request.header.status = 0;  // 0: init to 0 by client
    request.header.byteCount = 13; // 13 bytes of whole frame for telegram

    // PDU frame
    uint8_t delimiter = 2; // Frame Type STX 0x02(Indicates Slave or Burst Mode device)
                        // ACK 0x06
                        // BACK 0x01
    uint8_t byteCnt = 0;

    // body[0] is really the packet[8]
    request.body[0] = delimiter;
    request.body[1] = 0x80;
    request.body[2] = cmd;
    request.body[3] = byteCnt;
    request.body[4] = delimiter ^ 0x80 ^ cmd ^ byteCnt; // check byte


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