# arduino-collector
Arduino project to check temperature and send it to one MQTT broker.  

### Arduino project to meassure temp in my home rack
Arduino Uno board project that check the temperature of my home rack and send it to one MQTT broker.  
Extra - Optionally it will turn on a light with a motion sensor when my home rack door is opened.  
Compiled with `arduino-mk` linux package.  

`sketch.ino` -> is the main program file.  
`Makefile` -> the make file to compile it with arduino-mk.  

The following libraries need to be downloaded to `/usr/share/arduino/libraries/`:
* Wire
* WiFi
* SPI
* Time
* TimeAlarms
* Timezone

To upload `skecth.ino` sketch to arduino uno board just type:
```
$ fuser -k /dev/ttyACM0
$ make upload
```
Be sure to be in `dialout` group.  

To connect to serial type:  
```
$ screen /dev/ttyACM0 115200
```
Be sure to have in `Serial.begin(115200)` the same serial number. 
