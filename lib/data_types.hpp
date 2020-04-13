#pragma once
#include <cinttypes>
#include <iostream>
#include <iomanip>
#include <cstring>
#include "hart_device.hpp"
#include "nlohmann/json.hpp"

// uncomment if endianness of PLCnext does not match network byte order
#define REVERSE

using std::cout;
using std::endl;
using std::hex;
using std::dec;
using std::setfill;
using std::setw;
using std::string;
using nlohmann::json;

struct uint24_t {
    unsigned int data: 24;
};

struct hart_ip_hdr_t { // 8 bytes total
    uint8_t version;   //0
    uint8_t msgType;   //1
    uint8_t msgId;     //2
    uint8_t status;    //3
    uint16_t seqNo;     //4-5
    uint16_t byteCount; //6-7
};

string hartStatusCodetoString(uint8_t msgId, uint8_t code);

struct hart_ip_pkt_t {
    hart_ip_hdr_t header; // bytes 0-7
    uint8_t *body;
};

struct hart_io_capabilities {
    uint8_t maxIoCards;
    uint8_t maxChannels;
    uint8_t subDevicesPerCh;
    uint16_t numConnectedDevices;
    uint8_t numDelayedResp;
    uint8_t masterMode;
    uint8_t retryCnt; // 2 to 5, 3 is default
};

struct hart_pdu_delimiter {
    enum {
        BACK = 1,
        STX = 2,
        ACK = 6
    } frameType: 3;
    enum {
        ASYNC = 0,
        SYNC = 1
    } physicalLayerType: 2;
    uint8_t numExpansionBytes: 2;
    enum {
        POLLING = 0,
        UNIQUE = 1
    } addressType: 1;
};

struct hart_pdu_frame {
    hart_pdu_delimiter delimiter;
    uint8_t addr[5];
    uint8_t cmd;
    uint8_t byteCnt;
    uint8_t data[255];
    uint8_t chk;
};

hart_pdu_frame buildPduFrame(uint8_t *addrUniq, uint8_t cmd, uint8_t *data=nullptr, size_t dataSize=0);

void printBytes(uint8_t *bytes, size_t len);

string bytesToHexStr(uint8_t *bytes, int len);

void serialize(hart_ip_hdr_t header, uint8_t *bytes);
hart_ip_hdr_t deserializeHartIpHdr(uint8_t *bytes);

size_t serialize(hart_pdu_frame f, uint8_t *bytes);
hart_pdu_frame deserializeHartPduFrame(uint8_t *bytes);

template <typename T>
T fromBytes(uint8_t* bytes) {
    T x;
    uint8_t* bytes2 = bytes;
    #ifdef REVERSE
    const size_t s = sizeof(T);
    uint8_t reverse_bytes[s];
    for (int i=0; i<s; i++) {
        reverse_bytes[s-1-i] = bytes[i];
    }
    bytes2 = reverse_bytes;
    #endif
    memcpy(&x, bytes2, sizeof(T));
    return x;
}

template <typename T>
T fromBytes(uint8_t* bytes, size_t size) {
    T x;
    uint8_t* bytes2 = bytes;
    #ifdef REVERSE
    uint8_t reverse_bytes[size];
    for (int i=0; i<size; i++) {
        reverse_bytes[size-1-i] = bytes[i];
    }
    bytes2 = reverse_bytes;
    #endif
    memcpy(&x, bytes2, sizeof(T));
    return x;
}

template <typename T>
void toBytes(T x, uint8_t* bytes) {
    memcpy(bytes, &x, sizeof(T));
    #ifdef REVERSE
    const size_t s = sizeof(T);
    uint8_t reverse_bytes[s];
    for (int i=0; i<s; i++) {
        reverse_bytes[s-1-i] = bytes[i];
    }
    for (int i=0; i<s; i++) {
        bytes[i] = reverse_bytes[i];
    }
    #endif    
}

template <typename T>
void serialize(T x, uint8_t* bytes) {
    toBytes(x, bytes);
}

template <typename T>
void printBytes(T obj) {
    cout << hex << setfill('0');  // needs to be set only once
    unsigned char *ptr = reinterpret_cast<unsigned char *>(&obj);
    for (int i = 0; i < sizeof(T); i++, ptr++) {
        if (i && i % sizeof(uint64_t) == 0) {
            cout << endl;
        }
        cout << setw(2) << static_cast<unsigned>(*ptr) << " ";
    }
    cout << dec << endl;
}

json to_json(hart_var_set vars);

string to_hex_string(uint16_t data);