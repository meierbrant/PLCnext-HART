#include <iostream>
#include <string.h>
#include <cinttypes>
#include "lib/nettools.hpp"
#include "lib/socket.hpp"
#include "lib/hart_mux.hpp"
#include "lib/data_types.hpp"
#include "lib/udp_gateway_discovery.hpp"
#include "lib/cpp-httplib/httplib.h"
#include <fstream>

using namespace std;
using httplib::Server;
using httplib::Request;
using httplib::Response;
using httplib::DataSink;

// NOTE: this must be compiled with the -pthread flag because the httpserver is multithreaded

string BCAST_ADDR = "192.168.1.255";

const string UNIVERSAL_CMD_DEF_DIR = "cmd-definitions/universal";
json JSON_SUPPORTED_HART_CMDS = json::array();

json gatewaysCache = json::array();

json find_gw_by_sn(string bcast_addr, string serialNo) {
    for (int i=0; i<gatewaysCache.size(); i++) {
        if (gatewaysCache[i]["serialNo"] == serialNo) {
            return gatewaysCache[i];
        }
    }
    
    gatewaysCache = discoverGWs(bcast_addr);
    for (int i=0; i<gatewaysCache.size(); i++) {
        if (gatewaysCache[i]["serialNo"] == serialNo) {
            return gatewaysCache[i];
        }
    }
    return NULL;
}

json with_gw_data_or_error(Request req, Response &res, json (*gwDataHandler)(Request, Response, json)) {
    string serialNo(req.matches[1]);
    json gwData = find_gw_by_sn(BCAST_ADDR, serialNo);
    if (gwData != NULL) {
        return gwDataHandler(req, res, gwData);
    } else {
        return json::object({
            {"error", {
                {"status", res.status = 404},
                {"message", "gateway with SN " + serialNo + " was not found"}
            }}
        });
    }
}

