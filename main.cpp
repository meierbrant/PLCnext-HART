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

    HartMux hart_mux(HART_MUX_IP);
    hart_mux.initSession();
    hart_mux.getUniqueAddr();
    hart_mux.sock.close();

    return 0;
}