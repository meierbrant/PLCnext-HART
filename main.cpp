#include <iostream>
#include <string.h>
#include "lib/nettools.hpp"
#include "lib/socket.hpp"
#include "lib/hart_mux.hpp"
#include "lib/cpp-httplib/httplib.h"

using namespace std;

// NOTE: this must be compiled with the -pthread flag because the httpserver is multithreaded

string HART_MUX_IP = "192.168.254.254";

int main(int argc, char *argv[]) {
    // if (nettools::ping(HART_MUX_IP.c_str()) == 0) {
    //     cout << "Successful ping @ " << HART_MUX_IP << endl;
    // }
    
    
    HartMux hart_mux(HART_MUX_IP);
    hart_mux.initSession();
    // //hart_mux.beginSubDeviceAutodiscovery(5);
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

    /***
     * webserver on port 5900
     * library: https://github.com/yhirose/cpp-httplib
     */
    using httplib::Server;
    using httplib::Request;
    using httplib::Response;
    using httplib::DataSink;

    Server s;
    s.Get("/", [](const Request& req, Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content("This is the beginnings of a HART MUX web API!\n\n\
        Routes:\n\
        GET\t/\twelcome page\n\
        GET\t/info\tHART MUX info\n", "text/plain");
    });

    s.Get("/info", [&hart_mux](const Request& req, Response& res) {
        hart_mux.autodiscoverSubDevices();
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(hart_mux.to_json().dump(), "text/json");
    });

    const uint64_t DATA_CHUNK_SIZE = 4;

    s.Get("/stream", [&](const Request &req, Response &res) {
    auto data = new std::string("abcdefg");

    res.set_content_provider(
        data->size(), // Content length
        [data](uint64_t offset, uint64_t length, DataSink sink) {
        const auto &d = *data;
        sink(&d[offset], std::min(length, DATA_CHUNK_SIZE));
        },
        [data] { delete data; });
    });

    s.listen("localhost", 5900);
    // end webserver


    hart_mux.closeSession();

    return 0;
}