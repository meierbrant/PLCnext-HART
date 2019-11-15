#include "hart_device.hpp"
#include "data_types.hpp"

using std::cout;
using std::endl;

HartDevice::HartDevice() {}

HartDevice::HartDevice(uint16_t deviceTypeCode) {
    typeCode = deviceTypeCode;
    setTypeInfo(typeCode);
}

void HartDevice::setTypeInfo(uint16_t code) {
    switch (code) {
        case 0xb013:
            name = "GW PL ETH/UNI-BUS";
            company = "Phoenix Contact";
            break;
        
        default:
            name = "unknown";
            company = "unknown";
    }
}

void HartDevice::print() {
    cout << name << " (" << company << ") " << "address: ";
    printBytes((uint8_t *)addrUniq, 5);
}