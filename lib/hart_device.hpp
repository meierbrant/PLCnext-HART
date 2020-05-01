#pragma once

#include <cinttypes>
#include <iostream>
#include "nlohmann/json.hpp"

using std::string;
using nlohmann::json;

struct hart_var {
    string units;
    float value;
};

struct hart_var_set {
    float loopCurrent; // in milliamps
    hart_var pv;
    hart_var sv;
    hart_var tv;
    hart_var qv;
};

string getUnitsFromCode(uint8_t code);
hart_var_set deserializeHartVarSet(uint8_t *bytes, size_t bCnt);
void displayVars(hart_var_set vars);
json extendedDeviceStatusBitsToJson(uint8_t bits);

class HartMux; // forward declare

class HartDevice {
public:
    HartMux *hart_mux = nullptr;
    uint8_t addrUniq[5];
    uint16_t typeCode;
    uint8_t ioCard;
    uint8_t channel;
    uint16_t manufacturerId;
    uint8_t cmdRevLvl;
    uint8_t revision;
    uint8_t profile;
    uint16_t pvtLabelDistCode;
    string longTag;
    // ISSUE: for some reason, strings with more than 15 characters cause addrUniq to get wiped out
    // unless we initialize this with a lot of characters. The theoretical limit of the string data type
    // should be practically infinite
    string name = "unknown";
    string company = "unknown";
    uint8_t extendedDeviceStatusBits = 0;
    
    HartDevice();
    HartDevice(uint16_t deviceTypeCode);
    void setTypeInfo(uint16_t code);
    hart_var_set readVars();
    void sendCmd(unsigned char cmd, uint8_t *reqData, size_t reqDataCnt, uint8_t *resData, size_t &resDataCnt, uint8_t &status);
    json to_json();
    void print();

private:

};