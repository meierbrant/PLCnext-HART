#include "hart_devices.hpp"
#include "data_types.hpp"

using std::cout;
using std::endl;

hart_device_info getHartDevInfo(uint16_t deviceCode) {
    struct hart_device_info hdi;
    hdi.deviceType = deviceCode;
    switch (deviceCode) {
        case 0xb013:
            hdi.deviceName = "GW PL ETH/UNI-BUS";
            hdi.deviceCompany = "Phoenix Contact";
            break;
        
        default:
            hdi.deviceName = "unknown";
            hdi.deviceCompany = "unknown";
    }
    return hdi;
}

void displayDeviceInfo(hart_device_info hdi) {
    cout << hdi.deviceName << " (" << hdi.deviceCompany << ") " << "address: ";
    printBytes(hdi.addrUniq, 5);
}