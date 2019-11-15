#pragma once

#include <cinttypes>
#include <iostream>

using std::string;

class HartDevice {
    public:
    uint8_t addrUniq[5];
    uint16_t typeCode;
    string name;
    string company;

    HartDevice();
    HartDevice(uint16_t deviceTypeCode);
    void setTypeInfo(uint16_t code);
    void print();

    private:

};