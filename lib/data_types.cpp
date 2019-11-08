#include "data_types.hpp"

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