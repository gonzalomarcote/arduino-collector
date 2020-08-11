# arduino-cbyg
Arduino project to check temperature and display it in one LCD and turn on a light with motion sensor for my home rack.

### Arduino project for my home rack
Arduino Uno board project that shows in an LCD the rack temperature and have a motion sensor that triggers a light.  
Optionally I will recollect servers temperatures with a web service and will start on fan on high temps.  
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
