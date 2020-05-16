#include "data_types.hpp"
#include <sstream>

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

string bytesToHexStr(uint8_t *bytes, int len) {
    std::stringstream ss;
    ss << hex;

    for (int i=0; i<len; ++i)
        ss << setw(2) << setfill('0') << (int)bytes[i] << " ";

    return ss.str();
}

size_t serialize(hart_pdu_frame f, uint8_t *bytes, bool skip_checkbyte) {
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
    if (skip_checkbyte) nextAddr--;

    return nextAddr + 1;
}

hart_pdu_frame deserializeHartPduFrame(uint8_t *bytes, bool is_nested_frame) {
    cout << "deserialzieHartPduFrame()" << endl;
    hart_pdu_frame f;
    hart_pdu_delimiter d;
    memcpy(&d, bytes, 1);
    f.delimiter = d;
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

    // #ifdef DEBUG
    cout << "f.delimiter:" << endl;
    cout << "\taddressType = " << (unsigned int)f.delimiter.addressType << endl;
    cout << "\tnumExpansionBytes = " << (unsigned int)f.delimiter.numExpansionBytes << endl;
    cout << "\tframeType = " << (unsigned int)f.delimiter.frameType << endl;
    cout << "f.addr="; printBytes(f.addr, 5);
    cout << "f.cmd=" << (uint32_t)f.cmd << endl;
    cout << "f.byteCnt=" << (uint32_t)f.byteCnt << endl;
    cout << "f.data: "; printBytes(f.data, f.byteCnt);
    printf("f.chk=0x%x\n", (uint32_t)f.chk);
    // #endif

    // nested frames (from cmd 77) don't have checkbytes
    if (!is_nested_frame && f.chk != bytes[nextAddr]) {
        throw MismatchedCheckbytesException();
    }

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

json to_json(hart_var_set vars) {
    json data = {
        {"loopCurrent", {
            {"units", "mA"},
            {"value", vars.loopCurrent},
        }},
        {"pv", {
            {"units", vars.pv.units},
            {"value", vars.pv.value}
        }},
        {"sv", {
            {"units", vars.sv.units},
            {"value", vars.sv.value}
        }},
        {"tv", {
            {"units", vars.tv.units},
            {"value", vars.tv.value}
        }},
        {"qv", {
            {"units", vars.qv.units},
            {"value", vars.qv.value}
        }}
    };
    return data;
}

string to_hex_string(uint16_t data) {
    const char hex[] = "0123456789ABCDEF";
    char str[5];
    #ifdef REVERSE
    str[0] = hex[data >> 12];
    str[1] = hex[(data >> 8) & 0xf];
    str[2] = hex[(data >> 4) & 0xf];
    str[3] = hex[data & 0xf];
    #else
    str[0] = hex[data & 0xf];
    str[1] = hex[(data >> 4) & 0xf];
    str[2] = hex[(data >> 8) & 0xf];
    str[3] = hex[data >> 12];
    #endif
    str[4] = '\0';

    return string(str);
}