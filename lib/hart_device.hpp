#pragma once

#include <cinttypes>
#include <iostream>

using std::string;

class HartDevice {
public:
    uint8_t addrUniq[5];
    uint16_t typeCode;
    // ISSUE: for some reason, strings with more than 15 characters cause addrUniq to get wiped out
    // unless we initialize this with a lot of characters. The theoretical limit of the string data type
    // should be practically infinite
    string name = "unknown";
    string company = "unknown";
    uint8_t ioCard;
    uint8_t channel;
    uint16_t manufacturerId;
    uint8_t cmdRevLvl;
    string longTag;
    uint8_t revision;
    uint8_t profile;
    uint16_t pvtLabelDistCode;
    
    HartDevice();
    HartDevice(uint16_t deviceTypeCode);
    void setTypeInfo(uint16_t code);
    void print();

private:

};