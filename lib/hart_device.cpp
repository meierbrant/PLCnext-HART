#include <iostream>
#include "hart_device.hpp"
#include "hart_mux.hpp"
#include "data_types.hpp"
#include "nlohmann/json.hpp"
#include "csv-parser/parser.hpp"

using std::ifstream;
using std::cout;
using std::endl;
using std::string;
using aria::csv::CsvParser;

#define HART_DEVICE_INFO_CSV_FILE "hart-csv/HART-expanded-device-type-codes.csv"
#define HART_UNIT_CODE_CSV_FILE "hart-csv/HART-unit-codes.csv"

HartDevice::HartDevice() {}

HartDevice::HartDevice(uint16_t deviceTypeCode) {
    setTypeInfo(deviceTypeCode);
}

void HartDevice::setTypeInfo(uint16_t deviceTypeCode) {
    ifstream f(HART_DEVICE_INFO_CSV_FILE);
    CsvParser parser(f);
    uint16_t code;
    string code_string;
    string description, company_name;
    int col = 0;
    bool lookup_success = false;

    for (auto& row : parser) {
        for (auto& field : row) {
            switch (col) {
                case 0:
                    code_string = string(field);
                    code = (uint16_t)std::stoi(code_string, nullptr, 16);
                    break;
                case 1:
                    description = string(field);
                    break;
                case 2:
                    company_name = string(field);
            }
            col = (col + 1) % 3;
        }
        if (deviceTypeCode == code) {
            lookup_success = true;
            break;
        }
    }
    typeCode = deviceTypeCode;

    if (lookup_success) {
        name = description;
        company = company_name;
    }
}

hart_var_set HartDevice::readVars() {
    return hart_mux->readSubDeviceVars(*this);
}

void HartDevice::sendCmd(unsigned char cmd, uint8_t *reqData, size_t reqDataCnt, uint8_t *resData, size_t &resDataCnt, uint8_t &status) {
    hart_mux->sendSubDeviceCmd(cmd, *this, reqData, reqDataCnt, resData, resDataCnt, status);
}

void HartDevice::print() {
    cout << longTag << endl;
    cout << "\ttype: " << name << endl;
    cout << "\tcompany: " << company << endl;
    cout << "\taddress: "; printBytes((uint8_t *)addrUniq, 5);
    cout << "\tio card: " << (uint32_t)ioCard << endl;
    cout << "\tchannel: " << (uint32_t)channel << endl;
    // cout << "\tmfr ID: " << hex << (uint32_t)manufacturerId << dec << endl;
    cout << "\tHART revision: " << (uint32_t)cmdRevLvl << endl;
    // cout << "\tlong tag: " << longTag << endl;
    // cout << "\tdevice revision: " << (uint32_t)revision << endl;
    // cout << "\tdevice profile: " << (uint32_t)profile << endl;
    // cout << "\tprivate label distributor code: " << hex << (uint32_t)pvtLabelDistCode << dec << endl;
    cout << "\textended device status bits: "; printBytes(extendedDeviceStatusBits);
}

hart_var_set deserializeHartVarSet(uint8_t *bytes, size_t bCnt) {
    hart_var_set vars;
    vars.loopCurrent = fromBytes<float>(bytes, 4);
    hart_var pv;
    pv.units = getUnitsFromCode(bytes[4]);
    pv.value = fromBytes<float>(&bytes[5], 4);
    vars.pv = pv;
    if (bCnt >= 14) {
        hart_var sv;
        sv.units = getUnitsFromCode(bytes[9]);
        sv.value = fromBytes<float>(&bytes[10], 4);
        vars.sv = sv;
    }
    if (bCnt >= 19) {
        hart_var tv;
        tv.units = getUnitsFromCode(bytes[14]);
        tv.value = fromBytes<float>(&bytes[15], 4);
        vars.tv = tv;
    }
    if (bCnt == 24) {
        hart_var qv;
        qv.units = getUnitsFromCode(bytes[19]);
        qv.value = fromBytes<float>(&bytes[20], 4);
        vars.qv = qv;
    }
    return vars;
}

