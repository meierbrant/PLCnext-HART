#include "udp_gateway_discovery.hpp"

#include "data_types.hpp"
#include "udp_socket.hpp"
#include "nettools.hpp"
#include "nlohmann/json.hpp"
#include <chrono>

using namespace std;
using nlohmann::json;
using Clock = std::chrono::high_resolution_clock;

class NoGatewaysException: public exception {
  virtual const char* what() const throw() {
    return "no gateways responded";
  }
};

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

int gwDiscoverAttempt(json network_info, json *gwArray) {
    json newGws = json::array();
    string query, serialNo, bcastAddr, recvAddr;
    char buf[256];
    UdpSocket s(5000);
    bcastAddr = network_info["bcast"];
    recvAddr = network_info["bcast"];

    // FIXME: inconsistently getting correct recvAddr (50%)
    query = "GW PL ETH search.req\0";
    s.sendto(bcastAddr, query.c_str(), query.length());
    int n = s.recvfromMultiple(bcastAddr, buf, sizeof(buf), [&newGws, &network_info](string ip, char *data, size_t len){
        if (nettools::ip_is_on_subnet(ip, network_info["netmask"], network_info["network"])) {
            newGws[newGws.size()] = {
                {"ip", ip},
                {"serialNo", string(data)}
            };
        }
    }, 0, 2000000);
    if (n<0) return n;

    for (int i=0; i<newGws.size(); i++) {
        string ip = newGws[i]["ip"];
        serialNo = newGws[i]["serialNo"];
        query = "GW PL ETH ident.req: " + serialNo + '\0';
        s.sendto(ip, query.c_str(), query.length());
        s.recvfrom(ip, buf, sizeof(buf));
        
        json ioArray = json::array();
        string module;
        for (int i=0; i<5; i++) {
            module = resolveIoCardType(buf[i]);
            if (!module.empty()) ioArray[i] = module;
        }
        newGws[i]["modules"] = ioArray;
    }

    // merge newGws with gwArray
    for (int i=0; i<newGws.size(); i++) {
        bool exists = false;
        for (int j=0; j<(*gwArray).size(); j++) {
            string existingSn = (*gwArray)[j]["serialNo"];
            string newSn = newGws[i]["serialNo"];
            if (existingSn.compare(newSn) == 0) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            (*gwArray)[(*gwArray).size()] = newGws[i];
        }
    }

    return 0;
}

json discoverGWs(json network_info) {
    json gwArray = json::array();

    // UDP is unreliable, so send the bcast and listen multiple times
    int numtries = 3;

    for (int i=0; i<numtries; i++) {
        if (gwDiscoverAttempt(network_info, &gwArray) < 0) {
            printf("gw discovery attempt failed\n");
            throw FailedGwDiscoveryException();
            // this can happen when there was no GW on the given broadcast
            i--;
        }
    }

    return gwArray;
}

// int main() {
//     cout << discoverGWs().dump(2) << endl;
// }