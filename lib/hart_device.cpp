#include "hart_device.hpp"
#include "data_types.hpp"
#include "nlohmann/json.hpp"

using std::cout;
using std::endl;

HartDevice::HartDevice() {}

HartDevice::HartDevice(uint16_t deviceTypeCode) {
    typeCode = deviceTypeCode;
    setTypeInfo(typeCode);
}

void HartDevice::setTypeInfo(uint16_t code) {
    if (code == 0xb013) {
        // name = "GW PL ETH/UNI-BUS";
        // company = "Phoenix Contact";
    } else {
        name = "unknown";
        company = "unknown";
    }
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

string getUnitsFromCode(uint8_t code) {
    string units;
    if (code == 32) {
        units = "°C";
    } else {
        units = "unknown units";
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
        {"channel", channel}
    };

    return data;
}