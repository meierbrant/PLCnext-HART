#include "hart_mux.hpp"
#include "hart_devices.hpp"
#include "csv-parser/parser.hpp"

#include "nettools.hpp"
#include "nlohmann/json.hpp"
#include <string.h>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>

// #define DEBUG

using nlohmann::json;
using std::to_string;
using std::ifstream;
using std::ofstream;
using aria::csv::CsvParser;

const string CMD_DEFINITIONS_DIR = "cmd-definitions";

// Constructor defined in header

int HartMux::initSession() {
    #ifdef DEBUG
    cout << "initSession()" << endl;
    #endif
    sock.connect();
    memset(&request, 0, sizeof(hart_ip_pkt_t));

    /* Send Request */
    request.header.version = 1;
    request.header.msgType = 0; // 0: request
    request.header.msgId = 0;   // 0: session init
    request.header.status = 0;  // 0: init to 0 by client
    request.header.seqNo = 1;   // sequence init start value
    request.header.byteCount = 13; // 13 bytes of whole frame for init session telegram

    uint8_t body[request.header.byteCount-sizeof(request.header)];
    memset(body, 0, sizeof(body));
    request.body = body;
    uint8_t initSessionMaster = 1; // 1: primary master
    body[0] = initSessionMaster;
    serialize<uint32_t>(inactivityTimeout, &body[1]);

    sendData(body, sizeof(body));

    uint8_t data[13];
    recvData(data);

    getUniqueAddr();

    return 0;
}

int HartMux::closeSession() {
    #ifdef DEBUG
    cout << "closeSession()" << endl;
    #endif
    request.header.version = 1;
    request.header.msgType = 0; // 0: request
    request.header.msgId = 1;   // 0: session close
    request.header.status = 0;  // 0: init to 0 by client
    request.header.byteCount = 8;

    sendHeader();

    uint8_t data[8];
    recvData(data);

    if (response.header.status == 0) sock.close();

    return response.header.status;
}

uint8_t* HartMux::getUniqueAddr() { // cmd 0 by short address for gateway
    #ifdef DEBUG
    cout << "getUniqAddr()" << endl;
    #endif

    sendCmd(0, (uint8_t)0);
    // PDU frame up to data bytes should be 6 bytes
    
    uint16_t deviceTypeCode = (response.body[7] << 8) | response.body[8];
    setTypeInfo(deviceTypeCode);

    // FIXME: to resolve the weird issue that setTypeCode wipes out the response.body,
    // temporarily send cmd 0 again to regain that info.
    sendCmd(0, (uint8_t)0);

    addrUniq[0] = response.body[6+1];
    addrUniq[1] = response.body[6+2];
    memcpy(&addrUniq[2], &response.body[6+9], 3);

    extendedDeviceStatusBits = response.body[6+16];

    #ifdef DEBUG
    cout << "addrUniq = ";
    printBytes(addrUniq, 5);
    #endif

    return (uint8_t *)addrUniq;
}

void HartMux::readIOSystemCapabilities() { // cmd 74
    #ifdef DEBUG
    cout << "readIOSystemCapabilities()" << endl;
    #endif

    sendCmd(74, addrUniq);
    ioCapabilities.maxIoCards = response.body[10];
    ioCapabilities.maxChannels = response.body[11];
    ioCapabilities.subDevicesPerCh = response.body[12];
    ioCapabilities.numConnectedDevices = ((response.body[13] << 8) | response.body[14]) - 1; // exclude itself
    ioCapabilities.numDelayedResp = response.body[15];
    ioCapabilities.masterMode = response.body[16];
    ioCapabilities.retryCnt = response.body[17];
}

