# fanshim-cpp
C++ driver code for the fanshim on raspberry pi.
https://shop.pimoroni.com/products/fan-shim


**Warning/disclaimer:**

**very experimental code, for testing purposes only.**

**[may not work at any time or even cause hardware damage (e.g. from causing the fan to be constantly running)].**

**Only proceed if you know what the code is doing.**

## Credits
 - Wiring Pi http://wiringpi.com/
 - nlohmann/json https://github.com/nlohmann/json/
 - official fanshim controller code https://github.com/pimoroni/fanshim-python
 
 ## Build
 - Install wiring pi (pi 4 may need the latest verion `2.52` to work: http://wiringpi.com/wiringpi-updated-to-2-52-for-the-raspberry-pi-4b/)
 - Put the `json.hpp` file from https://github.com/nlohmann/json/releases in the same folder as the source code, tested with `3.7.0`
 - Compile with `g++ fanshim_driver.cpp -o fanshim_driver -O3 -lwiringPi -std=c++11` (tested with `clang++`)
 
 ## Example systemd service file
 Change `/path/to/compiled/binary` to the compiled binary path.
 
 ```
 [Unit]
Description=Fanshim C++ driver

[Service]
ExecStart=/path/to/compiled/binary
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target

 ```
 
 ## Configuration
 
 The configuration file is in `/usr/local/etc/fanshim.json`, example:
 ```json
 {
    "on-threshold": 60,
    "off-threshold": 50,
    "delay": 10
}
 ```
 
Will use the value in the file to override the defaults, no need to specify all keys, just the ones you want to change. Keys used:
 
 `on-threshold`/`off-threshold`: temperature, in Celsius, the threshold for turing on (off) the fan. Default to 60 and 50 respectively.
 
 `delay`: in seconds, the program will wait this amount of time before checking the temperature again. Default to 10.
 
 `budget`: an  integer n, the program will only turn on/off the fan if the temperature is consecutively above (below) the on (off) threshold for the last n temperature measurements. Defaults to 3.

 `brightness`: an integer from 0 to 31, LED brightness, 0 means no LED (default).
 
## Notes/todo
 - Does not currently handle exit signal.
 - No button support (I think given the small size of the button, it'll be easier to force the fan on/off through software. May add this later.)
 
## Additional features
 
 - Will output current status to the file `/usr/local/etc/node_exp_txt/cpu_fan.prom` so that it can be used with external programs to monitor, e.g. node_exporter + prometheus + grafana:
 
 ![screen](https://raw.githubusercontent.com/daviehh/fanshim-cpp/master/rpi_monit_eg.png)
