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
using nettools::netif_summary;

// NOTE: this must be compiled with the -pthread flag because the httpserver is multithreaded

string DEFAULT_BCAST_ADDR = "192.168.1.255";
json selected_network = NULL;
json networks;

const string UNIVERSAL_CMD_DEF_DIR = "cmd-definitions/universal";
json JSON_SUPPORTED_HART_CMDS = json::array();

json gatewaysCache = json::array();

json find_gw_by_sn(string bcast_addr, string serialNo) {
    for (int i=0; i<gatewaysCache.size(); i++) {
        if (gatewaysCache[i]["serialNo"] == serialNo) {
            return gatewaysCache[i];
        }
    }
    
    gatewaysCache = discoverGWs(selected_network);
    for (int i=0; i<gatewaysCache.size(); i++) {
        if (gatewaysCache[i]["serialNo"] == serialNo) {
            return gatewaysCache[i];
        }
    }
    return NULL;
}

json with_hart_mux_or_error(Request req, Response &res, json (*gwDataHandler)(Request, Response, HartMux &)) {
    string serialNo(req.matches[1]);
    json gwData = find_gw_by_sn(selected_network["bcast"], serialNo);
    if (gwData != NULL) {
        HartMux hart_mux(gwData["ip"]);
        hart_mux.initSession();
        hart_mux.autodiscoverSubDevices();
        json data = gwDataHandler(req, res, hart_mux);
        hart_mux.closeSession();
        return data;
    } else {
        return json::object({
            {"error", {
                {"status", res.status = 404},
                {"message", "gateway with SN " + serialNo + " was not found"}
            }}
        });
    }
}

json with_subdevice_or_error(Request req, Response &res, HartMux &hart_mux, json (*subdeviceHandler)(Request, Response, HartDevice &)) {
    int ioCard = stoi(req.matches[2]);
    int channel = stoi(req.matches[3]);
    HartDevice dev;
    bool found = false;
    for (int i=0; i<hart_mux.ioCapabilities.numConnectedDevices; i++) {
        dev = hart_mux.devices[i];
        if (dev.ioCard == (uint8_t)ioCard && dev.channel == (uint8_t)channel) {
            found = true;
            break;
        }
    }
    if (found) {
        return subdeviceHandler(req, res, dev);
    } else {
        return json::object({
            {"error", {
                {"status", res.status = 404},
                {"message", "device at ioCard " + to_string(ioCard) + " / channel " + to_string(channel) + " was not found"}
            }}
        });
    }
}

void loadSupportedHartCommands() {
    // cout << "loading supported HART commands" << endl;
    JSON_SUPPORTED_HART_CMDS = json::array();
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
}

json discoverNetworks() {
    struct netif_summary *cur_netif;
    int count;
    nettools::get_feasible_subnets(&cur_netif, &count);
    json nwData = json::array();
    for (int i=0; i<count; i++) {
        nwData[i] = {
            {"name", cur_netif->name},
            {"addr", cur_netif->addr},
            {"netmask", cur_netif->netmask},
            {"network", cur_netif->network},
            {"bcast", cur_netif->bcast}
        };
        if (selected_network == NULL && nwData[i]["bcast"] == DEFAULT_BCAST_ADDR) selected_network = nwData[i];

        cur_netif = cur_netif->next;
    }
    if (selected_network == NULL) selected_network = nwData[0];

    networks = nwData;
    return nwData;
}