HartDevice HartMux::readSubDeviceSummary(uint16_t index) {
    #ifdef DEBUG
    cout << "readSubDeviceSummary() " << index << "/" << ioCapabilities.numConnectedDevices << " devices" << endl;
    #endif
    uint8_t data[2];
    serialize(index, data);
    sendCmd(84, addrUniq, data, 2);
    
    size_t pduHdrSize = 10;
    uint16_t deviceTypeCode = (response.body[pduHdrSize+6] << 8) | response.body[pduHdrSize+7];
    HartDevice d(deviceTypeCode);

    // FIXME: to resolve the weird issue that setTypeCode wipes out the response.body,
    // temporarily send cmd 84 again to regain that info.
    sendCmd(84, addrUniq, data, 2);

    d.hart_mux = this;
    memcpy(d.addrUniq, &response.body[pduHdrSize+6], 5);
    d.ioCard = response.body[pduHdrSize+2];
    d.channel = response.body[pduHdrSize+3];
    d.manufacturerId = (response.body[pduHdrSize+4] << 8) | response.body[pduHdrSize+5];
    d.cmdRevLvl = response.body[pduHdrSize+11];
    char longTag[33];
    memcpy(longTag, &response.body[pduHdrSize+12], 32);
    longTag[32] = 0x00;
    d.longTag = string(longTag);
    d.revision = response.body[pduHdrSize+44];
    d.profile = response.body[pduHdrSize+45];
    d.pvtLabelDistCode = (response.body[pduHdrSize+46] << 8) | response.body[pduHdrSize+47];

    uint8_t buf[512];
    size_t count;
    uint8_t status;
    sendSubDeviceCmd(0, d, nullptr, 0, buf, count, status);
    if (count > 16) // extended device status not returned for older HART versions
        d.extendedDeviceStatusBits = buf[16];

    return d;
}

void HartMux::autodiscoverSubDevices() {
    readIOSystemCapabilities();
    for (int i=0; i<ioCapabilities.numConnectedDevices; i++) {
        devices[i] = readSubDeviceSummary(i+1);
    }
}

void HartMux::listDevices() {
    int n = ioCapabilities.numConnectedDevices;
    cout << "HART MUX has (" << n << ") devices connected:" << endl;
    for (int i=0; i<n; i++) {
        devices[i].print();
        hart_var_set vars = readSubDeviceVars(devices[i]);
        displayVars(vars);
        cout << endl;
    }
}

// Goes to the HART MUX to get the vars
hart_var_set HartMux::readSubDeviceVars(HartDevice dev) {
    #ifdef DEBUG
    cout << "readSubDeviceVars()" << endl;
    #endif

    hart_pdu_frame innerFrame = buildPduFrame(dev.addrUniq, 3);
    innerFrame.addr[0] = innerFrame.addr[0] & 0x3f | 0x80; // long poll address
    uint8_t innerData[255];
    size_t bCnt = 0;
    innerData[bCnt++] = dev.ioCard;
    innerData[bCnt++] = dev.channel;
    innerData[bCnt++] = 5; // transmit preamble countserialize(innerFrame, innerData);
    bCnt += serialize(innerFrame, &innerData[bCnt]);
    hart_pdu_frame outerFrame = buildPduFrame(addrUniq, 77, innerData, bCnt);

    sendPduFrame(outerFrame);
    hart_pdu_frame f = recvPduFrame();
    while (f.byteCnt == 2) { // got a bad response
        sendPduFrame(outerFrame);
        f = recvPduFrame();
    }

    hart_pdu_frame f2 = deserializeHartPduFrame(&f.data[4]); // 4 is start of inner PDU frame

    hart_var_set vars = deserializeHartVarSet(&f2.data[2], f2.byteCnt-2);
    return vars;
}

string getLastLineOfFile(string filepath) {
    std::string line = "";
    std::ifstream f(filepath);

    if (f.is_open()) {
        f.seekg(0, std::ios_base::end);
        char c = ' ';
        while(c != '\n'){
            f.seekg(-2, std::ios_base::cur);
            if ((int)f.tellg() <= 0) {
                f.seekg(0);
                break;
            }
            f.get(c);
        }

        std::getline(f,line);
        f.close();

        return line;
    } else {
        throw std::exception();
    }
}

