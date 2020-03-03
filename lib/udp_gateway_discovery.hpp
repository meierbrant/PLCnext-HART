#pragma once

#include "nlohmann/json.hpp"

using namespace std;
using nlohmann::json;

bool isBcastAddr(string ip);
string resolveIoCardType(char moduleCode);
json discoverGWs(string bcastAddr);