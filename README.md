# Product360
A 360 Product Photo sketch for NodeMcu v1.0 ESP8266 12E chip using Arduino IDE. The sketch is bult on top of following library:

- [gmag11/FSBrowserNG](https://github.com/gmag11/FSBrowserNG)
- [AccelStepper](http://www.airspayce.com/mikem/arduino/AccelStepper/)

## Pre-requesite
Some settings are required to make Arduino IDE compatible with Nodemcu chip. Below are some turorial with explained details:
- Setting up Arduino IDE for esp8266 chip: Tutorial [here](http://www.instructables.com/id/Quick-Start-to-Nodemcu-ESP8266-on-Arduino-IDE/).
- Arduino ESP8266 Filesystem Uploader: Tutorial [here](http://www.instructables.com/id/Using-ESP8266-SPIFFS/). Jump to step 2.

## Instruction
1. Once arduino has been set-up for nodemcu, connect nodemcu.
2. Make the following settings in Arduino IDE:
    - Tools > Board: NodeMcu 1.0 (ESP-12E Module) or your NodeMcu board.
    - Tools > CPU Frequency: 80MHz
    - Tools > Flash Size: 4M (3M SPIFFS)
    - Tools > Upload Speed: 115200
    - Tools > Port: your device port
3. Open Product360.ino sketch
4. Navigate to Tools > ESP8266 Sketch Data Upload. Upon click, the data will start uploading to NodeMcu.
5. After uploading data, navigate to Sketch > Verify/Compile. If there's no error, naviagete to Sketch > Upload. The code will start uploading.
    - Note: On Ubuntu/Linux Mint or other debian based linux, the `  sudo chmod 777 /dev/ttyUSBX `is required to be executed in terminal before start uploading, where X is confirmed at step 2 (device port).
6. Disconnect and reconnect NodeMcu or just power it up using mobile charger or powerbank. Scan wifi and connect to ESP**** wifi network.
7. Once connected, visit ` 192.168.4.1` in your browser. Enjoy!!!