hart_var_set HartMux::getSubDeviceVars(HartDevice dev) {
    // cout << "getSubDeviceVars()" << endl;

    int n = ioCapabilities.numConnectedDevices;
    cout << "n: " << n << endl;
    for (int i=0; i<n; i++) {
        hart_var_set vars = readSubDeviceVars(devices[i]);
    }
}

void HartMux::logVars(string dir) {
    // append the current values to the file
    int n = ioCapabilities.numConnectedDevices;
    for (int i=0; i<n; i++) {
        string path = dir+'/'+to_string(devices[i].ioCard)+'/'+to_string(devices[i].channel)+'/';
        string mkdir = "mkdir -p " + path;
        string logFile1 = "vars.log.1";
        string logFile2 = "vars.log.2";
        string activeLogFile = path + "vars.log";
        system(mkdir.c_str());
        hart_var_set vars = readSubDeviceVars(devices[i]);

        // ensure fs setup
        ifstream l1r(path+logFile1);
        if (!l1r.is_open()) ofstream l1w(path+logFile1);
        ifstream l2r(path+logFile2);
        if (!l2r) ofstream l2w(path+logFile2);
        struct stat lstatus;
        if (lstat(activeLogFile.c_str(), &lstatus) < 0) {
            symlink(logFile1.c_str(), activeLogFile.c_str());
        }

        // read the file size
        struct stat fstatus;
        stat(activeLogFile.c_str(), &fstatus);

        int lineSizeEstimate = (8+1+5)*4 + 24;
        int remainingSpace = maxLogfileSize - fstatus.st_size;

        if (remainingSpace < lineSizeEstimate) { // rotate the symlink to the older log file & erase
            string nextlog = logFile1;
            if (string(realpath(activeLogFile.c_str(),NULL)).compare(realpath(string(path+logFile1).c_str(),NULL)) == 0) nextlog = logFile2;
            remove(activeLogFile.c_str());
            symlink(nextlog.c_str(), activeLogFile.c_str());
            ofstream f(realpath(activeLogFile.c_str(),NULL));
            f.close(); // close having written an empty file
        }

        // append the new vars
        ofstream log(realpath(activeLogFile.c_str(),NULL), std::ios::app);
        
        log << vars.pv.value << ',';
        log << vars.pv.units << ',';

        log << vars.sv.value << ',';
        log << vars.sv.units << ',';

        log << vars.tv.value << ',';
        log << vars.tv.units << ',';

        log << vars.qv.value << ',';
        log << vars.qv.units << ',';

        time_t now = time(0);
        log << ctime(&now);
        log.close();
    }
}

json parseVarLogfile(string filepath, json data, size_t offset=0) {
    ifstream file(filepath);
    if (!file) return data;
    CsvParser parser(file);

    int r = offset;
    int c = 0;
    for (auto& row : parser) {
        for (auto& field : row) {
            switch (c) {
                case 0:
                    data["pv"][r]["value"] = stof(field);
                    break;
                case 1:
                    data["pv"][r]["units"] = field;
                    break;
                case 2:
                    data["sv"][r]["value"] = stof(field);
                    break;
                case 3:
                    data["sv"][r]["units"] = field;
                    break;
                case 4:
                    data["tv"][r]["value"] = stof(field);
                    break;
                case 5:
                    data["tv"][r]["units"] = field;
                    break;
                case 6:
                    data["qv"][r]["value"] = stof(field);
                    break;
                case 7:
                    data["qv"][r]["units"] = field;
                    break;
                case 8:
                    data["pv"][r]["timestamp"] = field;
                    data["sv"][r]["timestamp"] = field;
                    data["tv"][r]["timestamp"] = field;
                    data["qv"][r]["timestamp"] = field;
            }
            c = (c + 1) % 9;
        }
        r++;
    }

    return data;
}

