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
    // hart_mux.beginSubDeviceAutodiscovery(5);
    HartDevice sensor = hart_mux.readSubDeviceSummary(1);

    cout << endl;
    hart_mux.print();
    cout << "\tmax IO cards: " << (uint32_t)hart_mux.ioCapabilities.maxIoCards << endl;
    cout << "\tmax channels per IO card: " << (uint32_t)hart_mux.ioCapabilities.maxChannels << endl;
    cout << "\t# of devices connected: " << (uint32_t)hart_mux.ioCapabilities.numConnectedDevices << endl;
    cout << "\tmaster mode: " << (uint32_t)hart_mux.ioCapabilities.masterMode << endl;

    cout << endl;
    
    while (!hart_mux.stopAutodiscovery) {
        hart_mux.autodiscoverSubDevices();
        hart_mux.listDevices();
        sleep(1);
    }

    hart_mux.closeSession();

    return 0;
}