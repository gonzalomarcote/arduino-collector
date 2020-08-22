ARDUINO_DIR             = /usr/share/arduino
ARDUINO_PORT            = /dev/ttyACM0
AVRDUDE_ARD_BAUDRATE	= 115200
AVRDUDE_ARD_PROGRAMMER	= arduino
BOARD_TAG		= uno
MONITOR_PORT		= /dev/ttyACM0
ARDUINO_LIBS 		+= Wire \
			   WiFi \
                	   SPI \
                	   Time \
			   Timezone 

include /usr/share/arduino/Arduino.mk
