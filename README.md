# PLCnect HART Integration

The goal of this C++ library is to allow the PLCnext to connect with HART devices through the Phoenix Contact HART MUX.

## Installation
compile with `g++ -pthread -o hart_mux_server main.cpp lib/*.cpp`

## Usage
Running `./hart_mux_server` will start an HTTP server at `localhost:5900`.

## Issues
* `lib/hart_device.cpp`:`setTypeInfo(uint16_t deviceTypeCode)`
    
    For some reason, strings with more than 15 characters cause `addrUniq` and a lot of other class vars to get wiped out. The theoretical limit of the string data type should be practically infinite, so not sure where the memory leak is occurring.
    
* `lib/hart_device.cpp`:`setTypeInfo(uint16_t deviceTypeCode)`

    This functino re-parses the CSV file each time a lookup needs to be done (once per device discovery/update). It would be good to parse the file once and keep the info in memory.