string getUnitsFromCode(uint8_t unit_code) {
    ifstream f(HART_UNIT_CODE_CSV_FILE);
    CsvParser parser(f);
    uint8_t code;
    string units = "";
    string code_string;
    int col = 0,
        r = 0;
    bool lookup_success = false;

    for (auto& row : parser) {
        if (r++ != 0) {
            for (auto& field : row) {
                switch (col) {
                    case 0:
                        code_string = string(field);
                        code = (uint8_t)std::stoi(code_string, nullptr, 10);
                        break;
                    case 1:
                        units = string(field);
                }
                col = (col + 1) % 2;
            }
            if (unit_code == code) {
                lookup_success = true;
                break;
            }
        }
    }
    return units;
}

void displayVars(hart_var_set vars) {
    cout.precision(4);
    cout << "\tloop current: " << vars.loopCurrent << "mA" << endl;
    cout << "\tpv: " << vars.pv.value << vars.pv.units << endl;
    cout << "\tsv: " << vars.sv.value << vars.sv.units << endl;
    cout << "\ttv: " << vars.tv.value << vars.tv.units << endl;
    cout << "\tqv: " << vars.qv.value << vars.qv.units << endl;
}

json extendedDeviceStatusBitsToJson(uint8_t bits) {
    json data = json::array();
    if (!!(bits & 0x01)) {
        data[data.size()] = {
            {"name", "Maintenance Required"},
            {"description", "This bit is set to indicate that, while the device has not malfunctioned, the Field Device requires maintenance. Devices supporting this bit should support the Condensed Status Commands (see Common Practice Command Specification)."},
            {"condensedStatus", true}
        };
    } else if (!!(bits & 0x02)) {
        data[data.size()] = {
            {"name", "Device Variable Alert"},
            {"description", "This bit is set if any Device Variable is in an Alarm or Warning State. The host should identify the Device Variable(s) causing this to be set using the Device Variable Status indicators."},
            {"condensedStatus", false}
        };
    } else if (!!(bits & 0x04)) {
        data[data.size()] = {
            {"name", "Critical Power Failure"},
            {"description", "For devices that can operate from stored power. This bit is set when that power is becoming critically low. For example, a device scavenging power loosing that power source would set this bit. Devices must be able to sustain their network connection for at least 15 minutes from the when this bit is set. A device may begin gracefully disconnecting from the network if its power level drops too low."},
            {"condensedStatus", false}
        };
    } else if (!!(bits & 0x08)) {
        data[data.size()] = {
            {"name", "Failure"},
            {"description", "When this bit is set one or more Device Variables (i.e., measurement or control values) are invalid due to a malfunction in the field device or its peripherals. Devices supporting this bit must support the Condensed Status Commands (see Common Practice Command Specification)."},
            {"condensedStatus", true}
        };
    } else if (!!(bits & 0x10)) {
        data[data.size()] = {
            {"name", "Out of Specification"},
            {"description", "When set, this bit indicates deviations from the permissible ambient or process conditions have been detected that may compromise measurement or control accuracy (i.e., device performance may be degraded given current operating conditions). Devices supporting this bit must support the Condensed Status Commands (see Common Practice Command Specification)."},
            {"condensedStatus", true}
        };
    } else if (!!(bits & 0x20)) {
        data[data.size()] = {
            {"name", "Function Check"},
            {"description", "This bit is set if one or more Device Variables are temporarily invalid (e.g. frozen) due to ongoing work on the device. Devices supporting this bit must support the Condensed Status Commands (see Common Practice Command Specification)."},
            {"condensedStatus", true}
        };
    }

    return data;
}

json HartDevice::to_json() {
    json data = {
        {"name", name},
        {"company", company},
        {"manufacturerId", manufacturerId},
        {"cmdRevLvl", cmdRevLvl},
        {"longTag", longTag},
        {"revision", revision},
        {"profile", profile},
        {"privateLabelDistCode", pvtLabelDistCode},
        {"ioCard", ioCard},
        {"channel", channel},
        {"extendedDeviceStatus", extendedDeviceStatusBitsToJson(extendedDeviceStatusBits)}
    };

    return data;
}