json HartMux::getLogData(string dir, int ioCard, int channel) {
    string path = dir+'/'+to_string(ioCard)+'/'+to_string(channel)+'/';
    string logFile1path = path+"vars.log.1";
    string logFile2path = path+"vars.log.2";
    string activeLogFilePath = path + "vars.log";
    
    json data = {
        {"pv", json::array()},
        {"sv", json::array()},
        {"tv", json::array()},
        {"qv", json::array()}
    };

    if (realpath(activeLogFilePath.c_str(),NULL) == NULL) return data;

    // determine load order
    if (string(realpath(activeLogFilePath.c_str(),NULL)).compare(realpath(string(logFile1path).c_str(),NULL)) == 0) {
        data = parseVarLogfile(logFile2path, data);
        data = parseVarLogfile(logFile1path, data, data["pv"].size());
    } else {
        data = parseVarLogfile(logFile1path, data);
        data = parseVarLogfile(logFile2path, data, data["pv"].size());
    }
    
    return data;
}


/**
 * pollAddr is zero by default since drop networks are not supported yet
 */
int HartMux::sendCmd(unsigned char cmd, uint8_t pollAddr) {
    size_t byteCount = 0;
    uint8_t data[512];
    memset(data, 0, sizeof(data));
    if (cmd == 0) {
        hart_pdu_frame f;
        hart_pdu_delimiter d;
        d.frameType = hart_pdu_delimiter::STX;
        d.physicalLayerType = hart_pdu_delimiter::ASYNC;
        d.numExpansionBytes = 0;
        d.addressType = hart_pdu_delimiter::POLLING;
        f.delimiter = d;
        uint8_t del;
        memcpy(&del, &d, 1);
        f.addr[0] = pollAddr;
        f.cmd = cmd;
        f.byteCnt = 0;
        byteCount += serialize(f, data);
    } else {
        cout << "Error: short address only allowed for cmd 0" << endl;
        return -1;
    }

    request.header.version = 1;
    request.header.msgType = 0; // 0: Request
    request.header.msgId = 3;   // 3: Token-Passing PDU
    request.header.status = 0;  // 0: init to 0 by client
    
    sendData(data, byteCount);

    uint8_t buf[512];
    memset(buf, 0, 512);
    
    return recvData(buf);
}

/**
 * param uniqueAddr is 5 bytes
 */
int HartMux::sendCmd(unsigned char cmd, uint8_t *uniqueAddr, uint8_t *reqData, size_t reqDataCnt) {
    hart_pdu_frame f = buildPduFrame(uniqueAddr, cmd, reqData, reqDataCnt);

    sendPduFrame(f);

    uint8_t buf[512];
    memset(buf, 0, 512);
    
    return recvData(buf);
}

void HartMux::sendCmd(unsigned char cmd, uint8_t *uniqueAddr, uint8_t *reqData, size_t reqDataCnt, uint8_t *resData, size_t &resDataCnt) {
    hart_pdu_frame f = buildPduFrame(uniqueAddr, cmd, reqData, reqDataCnt);

    sendPduFrame(f);

    memset(resData, 0, 512);
    resDataCnt = recvData(resData);
}

void HartMux::sendSubDeviceCmd(unsigned char cmd, HartDevice dev, uint8_t *reqData, size_t reqDataCnt, uint8_t *resData, size_t &resDataCnt, uint8_t &status) {
    #ifdef DEBUG
    cout << "sendSubDeviceCmd()" << endl;
    #endif

    hart_pdu_frame innerFrame = buildPduFrame(dev.addrUniq, cmd, reqData, reqDataCnt);
    innerFrame.addr[0] = innerFrame.addr[0] & 0x3f | 0x80; // long poll address
    uint8_t innerData[255];
    size_t bCnt = 0;
    innerData[bCnt++] = dev.ioCard;
    innerData[bCnt++] = dev.channel;
    innerData[bCnt++] = 5; // transmit preamble countserialize(innerFrame, innerData);
    bCnt += serialize(innerFrame, &innerData[bCnt]);
    hart_pdu_frame outerFrame = buildPduFrame(addrUniq, 77, innerData, bCnt);

    sendPduFrame(outerFrame);
    hart_pdu_frame f = recvPduFrame();
    while (f.byteCnt == 2) { // got a bad response
        sendPduFrame(outerFrame);
        f = recvPduFrame();
    }

    hart_pdu_frame f2 = deserializeHartPduFrame(&f.data[4]); // 4 is start of inner PDU frame
    #ifdef DEBUG
    printf("longTag: %s\n", dev.longTag.c_str());
    printf("inner PDU frame:\tcmd: %i\tbyteCnt: %i\taddr: ", f2.cmd, f2.byteCnt); printBytes(f2.addr, sizeof(f2.addr));
    printf("\t data: \n"); printBytes(&f2.data[2], f2.byteCnt-2);
    #endif
    
    status = response.header.status;
    resDataCnt = f2.byteCnt-2;
    memcpy(resData, &f2.data[2], resDataCnt);
}

