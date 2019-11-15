#pragma once

#include <cinttypes>
#include <iostream>

using std::string;

struct hart_device_info {
    uint16_t deviceType;
    string deviceName;
    string deviceCompany;
    uint8_t addrUniq[5];
};

hart_device_info getHartDevInfo(uint16_t deviceCode);
void displayDeviceInfo(hart_device_info hdi);