#include <iostream>
#include <string.h>
#include "lib/nettools.hpp"
#include "lib/socket.hpp"
#include "lib/hart_mux.hpp"
#include "lib/cpp-httplib/httplib.h"

using namespace std;

// NOTE: this must be compiled with the -pthread flag because the httpserver is multithreaded

string HART_MUX_IP = "192.168.254.254";

void webserver() {
    // https://github.com/yhirose/cpp-httplib
    using namespace httplib;

    Server s;
    s.Get("/", [](const Request& req, Response& res) {
        res.set_content("This is the beginnings of a HART MUX web API!\n", "text/plain");
    });

    s.listen("localhost", 5900);
}

int main(int argc, char *argv[]) {
    // if (nettools::ping(HART_MUX_IP.c_str()) == 0) {
    //     cout << "Successful ping @ " << HART_MUX_IP << endl;
    // }
    
    
    // HartMux hart_mux(HART_MUX_IP);
    // hart_mux.initSession();
    // // hart_mux.beginSubDeviceAutodiscovery(5);
    // HartDevice sensor = hart_mux.readSubDeviceSummary(1);

    // hart_var_set sensor_vars = hart_mux.readSubDeviceVars(sensor);
    // displayVars(sensor_vars);
    // cout << endl;
    // hart_mux.print();
    // cout << "\tmax IO cards: " << (uint32_t)hart_mux.ioCapabilities.maxIoCards << endl;
    // cout << "\tmax channels per IO card: " << (uint32_t)hart_mux.ioCapabilities.maxChannels << endl;
    // cout << "\t# of devices connected: " << (uint32_t)hart_mux.ioCapabilities.numConnectedDevices << endl;
    // cout << "\tmaster mode: " << (uint32_t)hart_mux.ioCapabilities.masterMode << endl;

    // cout << endl;
    
    // while (!hart_mux.stopAutodiscovery) {
    //     hart_mux.autodiscoverSubDevices();
    //     hart_mux.listDevices();
    //     sleep(1);
    // }

    // hart_mux.closeSession();

    // web server on port 5900
    webserver();

    return 0;
}