int HartMux::send(uint8_t *bytes, size_t len, int flags) {
    int r = sock.send(bytes, len, flags);
    if (r < 0) {
        perror("Error recieving packet header from HART MUX");
        return -1;
    } else if (r == 0) {
        perror("The HART MUX hung up");
        return -1;
    }
}

int HartMux::recv(uint8_t *buf, size_t len, int flags) {
    int r = sock.recv(buf, len, flags);
    if (r < 0) {
        perror("Error recieving packet header from HART MUX");
    } else if (r == 0) {
        cout << "The HART MUX hung up" << endl;
    }
    return r;
}

int HartMux::sendHeader() {
    uint8_t bytes[request.header.byteCount];
    memset(bytes, 0, sizeof(bytes));
    serialize(request.header, bytes);
    send(bytes, request.header.byteCount, 0);
}

int HartMux::sendData(uint8_t *data, size_t len) {
    // send packet to MUX
    request.header.byteCount = len + sizeof(hart_ip_hdr_t);
    uint8_t bytes[request.header.byteCount];
    memset(bytes, 0, sizeof(bytes));
    serialize(request.header, bytes);
    
    memcpy(&bytes[sizeof(hart_ip_hdr_t)], data, len);
    send(bytes, request.header.byteCount, 0);
    #ifdef DEBUG
    cout << "sent packet:" << endl;
    printBytes(bytes, request.header.byteCount);
    cout << endl;
    #endif
}

int HartMux::recvData(uint8_t *buf) {
    // get response header from MUX
    uint8_t hdr_buf[sizeof(hart_ip_hdr_t)];
    memset(hdr_buf, 0, sizeof(hdr_buf));
    recv(hdr_buf, sizeof(hdr_buf), 0);
    response.header = deserializeHartIpHdr(hdr_buf);
    #ifdef DEBUG
    cout << "received header (network byte order) (expecting " << response.header.byteCount - sizeof(hart_ip_hdr_t) << " more bytes):" << endl;
    printBytes(hdr_buf, sizeof(hart_ip_hdr_t));
    #endif

    if ((response.header.seqNo != request.header.seqNo) || response.header.status > 0) {
        cout << hartStatusCodetoString(response.header.msgId, response.header.status) << endl;
        err = true;
        return -1;
    } else {
        if (request.header.seqNo > 0xfffe) {
            request.header.seqNo = 1;
        } else {
            request.header.seqNo++;
        }        
    }

    int r = 0;
    // ignore obviously corrupt transmissions
    if (response.header.byteCount < 65536) {
        
    
        // get rest of response from MUX
        uint8_t bdy_buf[response.header.byteCount - sizeof(hart_ip_hdr_t)];
        memset(buf, 0, sizeof(bdy_buf));
        if (sizeof(bdy_buf) > 0) {
            r = recv(buf, sizeof(bdy_buf), 0);
            response.body = buf;
            //*buf = bdy_buf;
            #ifdef DEBUG
            cout << "received data (network byte order):" << endl;
            printBytes(response.body, sizeof(bdy_buf));
            cout << endl;
            #endif
        }
    }

    return r;
}

