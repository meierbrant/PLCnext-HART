#include "data_types.hpp"
#include "udp_socket.hpp"
#include "nlohmann/json.hpp"

using namespace std;
using nlohmann::json;

bool isBcastAddr(string ip) {
    char bcast_check[] = "255";
    if (ip[12] == bcast_check[0] &&
        ip[13] == bcast_check[1] &&
        ip[14] == bcast_check[2]) {
        return true;
    }
    return false;
}

string resolveIoCardType(char moduleCode) {
    switch(moduleCode) {
        case 0x0F:
            return "GW PL HART8+AI";
        case 0x43:
            return "GW PL HART4-R";
        case 0x47:
            return "GW PL HART8-R";
        case 0x6B:
            return "GW PL HART4";
        case 0x6F:
            return "GW PL HART8";
        case 0x3B:
            return "GW PL DIO4";
        default:
            return "";
    }
}

json discoverGWs(string bcastAddr) {
    json gwData;
    string query, serialNo, recvAddr;
    char buf[256];
    UdpSocket s(5000);
    recvAddr = bcastAddr;

    // UDP is unreliable, so send the bcast and listen multiple times
    int numtries = 3;

    // FIXME: inconsistently getting correct recvAddr (50%)
    query = "GW PL ETH search.req\0";
    s.sendto(bcastAddr, query.c_str(), query.length());
    s.recvfrom(recvAddr, buf, sizeof(buf));
    serialNo = string(buf);
    
    query = "GW PL ETH ident.req: " + serialNo + '\0';
    s.sendto(recvAddr, query.c_str(), query.length());
    int n = s.recvfrom(recvAddr, buf, sizeof(buf));
    // printBytes((uint8_t*)buf, n);
    if (n<0) return json::object();
    json ioArray = json::array();
    string module;
    for (int i=0; i<5; i++) {
        module = resolveIoCardType(buf[i]);
        if (!module.empty()) ioArray[i] = module;
    }
    gwData[0] = {
        {"ip", recvAddr},
        {"serialNo", serialNo},
        {"modules", ioArray}
    };
    return gwData;
}

// int main() {
//     cout << discoverGWs().dump(2) << endl;
// }