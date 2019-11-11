#pragma once
#include <cinttypes>
#include <iostream>
#include <iomanip>
#include <cstring>

// uncomment if endianness of PLCnext does not match network byte order
#define REVERSE

using std::cout;
using std::endl;
using std::hex;
using std::dec;
using std::setfill;
using std::setw;
using std::string;

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

struct hart_unique_id_res_t {
    uint8_t a = 254;
    uint8_t deviceType;
    
};

void printBytes(uint8_t *bytes, size_t len);
void serializeHartIpHdr(hart_ip_hdr_t header, uint8_t *bytes);
hart_ip_hdr_t deserializeHartIpHdr(uint8_t *bytes);

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