int HartMux::sendPduFrame(hart_pdu_frame frame) {
    uint8_t data[512];
    size_t byteCount = serialize(frame, data);

    request.header.version = 1;
    request.header.msgType = 0; // 0: Request
    request.header.msgId = 3;   // 3: Token-Passing PDU
    request.header.status = 0;  // 0: init to 0 by client
    sendData(data, byteCount);
}

hart_pdu_frame HartMux::recvPduFrame() {
    uint8_t buf[512];
    recvData(buf);
    hart_pdu_frame f = deserializeHartPduFrame(buf);
    return f;
}

void autodiscoverLoop(HartMux *mux, int seconds) {
    while (!mux->stopAutodiscovery) {
        mux->autodiscoverSubDevices();
        sleep(seconds);
    }
}

json HartMux::to_json() {
    json data = HartDevice::to_json();

    data["ipAddress"] = ipAddress;
    data["devices"] = json::array();
    int n = ioCapabilities.numConnectedDevices;
    for (int i=0; i<n; i++) {
        data["devices"][i] = devices[i].to_json();
    }

    return data;
}

json cmdRsponseToJson(int cmd, uint8_t *data, int len, uint8_t status) {
    json r;
    json cmdDef;
    string configFile = CMD_DEFINITIONS_DIR + "/universal/" + std::to_string(cmd) + ".json";
    std::ifstream f(configFile.c_str());
    if (!f) throw CmdDefNotFound();
    f >> cmdDef;
    f.close();

    r["number"] = cmdDef["number"];
    r["description"] = cmdDef["description"];
    r["status"] = cmdDef["responseCodes"][std::to_string(status)];
    if (r["status"].is_null()) r["status"] = {
        {"class", "error"},
        {"description:", "unknown"}
    };
    r["data"] = json::array();

    for (int i=0; i<cmdDef["responseData"].size(); i++) {
        json fieldDef = cmdDef["responseData"][i];
        json parsedVal;
        // cout << "fieldDef: " << fieldDef.dump() << endl;
        string format(fieldDef["format"]);
        if (fieldDef["byte"].is_string()) { // it's a range of bytes
            string rangestr(fieldDef["byte"]);
            std::stringstream rangess(rangestr);
            std::string segment;
            std::vector<int> range;

            while(std::getline(rangess, segment, '-')) {
                range.push_back(stoi(segment));
            }

            if (range[1]+1 > len) return r; // optional data bytes not present
            parsedVal = parseResponseBytes(&data[range[0]], format, range[1]-range[0]+1);
        } else if (fieldDef["byte"].is_number_unsigned()) { // it's a single byte
            if ((int)fieldDef["byte"]+1 > len) return r; // optional data byte not present
            parsedVal = parseResponseBytes(&data[(int)fieldDef["byte"]], format, 1);
        }

        r["data"][i] = {
            {"byte", fieldDef["byte"]},
            {"format", fieldDef["format"]},
            {"description", fieldDef["description"]},
            {"value", parsedVal}
        };
    }

    return r;
}

json parseResponseBytes(uint8_t *bytes, string format, int count) {
    // printf("parseResponseBytes(format: %s, count: %i, bytes: ", format.c_str(), count); printBytes(bytes, count);
    json val;
    if (format.compare("float") == 0) {
        float f = fromBytes<float>(bytes);
        val = f;
    } else if (format.compare("unsigned-24") == 0) {
        unsigned int i;
        uint24_t u = fromBytes<uint24_t>(bytes);
        memcpy(&i+1, &u, 3);
        val = i;
    } else if (format.compare("bits") == 0) {
        json bitArray = json::array();
        for (int byte=0; byte<count; byte++) {
            for (int bit=0; bit<8; bit++) {
                bitArray[8*byte+bit] = ((bytes[byte] >> bit) & 1) ? 1 : 0;
            }
        }
        val = bitArray;
    } else if (format.compare("latin-1") == 0) {
        bytes[count] = '\0';
        string s((char*)bytes);
        val = s;
    } else {
        val = bytesToHexStr(bytes, count);
    }
    return val;
}