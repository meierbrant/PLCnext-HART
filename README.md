# PLCnect HART Integration

The goal of this C++ library is to allow the PLCnext to connect with HART devices through the Phoenix Contact HART MUX.

## Installation
compile with `g++ -pthread -o hart_mux_server *.cpp`

## Usage
Running `sudo ./hart_mux_server` will start an HTTP server at `localhost:5900`.
