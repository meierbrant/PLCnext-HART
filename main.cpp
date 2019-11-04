#include <iostream>
#include <string.h>
#include "lib/nettools.hpp"
#include "lib/socket.hpp"
#include "lib/hart_mux.hpp"

using namespace std;

string HART_MUX_IP = "192.168.254.26";

int main(int argc, char *argv[]) {
    // if (nettools::ping(HART_MUX_IP.c_str()) == 0) {
    //     cout << "Successful ping @ " << HART_MUX_IP << endl;
    // }

    // Test ability to convert between endianness
    float pi = 3.1415926536;
    uint8_t f[sizeof(float)];
    toBytes<float>(pi, f);
    cout << "bytes of pi: {"<<(int)f[0]<<", "<<(int)f[1]<<", "<<(int)f[2]<<", "<<(int)f[3]<<"}"<<endl;
    cout << "conversion from bytes to float: " << fromBytes<float>(f) << endl;

    // Socket g("www.google.com");
    // cout << "connected: " << g.connect() << endl;
    // cout << "sending..." << endl;
    // const char *msg = "Test";
    // int r = g.send(msg, strlen(msg), MSG_DONTWAIT);
    // cout << "sent " << r << " bytes" << endl;
    // g.close();

    HartMux hart_mux(HART_MUX_IP);
    hart_mux.initSession();
    hart_mux.sock.close();

    return 0;
}