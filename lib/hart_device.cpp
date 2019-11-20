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
    if (code == 0xb013) {
        //name = "GW PL ETH/UNI-BUS";
        //company = "Phoenix Contact";
    } else {
        name = "unknown";
        company = "unknown";
    }
}

void HartDevice::print() {
    cout << longTag << endl;
    cout << "\taddress: "; printBytes((uint8_t *)addrUniq, 5);
    cout << "\ttype: " << name << endl;
    cout << "\tcompany: " << company << endl;
    cout << "\tmfr ID: " << hex << (uint32_t)manufacturerId << dec << endl;
    cout << "\tHART revision: " << (uint32_t)cmdRevLvl << endl;
    cout << "\tlong tag: " << longTag << endl;
    cout << "\tdevice revision: " << (uint32_t)revision << endl;
    cout << "\tdevice profile: " << (uint32_t)profile << endl;
    cout << "\tprivate label distributor code: " << hex << (uint32_t)pvtLabelDistCode << dec << endl;
}