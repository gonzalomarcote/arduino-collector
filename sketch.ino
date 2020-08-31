/* Sketch - arduino-collector - Temperature Sensor Poject - V 1.0
Author: Gonzo - gonzalomarcote@gmail.com
*/


// Libraries to connect to SPI, Wifi, WiFiUDP, Time and Pubsubclient
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Time.h>
#include <Timezone.h>
#include <PubSubClient.h>

// Led setup
const int ledPin = 9;

// TMP36 setup
const int tmpPin = 0;

// PIR setup
byte sensorPin = 6;

// Counter variable
int count = 0;

// Wifi setup
char ssid[] = "cbyg";		// Wifi SSID name
char pass[] = "1019goN$44";	// Wifi password
int status = WL_IDLE_STATUS;	// Wifi status

// Network setup
IPAddress ip(192, 168, 1, 114);
IPAddress dns(192, 168, 1, 254);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress checkIp;			// Variable to store IP address
IPAddress checkGateway;			// Variable to store IP gateway

// MQTT server setup
char mqtt_server[] = "broker.cbyg.marcote.org";
const char* topicName = "collector1";

// Initialize the Wifi client and PubSub client libraries
WiFiClient client;
PubSubClient pubClient(client);

// NTP setup
unsigned int localPort = 8888;		// local port to listen for UDP packets
char timeServer[] = "192.168.1.254";	// ntp at cbyg.marcote.org NTP server
const int NTP_PACKET_SIZE = 48;		// NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE];	// buffer to hold incoming and outgoing packets
WiFiUDP Udp;				// A UDP instance to let us send and receive packets over UDP


// --------- Functions ------------ //


// Function to send an NTP request to the time server at the given address
unsigned long sendNTPpacket(char* address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;            // Stratum, or type of clock
  packetBuffer[2] = 6;            // Polling Interval
  packetBuffer[3] = 0xEC;         // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
  return 0;
}

// Function to get the NTP time in Unix time
unsigned long getNtpTime() {
  // After connecting to wifi we update date and time with NTP protocol
  sendNTPpacket(timeServer); // send an NTP packet to a time server

  // wait to see if a reply is available
  delay(1000);
  if (Udp.parsePacket()) {
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    // the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, extract the two words

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900)
    unsigned long secsSince1900 = highWord << 16 | lowWord;

    // now convert NTP time into everyday time
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years
    unsigned long epoch = secsSince1900 - seventyYears;
    Serial.println("Time synced with NTP server!");
    Serial.println("");
    Serial.println("Updating local Time with NTP ...");
    // print Unix time
    return epoch;
    
  }
  Serial.println("ERROR. NTP update failed");
  return 0; // return 0 if unable to get the time
}

// Function to add one 0 to correct display time
void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}

// Function to get TMP36 temperature
float tmp36() {
  // Getting the voltage reading from the temperature sensor
  int reading = analogRead(tmpPin);
  float voltage = reading * 5.0;
  voltage /= 1024.0;
  //Serial.print(voltage); Serial.println(" volts");

  // Calculate the temperature
  float temperature = (voltage - 0.5) * 100 ;
  return temperature;
  //Serial.print(temperature); Serial.println(" ºC");
}


