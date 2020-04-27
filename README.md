# PLCnext HART Integration

The goal of this C++ library is to allow the PLCnext to connect with HART devices through the Phoenix Contact HART MUX.

## Test from Linux Dev Machine
<!-- Make sure `libssl-dev` and `libssl-dev:1386` are installed. -->
Compile with:
```
g++ -pthread -o hart_mux_server main.cpp lib/*.cpp
```

To start the HTTP server for the HART Gateway API at `localhost:5900`:
```
./hart_mux_server
```

To start the UI webserver at `localhost:8080`:
```
cd hmi/pxc-hart-management
npm run serve
```

## Deploy on PLCnext

### Cross-Compile from Linux Dev Machine

Install the `g++-arm-linux-gnueabihf` package to allow the host machine to cross-compile for arm:
```
sudo apt install g++-arm-linux-gnueabihf
```
To fix an issue with enums that occurs switching to arm, open `lib/csv-parser/parser.hpp` and change
```c++
enum class Term : char { CRLF = -2 };
```
to
```c++
enum class Term { CRLF = -2 };
```

Compile with:
```
arm-linux-gnueabihf-g++ -std=c++11 -Wno-psabi -pthread -o arm_hartip_server main.cpp lib/*.cpp
```

## Installing on PLCnext
SSH into the PLCnext and set up the environment:
```bash
ssh admin@192.168.1.10
mkdir -p /opt/plcnext/hartip
```

From the linux dev machine, cross-compile the C++ backend for ARM and copy the cross-compiled binary to the PLCnext:
```
scp arm_hartip_server admin@192.168.1.10:/opt/plcnext/hartip
scp -r hart-csv admin@192.168.1.10:/opt/plcnext/hartip
```

Build the web UI assets and copy them to the PLCnext:
```
cd hmi/pxc-hart-management
npm run build
scp -r dist admin@192.168.1.10:/opt/plcnext/hartip
```
You should now have both the `arm_hartip_server` binary, the `hart-csv` directory, and the `dist` directory with the web UI assets in `/opt/plcnext/hartip`.

### Adding the Hart Management Web UI to the Nginx server
SSH into the PLCnext and edit the nginx conf file, `/etc/nginx/nginx.conf`.

Find the port 80 server section and mount the hartip `dist` folder at `/hartip`. Make sure to comment out the SSL redirect line. This is necessary because the hartip-server only supports HTTP, not HTTPS.
```bash
    server {
        listen  80;

        location /hartip {
            alias /opt/plcnext/hartip/dist;
            index index.html;

            add_header X-Frame-Options SAMEORIGIN;
        }
        #return 301 https://$host$request_uri;
    }
```
Don't forget to restart nginx as root:
```bash
/etc/init.d/nginx restart
```

### Making the HART IP server run on startup
Copy `hartip_server_loop.sh` from the host machine to `/opt/plcnext/hartip` on the PLCnext:
```bash
scp hartip_server_loop.sh admin@192.168.1.10:/opt/plcnext/hartip
```

As root, copy the `hartip-server` script in this repo to `/etc/init.d/` and change its permissions to `755`. Then add it to the default daemons for startup:
```bash
update-rc.d hartip-server defaults
```


## Issues
* `lib/udp_dateway_discovery.cpp`:`discoverGWs()`

    IP broadcast address is hardcoded. The broadcast address of the PLCnext's network should be auto discovered.

* `lib/hart_device.cpp`:`setTypeInfo(uint16_t deviceTypeCode)`
    
    For some reason, strings with more than 15 characters cause `addrUniq` and a lot of other class vars to get wiped out. The theoretical limit of the string data type should be practically infinite, so not sure where the memory leak is occurring.
    
* `lib/hart_device.cpp`:`setTypeInfo(uint16_t deviceTypeCode)`

    This function re-parses the CSV file each time a lookup needs to be done (once per device discovery/update). It would be good to parse the file once and keep the info in memory.

* `lib/hart_device.cpp`:`getUnitsFromCode(uint8_t unit_code)`

    This function re-parses the CSV file each time a lookup needs to be done (once per variable update). It would be good to parse the file once and keep the info in memory.


* About 30% of the time, when the `hart_mux_server` is started, it is incapable of discovering gateways no matter how many times you ask it. When trying to access `/gw/{serialNo}/info`, the server throws the following error:
    ```
    terminate called after throwing an instance of 'nlohmann::detail::type_error'
    what():  [json.exception.type_error.302] type must be string, but is null
    ```

    It is known that occasionally, after the discovery network broadcast is sent out, the response appears to be sourced from the broadcase address (i.e. 192.168.254.255) instead of from the gateway that sent the response. When this happens the first time in the server, it seems that no matter how many times the server attempts this process, the result is the same. This may mean that there is some outside condition that must be true when the server starts.

* The `hart_mux_server` does not yet handle autodiscovery cases when there are multiple gateways on the network.