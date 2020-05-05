#pragma once

#include "nlohmann/json.hpp"

using namespace std;
using nlohmann::json;

class FailedGwDiscoveryException {};

bool isBcastAddr(string ip);
string resolveIoCardType(char moduleCode);
int _gw_discover_attempt(json **gwArray);
json discoverGWs(json network_info);