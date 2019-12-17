#include "data_types.hpp"

string hartStatusCodetoString(uint8_t msgId, uint8_t code) {
    if (msgId == 0) {
        switch(code) {
            case 0: return string("Success"); break;
            case 2: return string("Error: invalid selection (invalid master type)"); break;
            case 5: return string("Error: too few data bytes received"); break;
            case 6: return string("Error: device specific command error"); break;
            case 8: return string("Warning: set to nearest possible value (inactivity timer value"); break;
            case 14: return string("Error: version not supported"); break;
            case 15: return string("Error: all available sessions in use"); break;
            case 16: return string("Error: access restricted (session already established)"); break;
            default: return string("undefined");
        }
    } else if (msgId == 1 || msgId == 2) {
        switch(code) {
            case 0: return string("Success"); break;
            case 6: return string("Error: device specific command error"); break;
            default: return string("");
        }
    } else if (msgId == 3) {
        switch(code) {
            case 0: return string("Success"); break;
            case 5: return string("Error: too few data bytes received"); break;
            case 6: return string("Error: device specific command error"); break;
            case 15: return string("Error: unsupported message ID"); break;
            case 16: return string("Error: access restricted (server resources exhausted)"); break;
            default: return string("");
        }
    }
}

void printBytes(uint8_t *bytes, size_t len) {
    for (int i = 0; i < len; i++) {
        if (i && i % 8 == 0) cout << "  ";
        if (i && i % 32 == 0) cout << endl;
        cout << hex << setfill('0') << setw(2) << (uint32_t)bytes[i] << " ";
    }
    cout << dec << endl;
}

hart_pdu_frame buildPduFrame(uint8_t *addrUniq, uint8_t cmd, uint8_t *data, size_t dataSize) {
    hart_pdu_frame f;
    hart_pdu_delimiter d;
    d.frameType = hart_pdu_delimiter::STX;
    d.physicalLayerType = hart_pdu_delimiter::ASYNC;
    d.numExpansionBytes = 0;
    d.addressType = hart_pdu_delimiter::UNIQUE;
    f.delimiter = d;
    memcpy(f.addr, addrUniq, 5);
    f.cmd = cmd;
    memcpy(f.data, data, dataSize);
    f.byteCnt = dataSize;

    return f;
}


size_t serialize(hart_pdu_frame f, uint8_t *bytes) {
    memcpy(bytes, &f.delimiter, 1);
    // deal with varying address as indicated by delimiter
    int nextAddr = 1;
    if (f.delimiter.addressType == hart_pdu_delimiter::UNIQUE) {
        memcpy(&bytes[1], f.addr, 5);
        nextAddr += 5;
    } else {
        bytes[nextAddr++] = *f.addr | 0x80; // single byte polling addr
    }
    // deal with varying expansion bytes as indicated by delimeter
    for (int i=0; i<f.delimiter.numExpansionBytes; i++) {
        bytes[nextAddr++] = 0;
    }

    bytes[nextAddr++] = f.cmd;
    bytes[nextAddr++] = f.byteCnt;
    memcpy(&bytes[nextAddr], f.data, f.byteCnt);
    nextAddr += f.byteCnt;
    // calculate check byte
    f.chk = 0;
    for (int i=0; i<nextAddr; i++) {
        f.chk ^= bytes[i];
    }
    bytes[nextAddr] = f.chk;

    return nextAddr + 1;
}

hart_pdu_frame deserializeHartPduFrame(uint8_t *bytes) {
    hart_pdu_frame f;
    hart_pdu_delimiter d;
    f.delimiter = d;
    memcpy(&d, bytes, 1);
    // deal with varying address as indicated by delimiter
    int nextAddr = 1;
    if (d.addressType == hart_pdu_delimiter::UNIQUE) {
        memcpy(f.addr, &bytes[nextAddr], 5);
        nextAddr += 5;
    } else {
        f.addr[0] = bytes[nextAddr++]; // single byte polling addr
    }
    // deal with varying expansion bytes as indicated by delimeter
    for (int i=0; i<d.numExpansionBytes; i++) {
        nextAddr++;
    }
    f.cmd = bytes[nextAddr++];
    f.byteCnt = bytes[nextAddr++];
    memcpy(f.data, &bytes[nextAddr], f.byteCnt);
    nextAddr += f.byteCnt;
    // calculate check byte
    f.chk = 0;
    for (int i=0; i<nextAddr; i++) {
        f.chk ^= bytes[i];
    }

    #ifdef DEBUG
    cout << "d.frameType=" << (uint32_t)d.frameType << endl;
    cout << "f.addr="; printBytes(f.addr, 5);
    cout << "f.cmd=" << (uint32_t)f.cmd << endl;
    cout << "f.byteCnt=" << (uint32_t)f.byteCnt << endl;
    cout << "f.data: "; printBytes(f.data, f.byteCnt);
    #endif

    return f;
}


void serialize(hart_ip_hdr_t header, uint8_t *bytes) {
    bytes[0] = header.version;
    bytes[1] = header.msgType;
    bytes[2] = header.msgId;
    bytes[3] = header.status;
    serialize<uint16_t>(header.seqNo, &bytes[4]);
    serialize<uint16_t>(header.byteCount, &bytes[6]);
    // 8 bytes total
}

hart_ip_hdr_t deserializeHartIpHdr(uint8_t *bytes) {
    hart_ip_hdr_t header;
    header.version = bytes[0];
    header.msgType = bytes[1];
    header.msgId = bytes[2];
    header.status = bytes[3];
    header.seqNo = fromBytes<uint16_t>(&bytes[4]);
    header.byteCount = fromBytes<uint16_t>(&bytes[6]);
    // 8 bytes total
    return header;
}