int main(int argc, char *argv[]) {
    // load supported HART commands
    cout << "loading supported HART commands" << endl;
    for (int cmd=0; cmd<50; cmd++) {
        json cmdDef;
        string configFile = UNIVERSAL_CMD_DEF_DIR + "/" + std::to_string(cmd) + ".json";
        std::ifstream f(configFile.c_str());
        if (!f) continue;
        f >> cmdDef;
        f.close();
        JSON_SUPPORTED_HART_CMDS[JSON_SUPPORTED_HART_CMDS.size()] = {
            {"number", cmd},
            {"description", cmdDef["description"]}
        };
    }

    // periodically log the HART Gateway's subdevice variables
    cout << "starting logger" << endl;
    gatewaysCache = discoverGWs(BCAST_ADDR);
    thread loggingThread([&]() {
        while (true) {
            for (int i=0; i<gatewaysCache.size(); i++) {
                HartMux hart_mux(gatewaysCache[i]["ip"]);
                hart_mux.initSession();
                hart_mux.autodiscoverSubDevices();

                string sn = gatewaysCache[i]["serialNo"];
                hart_mux.logVars("data/"+sn);
                hart_mux.closeSession();
            }
            sleep(60);
        }
    });
    loggingThread.detach();

    /***
     * webserver on port 5900
     * library: https://github.com/yhirose/cpp-httplib
     */

    Server s;
    s.Get("/", [](const Request& req, Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content("This is the beginnings of a HART MUX web API!\n\n\
        Routes:\n\
        GET\t/\t\t\t\t\twelcome page\n\
        GET\t/gw/discover\t\t\t\tdiscover all gateways on the network\n\
        GET\t/gw/{serialNo}/info\t\t\tHART MUX info (including connected devices)\n\
        GET\t/gw/{serialNo}/info/{card}/{ch}\t\tread device info\n\
        GET\t/gw/{serialNo}/vars\t\t\tget list of all devices w/ their vars\n\
        GET\t/gw/{serialNo}/vars/{ioCard}\t\tget list of all devices on this io card w/ their vars\n\
        GET\t/gw/{serialNo}/vars/{card}/{ch}\t\tread vars from device\n\
        GET\t/gw/{serialNo}/subdevice/{ioCard}/{channel}/cmd/{cmd}\n", "text/plain");
    });

    s.Get("/gw/discover", [](const Request& req, Response& res) {
        json gwData = discoverGWs(BCAST_ADDR);
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(gwData.dump(), "text/json");
    });

    // GET /gw/{serialNo}/info
    s.Get(R"(/gw/(\d+)/info)", [](const Request& req, Response& res) {        
        json data = with_gw_data_or_error(req, res, [](Request req, Response res, json gwData) {
            HartMux hart_mux(gwData["ip"]);
            hart_mux.initSession();
            hart_mux.autodiscoverSubDevices();
            hart_mux.closeSession();

            return hart_mux.to_json();
        });

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(data.dump(), "text/json");
    });

    // GET /gw/{serialNo}/info/{ioCard}/{channel}
    s.Get(R"(/gw/(\d+)/info/(\d+)/(\d+))", [](const Request& req, Response& res) {      
        json data = with_gw_data_or_error(req, res, [](Request req, Response res, json gwData) {
            string ioCard(req.matches[2]);
            string channel(req.matches[3]);

            HartMux hart_mux(gwData["ip"]);
            hart_mux.initSession();
            hart_mux.autodiscoverSubDevices();
            HartDevice dev;
            for (int i=0; i<hart_mux.ioCapabilities.numConnectedDevices; i++) {
                dev = hart_mux.devices[i];
                if (dev.ioCard == (uint8_t)stoi(ioCard) && dev.channel == (uint8_t)stoi(channel)) break;
            }
            hart_mux.closeSession();

            return dev.to_json();
        });

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(data.dump(), "text/json");
    });

    // GET /gw/{serialNo}/vars
    s.Get(R"(/gw/(\d+)/vars)", [](const Request& req, Response& res) {
        json data = with_gw_data_or_error(req, res, [](Request req, Response res, json gwData) {
            HartMux hart_mux(gwData["ip"]);
            hart_mux.initSession();
            hart_mux.autodiscoverSubDevices();

            json data = json::object();
            data["devices"] = json::array();
            for (int i=0; i<hart_mux.ioCapabilities.numConnectedDevices; i++) {
                HartDevice dev = hart_mux.devices[i];
                cout << i+1 << "/" << hart_mux.ioCapabilities.numConnectedDevices << ": name=" << dev.name << "\taddr="; printBytes(dev.addrUniq, 5);
                long addr = 0;
                for (int i=0; i<sizeof(dev.addrUniq); i++) {
                    addr += dev.addrUniq[i];
                }
                if (addr == 0) { // ISSUE: this workaround prevents hanging when the device address is null
                    continue;
                }
                json vars = to_json(hart_mux.readSubDeviceVars(dev));
                data["devices"][i] = dev.to_json();
                data["devices"][i]["vars"] = vars;
            }

            hart_mux.closeSession();
            return data;
        });

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(data.dump(), "text/json");
    });

    // GET /gw/{serialNo}/vars/{ioCard}
    s.Get(R"(/gw/(\d+)/vars/(\d+))", [](const Request& req, Response& res) {
        json data = with_gw_data_or_error(req, res, [](Request req, Response res, json gwData) {
            string ioCard(req.matches[2]);

            HartMux hart_mux(gwData["ip"]);
            hart_mux.initSession();
            hart_mux.autodiscoverSubDevices();

            json data = json::object();
            data["devices"] = json::array();
            int count = 0;
            for (int i=0; i<hart_mux.ioCapabilities.numConnectedDevices; i++) {
                HartDevice dev = hart_mux.devices[i];
                // cout << i+1 << "/" << hart_mux.ioCapabilities.numConnectedDevices << ": name=" << dev.name << "\taddr="; printBytes(dev.addrUniq, 5);
                long addr = 0;
                for (int i=0; i<sizeof(dev.addrUniq); i++) {
                    addr += dev.addrUniq[i];
                }
                if (addr == 0) { // ISSUE: this workaround prevents hanging when the device address is null
                    continue;
                }
                if (dev.ioCard == (uint8_t)stoi(ioCard)) {
                    json vars = to_json(hart_mux.readSubDeviceVars(dev));
                    data["devices"][count] = dev.to_json();
                    data["devices"][count++]["vars"] = vars;
                }
            }

            hart_mux.closeSession();
            return data;
        });
        
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(data.dump(), "text/json");
    });

    // GET /gw/{serialNo}/vars/{ioCard}/{channel}
    s.Get(R"(/gw/(\d+)/vars/(\d+)/(\d+))", [](const Request& req, Response& res) {
        json data = with_gw_data_or_error(req, res, [](Request req, Response res, json gwData) {
            string ioCard(req.matches[2]);
            string channel(req.matches[3]);

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
            return to_json(var_set);
        });

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(data.dump(), "text/json");
    });

    // GET /gw/{serialNo}/log/{ioCard}/{channel}
    s.Get(R"(/gw/(\d+)/log/(\d+)/(\d+))", [](const Request& req, Response& res) {
        json data = with_gw_data_or_error(req, res, [](Request req, Response res, json gwData) {
            string serialNo(req.matches[1]);
            string ioCard(req.matches[2]);
            string channel(req.matches[3]);

            HartMux hart_mux(gwData["ip"]);
            hart_mux.initSession();
            hart_mux.autodiscoverSubDevices();
            json logData = hart_mux.getLogData("data/"+serialNo, stoi(ioCard), stoi(channel));
            hart_mux.closeSession();
            return logData;
        });

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(data.dump(), "text/json");
    });

    // GET /gw/{serialNo}/subdevice/{ioCard}/{channel}/cmd/{cmd}
    s.Get(R"(/gw/(\d+)/subdevice/(\d+)/(\d+)/cmd/(\d+))", [](const Request& req, Response& res) {
        // cout << "GET /gw/{serialNo}/subdevice/{ioCard}/{channel}/cmd/{cmd}" << endl;
        json data = with_gw_data_or_error(req, res, [](Request req, Response res, json gwData) {
            string ioCard(req.matches[2]);
            string channel(req.matches[3]);
            int cmd = stoi(req.matches[4]);

            HartMux hart_mux(gwData["ip"]);
            hart_mux.initSession();
            hart_mux.autodiscoverSubDevices();
            HartDevice dev;
            for (int i=0; i<hart_mux.ioCapabilities.numConnectedDevices; i++) {
                dev = hart_mux.devices[i];
                if (dev.ioCard == (uint8_t)stoi(ioCard) && dev.channel == (uint8_t)stoi(channel)) break;
            }

            uint8_t buf[512];
            size_t cnt;
            uint8_t status;
            hart_mux.sendSubDeviceCmd(cmd, dev, nullptr, 0, buf, cnt, status);
            hart_mux.closeSession();

            json result;
            try {
                result = cmdRsponseToJson(cmd, buf, cnt, status);
            } catch (CmdDefNotFound e) {
                result = {
                    {"status", res.status = 400},
                    {"message", "Command not found. No JSON definition file?"}
                };
            }

            return result;
        });
        
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(data.dump(), "text/json");
    });

    // GET /gw/{serialNo}/subdevice/{ioCard}/{channel}/commands
    s.Get(R"(/gw/(\d+)/subdevice/(\d+)/(\d+)/commands)", [](const Request& req, Response& res) {
        // cout << "GET /gw/{serialNo}/subdevice/{ioCard}/{channel}/commands" << endl;
        json data = with_gw_data_or_error(req, res, [](Request req, Response res, json gwData) {
            string ioCard(req.matches[2]);
            string channel(req.matches[3]);

            return JSON_SUPPORTED_HART_CMDS;
        });
        
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(data.dump(), "text/json");
    });

    string domain = "0.0.0.0";
    unsigned int port = 5900;
    cout << "starting HART IP server at "+domain+":"<<port<<endl;
    s.listen(domain.c_str(), port);
    // end webserver

    return 0;
}