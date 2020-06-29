# Fabman door reader

This repository contain firmware and 3D files for Fabman door reader. This device and firmware were designed and developed by FabLab Brno. 

You can find here: 
 - PCB data for first and second revision of HW 
 - STL files for case
 - design files of the case for Fusion 360
 - source code of the firmware for this device 
 - images of assembled board and images of the case
 
We used Arduino IDE for development of this firmware. 

You will need these libraries to compile FW from source code: 
 - ArduinoJson (v. 6.13.0 in Library Manager)
 - WiFiManager (https://github.com/tzapu/WiFiManager/tree/development)- install it in IDE from ZIP file
 - SimpletTimer (https://github.com/jfturcot/SimpleTimer) - install it in IDE from ZIP file
 - SmartLeds (https://platformio.org/lib/show/1740/SmartLeds) - install it in IDE from ZIP file

#### This firmware include following:

- [x] OTA update
- [x] WiFiManager - you can setup your WiFi network for OTA updates.

### There are these revision of the board:

**rev. A**

First version of the board. Step-down converter is not working (wrong layout). RGB LED need bodge wire to the IO4 on ESP32. Everything else is working. 

**rev. B(untested)** 

Step-down converter is now using reference design for MP1584EN (working on external board). RGB LED connection fixed. Relay and 3-pon JST connector moved to bottom side to save some height. Added setup button for WiFiManager.

![](https://github.com/fablab-brno/Fabman-door-reader/blob/master/img/FM_door.JPG)

