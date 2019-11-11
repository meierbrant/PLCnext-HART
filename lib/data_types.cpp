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

void serializeHartIpHdr(hart_ip_hdr_t header, uint8_t *bytes) {
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