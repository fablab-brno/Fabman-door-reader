# Fabman door reader

This repository is for Fabman door reader. This device and firmware were designed and developed by FabLab Brno. 

You can find here: 
 - PCB data for all revision of HW 
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
- [x] WiFiManager - you can setup your WiFi network for OTA updates, API key for Fabman bridge, name of the device, ID of the device and if connection type (WiFi/Ethernet) 

If somebody swipe their card and device is properly set to use Fabman service, it will send standart query and behave on received data. There is also option (which we use, but it is not default behaviour) to check, if member has **any** actvive package, and if so,it will be allowd to use door. This behaviour must be set by bollean variable (checkOH in config.h file) and FW must recompiled and flashed again in to the device. I will try to add this as option via WiFiManager in the future.  
OTA updates are available, but you must have some server where you can store text and bin files. Device check for new FW update at every start. 

#### HW include following: 

 - RGB LED (WS2812B) - which indicate status of the device (breathing blue - idle, green - allowed access, red - denied access, orange - WiFiManager setup process)
 - 12V to 5V voltage converter based on MP1584EN
 - 3-pin connector for 12V power connection (+12V, GND, GND for lock)
 - 2-pin connector for security alarm (optional). This is normaly closed when cover is closed as screw is pushing towards tactile switch. If somebody open the cover, switch will be released and external alarm system should trigger some alert.    
 - Security button
 - 3-pin JST connector for RFID data. This one is connected to the GWIOT 7941E RFID module which sends data via UART connection (9600 bauds). There is 5V, DATA and GND.  
 - 5V DC relay G6L-1F-DC5 which connect GND to the lock in the door. Positive voltage must be connected to the door directly.
 - 4-pin connector for UART. There is 5V, GND, RX and TX connection and this connector is used for programming and debugging.
 - Reset button
 - Boot button
 - Setup button - if you hold this button when device is powered on or reseted, it will call function which will turn on AP mode in ESP32 and will fire up web server with default address **192.168.4.1**. 

You can find some images of the board, 3D printed case and whole assembly in **/img** folder.
  
#### There are these revision of the board:

**rev. A**

First version of the board. Step-down converter is not working (wrong layout). RGB LED need bodge wire to the IO4 on ESP32. Everything else is working. 

**rev. B** 

Step-down converter is now using reference design for MP1584EN. RGB LED connection fixed. Relay and 3-pin JST connector moved to bottom side to save some height. Added setup button for WiFiManager. Added 6-pin connector to connect LAN module via SPI bus. This was working but LAN module (ENC28J60) does not supprt SSL connection.

**rev. C**

6-pin connector changed for 14-pin connector which support LAN8720 module. This module is supported by Arduino-ESP32 development tools. This as tested and it is working. **This board does not support PoE**. 

![](https://github.com/fablab-brno/Fabman-door-reader/blob/master/img/FM_door.JPG)

