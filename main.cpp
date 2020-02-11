#include <iostream>
#include <string.h>
#include "lib/nettools.hpp"
#include "lib/socket.hpp"
#include "lib/hart_mux.hpp"
#include "lib/data_types.hpp"
#include "lib/udp_gateway_discovery.hpp"
#include "lib/cpp-httplib/httplib.h"

using namespace std;

// NOTE: this must be compiled with the -pthread flag because the httpserver is multithreaded

string HART_MUX_IP = "192.168.254.254";

int main(int argc, char *argv[]) {
    // if (nettools::ping(HART_MUX_IP.c_str()) == 0) {
    //     cout << "Successful ping @ " << HART_MUX_IP << endl;
    // }
    
    // hart_mux.initSession();

    // hart_mux.autodiscoverSubDevices();
    // HartDevice sensor = hart_mux.readSubDeviceSummary(1);
    // sensor.print();

    // cout << "sending cmd 11" << endl;
    // uint8_t data[512];
    // uint8_t res[512];
    // size_t resSize;
    // hart_mux.sendCmd(11, sensor.addrUniq, data, 0, res, resSize);
    // cout << "recieved" << endl;
    // printBytes(res, resSize);

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

    // cout << discoverGWs().dump(2) << endl;

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
        cout << "GET /" << endl;
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content("This is the beginnings of a HART MUX web API!\n\n\
        Routes:\n\
        GET\t/\t\t\t\t\twelcome page\n\
        GET\t/gw/{serialNo}/info\t\t\tHART MUX info\n\
        GET\t/gw/discover\t\t\t\tdiscover all gateways on the network\n\
        GET\t/gw/{serialNo}/vars/{card}/{ch}\t\tread vars from device\n", "text/plain");
    });

    // GET /gw/{serialNo}/info
    s.Get(R"(/gw/(\d+)/info)", [](const Request& req, Response& res) {
        string serialNo(req.matches[1]);
        json gws = discoverGWs();
        json gwData;
        for (int i=0; i<gws["gateways"].size(); i++) {
            gwData = gws["gateways"][i];
            if (gwData["serialNo"] == serialNo) break;
        }
        
        HartMux hart_mux(gwData["ip"]);
        hart_mux.initSession();
        hart_mux.autodiscoverSubDevices();
        hart_mux.closeSession();
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(hart_mux.to_json().dump(), "text/json");
    });

    s.Get("/gw/discover", [](const Request& req, Response& res) {
        json gwData = discoverGWs();
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(gwData.dump(), "text/json");
    });

    // GET /gw/{serialNo}/vars/{ioCard}/{channel}
    s.Get(R"(/gw/(\d+)/vars/(\d+)/(\d+))", [](const Request& req, Response& res) {
        string serialNo(req.matches[1]);
        string ioCard(req.matches[2]);
        string channel(req.matches[3]);

        json gws = discoverGWs();
        json gwData;
        for (int i=0; i<gws["gateways"].size(); i++) {
            gwData = gws["gateways"][i];
            if (gwData["serialNo"] == serialNo) break;
        }
        HartMux hart_mux(gwData["ip"]);
        hart_mux.initSession();
        hart_mux.autodiscoverSubDevices();
        HartDevice dev;
        for (int i=0; i<hart_mux.ioCapabilities.numConnectedDevices; i++) {
            dev = hart_mux.devices[i];
            if (dev.ioCard == (uint8_t)stoi(ioCard) && dev.channel == (uint8_t)stoi(channel)) break;
        }

        hart_var_set var_set = hart_mux.readSubDeviceVars(dev);

        hart_mux.closeSession();
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(to_json(var_set).dump(), "text/json");
    });

    const uint64_t DATA_CHUNK_SIZE = 4;

    // s.Get("/stream", [&](const Request &req, Response &res) {
    //     auto data = new std::string("abcdefg");

    //     res.set_content_provider(data->size(), // Content length
    //         [data](uint64_t offset, uint64_t length, DataSink sink) {
    //             const auto &d = *data;
    //             sink(&d[offset], std::min(length, DATA_CHUNK_SIZE));
    //         },
    //         [data] { delete data; });
    // });

    string domain = "localhost";
    unsigned int port = 5900;
    cout << "starting webserver at "+domain+":"<<port<<endl;
    s.listen(domain.c_str(), port);
    // end webserver

    return 0;
}