void setup() {
  
  // Initialize serial
  Serial.begin(115200);
  
  // Set ledPin pin out
  pinMode(ledPin, OUTPUT);

  // Set PIR pin in
  pinMode(sensorPin,INPUT);

  // Welcome message
  Serial.println("=== Welcome to arduino-collector Temp sensor 1.0 ===");
  Serial.println("");

  // We connect to Wifi only on Arduino boot
  // Wifi setup
  WiFi.config(ip, dns, gateway, subnet);    // Configure network parameters
  Serial.println("Connecting to Wifi ...");
  status = WiFi.begin(ssid, pass);          // Trying to connect to wifi using WPA2 encryption

  // Failed wWiFi connection
  if ( status != WL_CONNECTED) { 
    Serial.println("ERROR: Failed to connect to Wifi!");
    Serial.println("");
    while(true);
  }

  // Succesfully WiFi connection
  else {
    Serial.println("Connected to Wifi network!");
    // Print IP & Gateway addresses:
    checkIp = WiFi.localIP();
    checkGateway = WiFi.gatewayIP();
    Serial.print("IP address is: ");
    Serial.println(checkIp);
    Serial.print("Gateway address is: ");
    Serial.println(checkGateway);
    
    // If connected to the Wifi, we initializate Udp on specified port
    Serial.print("Initializing UDP on port: ");
    Serial.println(localPort);
    Serial.println("");
    Udp.begin(localPort);
  }

  // After connecting to wifi we initialize MQTT server
  pubClient.setServer(mqtt_server, 30583);

  // Connect to MQTT broker if disconnected
  Serial.println("Connecting to MQTT broker.cbyg.marcote.org ...");
  pubClient.connect("Arduino Collector 1", "user", "passwd");

  // Failed MQTT connection
  if (!pubClient.connected()) {
    Serial.println("ERROR: Failed to connect to MQTT!");
    Serial.println("");
  }

  // Succesfully MQTT connection
  else {
    Serial.println("Connected to MQTT broker.cbyg.marcote.org!");
    Serial.println("");
  }

  // After connecting to wifi we check time with NTP protocol
  Serial.println("Connecting to NTP server ...");
  setSyncProvider(getNtpTime);

  if (timeStatus() == 2) {
    Serial.println("Local time updated with NTP!");
  } else {
    Serial.println("ERROR. Local time not updated with NTP");
    Serial.print("Update status is: ");
    Serial.println(timeStatus());
  }

  // We update the current Time and Date with DST
  time_t central, utc;
  TimeChangeRule esCEST = {"CEST", Last, Sun, Mar, 2, +120};  // UTC + 2 hours
  TimeChangeRule esCET = {"CET", Last, Sun, Oct, 3, +60};     // UTC + 1 hours
  Timezone esCentral(esCEST, esCET);
  utc = now();                                                // Current time from the Time Library
  central = esCentral.toLocal(utc);

  Serial.print("Local Date and Time updated with DST is: ");
  Serial.print(day(central));
  Serial.print(+ "/") ;
  Serial.print(month(central));
  Serial.print(+ "/") ;
  Serial.print(year(central)); 
  Serial.print( " ") ;
  Serial.print(hour(central));
  Serial.print(+ ":") ;
  Serial.print(minute(central));
  Serial.print(":") ;
  Serial.println(second(central));  

  // End setup
  Serial.println("");
  Serial.println("===========================");

}


void loop() {

  // Read motion sensor
  byte state = digitalRead(sensorPin);
  digitalWrite(ledPin, state);

  // Send data to MQTT broker every 60 seconds
  if (count >= 60) {
    // Connect to MQTT broker if disconnected
    if (!pubClient.connected()) {
      Serial.println("Connection to MQTT expired. Connecting to MQTT broker.cbyg.marcote.org ...");
      pubClient.connect("Arduino Collector 1", "user", "passwd");
      float temp = tmp36();
      char buffer[10];
      dtostrf(temp, 4, 2, buffer);
      pubClient.publish(topicName, buffer);
    }

    // Publish to MQTT broker
    else {
      float temp = tmp36();
      char buffer[10];
      dtostrf(temp, 4, 2, buffer);
      pubClient.publish(topicName, buffer);
    }

    // Set count to 0
    count = 0;
  }

  if(state == 1) {

    int x = 0;
    for (x = 0; x < 60; x ++) {
      // Door opened
      Serial.println("Door open. Turn on light");
      digitalWrite(ledPin, state);

      // Print out the temperature
      Serial.print(tmp36()); Serial.println(" ºC");

      delay(1000);
      count ++;
      Serial.println(count);
    }
  }

  else if(state == 0) {
    // Door closed
    Serial.println("Door closed. Turn off light");

    // Print out the temperature
    Serial.print(tmp36()); Serial.println(" ºC");

    delay(1000);
    count ++;
    Serial.println(count);
  }

}
