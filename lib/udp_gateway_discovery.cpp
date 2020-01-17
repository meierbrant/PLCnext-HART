#include "data_types.hpp"
#include "udp_socket.hpp"

using namespace std;

bool isBcastAddr(string ip) {
    char bcast_check[] = "255";
    if (ip[12] == bcast_check[0] &&
        ip[13] == bcast_check[1] &&
        ip[14] == bcast_check[2]) {
        cout << "is broadcast\n" << endl;
        return true;
    }
    return false;
}

int main(int argc, char *argv[])
{
    string query, serialNo, recvAddr;
    char buf[256];
    UdpSocket s(5000);
    string bcastAddr = "192.168.254.255";
    recvAddr = bcastAddr;

    // LEFTOFF: inconsistently getting correct recvAddr (50%)
    // This loop actually doesn't help at all.
    // while (isBcastAddr(recvAddr)) {
    query = "GW PL ETH search.req\0";
    s.sendto(bcastAddr, query.c_str(), query.length());
    s.recvfrom(recvAddr, buf, sizeof(buf));
    serialNo = string(buf);
    // }
    cout << "to " << bcastAddr << ": " << query << endl;
    cout << "from: " << recvAddr << ": " << serialNo << endl;

    string addr2 = bcastAddr;
    query = "GW PL ETH ident.req: " + serialNo + '\0';
    s.sendto(recvAddr, query.c_str(), query.length());
    int n = s.recvfrom(addr2, buf, sizeof(buf));
    cout << "to " << recvAddr << ": " << query << endl;
    printBytes(query);
    cout << "from: " << addr2 << ": " << endl;
    printBytes((uint8_t*)buf, n);
    cout << "n=" << n << endl;
}