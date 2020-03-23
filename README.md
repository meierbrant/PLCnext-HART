# PLCnext HART Integration

The goal of this C++ library is to allow the PLCnext to connect with HART devices through the Phoenix Contact `GW PL ETH/UNI-BUS` or `GW PL ETH/BASIC-BUS`.

When cloning this repo, make sure to `--recurse-submodules` to get `cpp-httplib`.

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
_Note: For Windows, you can use [WinSCP](https://winscp.net/eng/index.php) instead of scp to graphically transfer files and you can use [PuTTY](https://www.putty.org/) for ssh._

### Transfering the Files

SSH into the PLCnext as the `admin` user and create the hartip folder:
```bash
mkdir -p /opt/plcnext/hartip
```

Copy the cross-compiled binary, `arm_hartip_server`, and the `hart-csv`, `cmd-definitions`, and `dist` directories to the PLCnext (into the `/opt/plcnext/hartip` directory). Linux example:
```
scp arm_hartip_server admin@192.168.1.10:/opt/plcnext/hartip
scp -r hart-csv admin@192.168.1.10:/opt/plcnext/hartip
scp -r cmd-definitions admin@192.168.1.10:/opt/plcnext/hartip
scp -r dist admin@192.168.1.10:/opt/plcnext/hartip
```

### Adding the HART Monitoring Web UI to the Nginx server
This will make the HART Management System mount at `http://192.168.1.10/hartip`.

SSH into the PLCnext and edit the nginx conf file, `/etc/nginx/nginx.conf`. You may need to SSH as the `admin` user, then run `su root` to gain root privileges first. _Note: If you are not able to log in with `su root`, you may need to set the root password first with the command `sudo passwd root` (enter your admin password first, then you will be prompted to enter the new password for root). Then edit the config file by running `nano /etc/nginx/nginx.conf`._

Add a new port 80 server section and mount the hartip `dist` folder at the `/hartip` URI. Make sure to comment out the SSL redirect line. This is necessary because the hartip-server only supports HTTP, not HTTPS. The new section should look like this:
```bash
    server {
        listen  80;

        location /hartip {
            alias /opt/plcnext/hartip/dist;
            index index.html;
            ssi on;
            # Without this line, most URI's from the app will result in 404 errors
            # since this is a Single Page Application and the "pages" don't technically exist.
            try_files $uri $uri/ /index.html

            add_header X-Frame-Options SAMEORIGIN;
        }
        error_page 500 502 503 504 /500x.html;
        location /50x.html {
            root /var/www/localhost/html;
        }
        #return 301 https://$host$request_uri;
    }
```
Don't forget to restart nginx as root:
```bash
/etc/init.d/nginx restart
```

### Making the HART IP server run on startup
Copy `hartip_server_loop.sh` from the host machine to `/opt/plcnext/hartip` on the PLCnext. Linux example:
```bash
scp hartip_server_loop.sh admin@192.168.1.10:/opt/plcnext/hartip
```

As root(you may need to SSH in as `admin` first, then login as root using `su root`), copy the `hartip-server` script to `/etc/init.d/`on the PLCnext and change its permissions to `755` with:
```bash
chmod 755 /etc/init.d/hartip-server
```
Then add it to the default daemons for startup:
```bash
update-rc.d hartip-server defaults
```

## Additional Developer Info

### Test from Linux Dev Machine
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

### Cross-Compile from Linux Dev Machine

Install the `g++-arm-linux-gnueabihf` package to allow the host machine to cross-compile for arm:
```
sudo apt install g++-arm-linux-gnueabihf
```

Compile with:
```
arm-linux-gnueabihf-g++ -std=c++11 -Wno-psabi -pthread -o arm_hartip_server main.cpp lib/*.cpp
```

### Build the assets for production
```bash
cd hmi/pxc-hart-management
npm run build
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


* About 30% of the time, when the `hart_mux_server` is started, it is incapable of discovering gateways no matter how many times you ask it.

    It is known that occasionally, after the discovery network broadcast is sent out, the response appears to be sourced from the broadcase address (i.e. 192.168.254.255) instead of from the gateway that sent the response. When this happens the first time in the server, it seems that no matter how many times the server attempts this process, the result is the same. This may mean that there is some outside condition that must be true when the server starts.

* The `hart_mux_server` does not yet handle autodiscovery cases when there are multiple gateways on the network (but there is an [in-progress branch](https://github.com/meierbrant/PLCnext-HART/tree/feature/multiple-gw-discovery) for it).