int main(int argc, char *argv[]) {
    // init network data
    discoverNetworks();

    // load supported HART commands
    loadSupportedHartCommands();

    // periodically log the HART Gateway's subdevice variables
    cout << "starting logger" << endl;
    gatewaysCache = discoverGWs(selected_network);
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
    discoverNetworks();

    s.Get("/", [](const Request& req, Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content("\
        <style>\
        table tr:nth-child(even) {\
            background-color: #eee;\
        }\
        table tr:nth-child(odd) {\
            background-color: #fff;\
        }\
        </style>\
        \
        <p>This is the beginnings of a HART MUX web API!</p>\
        <table border>\
        <tr><th>Method</th><th>Route</th><th>Description</th></tr>\
        <tr><td>GET</td><td>/</td><td>welcome page</td></tr>\
        <tr><td>GET</td><td>/gw/discover</td><td>discover all gateways on the network</td></tr>\
        <tr><td>GET</td><td>/gw/{serialNo}/info</td><td>HART MUX info (including connected devices)</td></tr>\
        <tr><td>GET</td><td>/gw/{serialNo}/info/{card}/{ch}</td><td>read device info</td></tr>\
        <tr><td>GET</td><td>/gw/{serialNo}/vars</td><td>get list of all devices w/ their vars</td></tr>\
        <tr><td>GET</td><td>/gw/{serialNo}/vars/{ioCard}</td><td>get list of all devices on this io card w/ their vars</td></tr>\
        <tr><td>GET</td><td>/gw/{serialNo}/vars/{card}/{ch}</td><td>read vars from device</td></tr>\
        <tr><td>GET</td><td>/gw/{serialNo}/log/{ioCard}/{channel}</td><td>get the logged vars (pv, sv, tv, qv) for this device</td></tr>\
        <tr><td>GET</td><td>/gw/{serialNo}/subdevice/{ioCard}/{channel}/cmd/{cmd}</td><td>send a command to this device</td></tr>\
        <tr><td>GET</td><td>/gw/{serialNo}/subdevice/{ioCard}/{channel}/commands</td><td>get a list of all supported commands for this device</td></tr>\
        </table>", "text/html");
    });

    // Allow GET, POST to /networks
    s.Options("/networks", [](const Request& req, Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Headers", "content-type");
        res.set_header("Access-Control-Allow-Methods", "GET, POST");
    });

    // GET /networks
    s.Get("/networks", [](const Request& req, Response& res) {
        // cout << "GET /networks/discover" << endl;
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(discoverNetworks().dump(), "text/json");
    });

    // POST /networks
    s.Post("/networks", [&](const Request& req, Response& res) {
        json postData = json::parse(req.body);
        json data = {
            {"status", res.status = 503},
            {"message", "could not find that network"}
        };

        if (postData["network"].is_string()) {
            for (int i=0; i<networks.size(); i++) {
                string ip = networks[i]["network"];
                if (ip.compare(postData["network"]) == 0) {
                    selected_network = networks[i];
                    cout << "new bcast addr: " << selected_network["bcast"] << endl;
                    data["status"] = res.status = 200;
                    data["message"] = "success";
                }
            }
        } else {
            data["status"] = res.status = 400;
            data["message"] = "no network specified";
        }

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(data.dump(), "text/json");
    });

    // GET /gw/discover
    s.Get("/gw/discover", [](const Request& req, Response& res) {
        // cout << "GET /gw/discover" << endl;
        json gwData = discoverGWs(selected_network);

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(gwData.dump(), "text/json");
    });

    // GET /gw/{serialNo}/info
    s.Get(R"(/gw/(\d+)/info)", [](const Request& req, Response& res) {        
        // cout << "GET /gw/{serialNo}/info" << endl;
        json data = with_hart_mux_or_error(req, res, [](Request req, Response res, HartMux &hart_mux) {
            return hart_mux.to_json();
        });

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(data.dump(), "text/json");
    });

    // GET /gw/{serialNo}/info/{ioCard}/{channel}
    s.Get(R"(/gw/(\d+)/info/(\d+)/(\d+))", [](const Request& req, Response& res) {      
        // cout << "GET /gw/{serialNo}/info/{ioCard}/{channel}" << endl;
        json data = with_hart_mux_or_error(req, res, [](Request req, Response res, HartMux &hart_mux) {
            return with_subdevice_or_error(req, res, hart_mux, [](Request req, Response res, HartDevice &hart_dev) {
                return hart_dev.to_json();
            });
        });

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(data.dump(), "text/json");
    });

    // GET /gw/{serialNo}/vars
    s.Get(R"(/gw/(\d+)/vars)", [](const Request& req, Response& res) {
        // cout << "GET /gw/{serialNo}/vars" << endl;
        json data = with_hart_mux_or_error(req, res, [](Request req, Response res, HartMux &hart_mux) {
            json data = json::object();
            data["devices"] = json::array();
            for (int i=0; i<hart_mux.ioCapabilities.numConnectedDevices; i++) {
                HartDevice dev = hart_mux.devices[i];
                cout << i+1 << "/" << hart_mux.ioCapabilities.numConnectedDevices << ": name=" << dev.name << "\taddr="; printBytes(dev.addrUniq, 5);
                json vars = to_json(hart_mux.readSubDeviceVars(dev));
                data["devices"][i] = dev.to_json();
                data["devices"][i]["vars"] = vars;
            }
            return data;
        });

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(data.dump(), "text/json");
    });

    // GET /gw/{serialNo}/vars/{ioCard}
    s.Get(R"(/gw/(\d+)/vars/(\d+))", [](const Request& req, Response& res) {
        // cout << "GET /gw/{serialNo}/vars/{ioCard}" << endl;
        json data = with_hart_mux_or_error(req, res, [](Request req, Response res, HartMux &hart_mux) {
            string ioCard(req.matches[2]);

            json data = json::object();
            data["devices"] = json::array();
            int count = 0;
            for (int i=0; i<hart_mux.ioCapabilities.numConnectedDevices; i++) {
                HartDevice dev = hart_mux.devices[i];
                // cout << i+1 << "/" << hart_mux.ioCapabilities.numConnectedDevices << ": name=" << dev.name << "\taddr="; printBytes(dev.addrUniq, 5);
                if (dev.ioCard == (uint8_t)stoi(ioCard)) {
                    json vars = to_json(hart_mux.readSubDeviceVars(dev));
                    data["devices"][count] = dev.to_json();
                    data["devices"][count++]["vars"] = vars;
                }
            }

            return data;
        });
        
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(data.dump(), "text/json");
    });

    // GET /gw/{serialNo}/vars/{ioCard}/{channel}
    s.Get(R"(/gw/(\d+)/vars/(\d+)/(\d+))", [](const Request& req, Response& res) {
        // cout << "GET /gw/{serialNo}/vars/{ioCard}/{channel}" << endl;
        json data = with_hart_mux_or_error(req, res, [](Request req, Response res, HartMux &hart_mux) {
            return with_subdevice_or_error(req, res, hart_mux, [](Request req, Response res, HartDevice &hart_dev) {
                hart_var_set var_set = hart_dev.readVars();
                return to_json(var_set);
            });
        });

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(data.dump(), "text/json");
    });

    // GET /gw/{serialNo}/log/{ioCard}/{channel}
    s.Get(R"(/gw/(\d+)/log/(\d+)/(\d+))", [](const Request& req, Response& res) {
        // cout << "GET /gw/{serialNo}/log/{ioCard}/{channel}" << endl;
        json data = with_hart_mux_or_error(req, res, [](Request req, Response res, HartMux &hart_mux) {
            string serialNo(req.matches[1]);
            string ioCard(req.matches[2]);
            string channel(req.matches[3]);

            json logData = hart_mux.getLogData("data/"+serialNo, stoi(ioCard), stoi(channel));
            return logData;
        });

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(data.dump(), "text/json");
    });

    // GET /gw/{serialNo}/subdevice/{ioCard}/{channel}/cmd/{cmd}
    s.Get(R"(/gw/(\d+)/subdevice/(\d+)/(\d+)/cmd/(\d+))", [](const Request& req, Response& res) {
        // cout << "GET /gw/{serialNo}/subdevice/{ioCard}/{channel}/cmd/{cmd}" << endl;
        json data = with_hart_mux_or_error(req, res, [](Request req, Response res, HartMux &hart_mux) {
            return with_subdevice_or_error(req, res, hart_mux, [](Request req, Response res, HartDevice &hart_dev) {
                int cmd = stoi(req.matches[4]);

                uint8_t buf[512];
                size_t cnt;
                uint8_t status;
                hart_dev.sendCmd(cmd, nullptr, 0, buf, cnt, status);

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
        });
        
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(data.dump(), "text/json");
    });

    // GET /gw/{serialNo}/subdevice/{ioCard}/{channel}/commands
    s.Get(R"(/gw/(\d+)/subdevice/(\d+)/(\d+)/commands)", [](const Request& req, Response& res) {
        // cout << "GET /gw/{serialNo}/subdevice/{ioCard}/{channel}/commands" << endl;
        json data = with_hart_mux_or_error(req, res, [](Request req, Response res, HartMux &hart_mux) {
            return with_subdevice_or_error(req, res, hart_mux, [](Request req, Response res, HartDevice &hart_dev) {
                loadSupportedHartCommands();
                return JSON_SUPPORTED_HART_CMDS;
            });
        });
        
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(data.dump(), "text/json");
    });

    // GET catchall -> render 404
    s.Get(R"(/(.+))", [](const Request& req, Response& res) {
        json data = {
            {"status", res.status = 404},
            {"message", "route not found"}
        };
        
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(data.dump(), "text/json");
    });

    string domain = "0.0.0.0";
    unsigned int port = 5900;
    cout << "starting HART IP server at "+domain+":"<<port<<endl;
    cout << "initial selected_network: " << selected_network.dump() << endl;
    s.listen(domain.c_str(), port);
    // end webserver

    return 0;
}