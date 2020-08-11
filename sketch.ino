/* Sketch - CBYG Temp Sensor Poject - V. 1.0
Author: Gonzo - gonzalomarcote@gmail.com
*/


// Libraries to connect to SPI, Wifi, WiFiUDP, Time
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Time.h>
#include <Timezone.h>
#include <LiquidCrystal.h>

// ledPin setup
const int ledPin = 9;

// PIR setup
byte sensorPin = 6;

// LCD setup
LiquidCrystal lcd(10, 8, 5, 4, 3, 2);

// Web service
char server[] = "www.google.com";

// Wifi setup
char ssid[] = "cbyg";		// Wifi SSID name
char pass[] = "1019goN$44";	// Wifi password
int status = WL_IDLE_STATUS;	// Wifi status

// Initialize the Wifi client library
WiFiClient client;

// Network setup
IPAddress ip(192, 168, 1, 156);
IPAddress dns(192, 168, 1, 254);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress checkIp;			// Variable to store IP address
IPAddress checkGateway;			// Variable to store IP gateway

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
    Serial.println("Updating local Time with NTP...");
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


void setup() {
  
  // Initialize serial
  Serial.begin(115200);
  
  // Set ledPin pin out
  pinMode(ledPin, OUTPUT);

  // Set PIR pin in
  pinMode(sensorPin,INPUT);

  // Set LCD
  lcd.begin(16, 2);

  // Welcome message
  Serial.println("== Welcome to CBYG Temp sensor 1.0 ==");
  Serial.println("");


  // We connect to Wifi only on Arduino boot
  // Wifi setup
  WiFi.config(ip, dns, gateway, subnet);    // Configure network parameters
  Serial.println("Connecting to Wifi ...");
  status = WiFi.begin(ssid, pass);          // Trying to connect to wifi using WPA2 encryption

  // Failed wifi connection
  if ( status != WL_CONNECTED) { 
    Serial.println("ERROR: Failed to connect to Wifi!");
    Serial.println("");
    while(true);
  }

  // Succesfully connected:
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

  // After connecting to wifi we check time with NTP protocol
  Serial.println("Connecting to NTP server ...");
  setSyncProvider(getNtpTime);

  if (timeStatus() == 2) {
    Serial.println("Local time updated with NTP!");
    Serial.print("Update status is: ");
    Serial.println(timeStatus());
  } else {
    Serial.println("ERROR. Local time not updated with NTP");
    Serial.print("Update status is: ");
    Serial.println(timeStatus());
  }

  /*
  Serial.print("Local Date and Time updated with NTP is: ");
  time_t t = now();
  Serial.print(day(t));
  Serial.print(+ "/") ;
  Serial.print(month(t));
  Serial.print(+ "/") ;
  Serial.print(year(t)); 
  Serial.print( " ") ;
  Serial.print(hour(t));  
  Serial.print(+ ":") ;
  Serial.print(minute(t));
  Serial.print(":") ;
  Serial.println(second(t));
  */


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
  Serial.println("");

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("Connected to server");
    // Make a HTTP request:
    client.println("GET /search?q=arduino HTTP/1.1");
    client.println("Host: www.google.com");
    client.println("Connection: close");
    client.println();
  }

}


void loop() {

  // Read motion sensor
  byte state = digitalRead(sensorPin);
  digitalWrite(ledPin, state);

  if(state == 1) {
    Serial.println("Door open. Turn on LCD and light");
    digitalWrite(ledPin, state);
    // Turn on the display
    lcd.display();
    delay(500);
    lcd.setCursor(0, 0);
    lcd.print("  CBYG - Rack");
    lcd.setCursor(0, 1);
    lcd.print("R:40  B:42  P:32");
    delay(30000);
  }
  else if(state == 0) {
    Serial.println("Door closed. Turn off LCD and light");
    // Turn off the display
    lcd.noDisplay();
    delay(500);
  }

  // if there are incoming bytes available
  // from the server, read them and print them:
  //while (client.available()) {
  //  char c = client.read();
  //  Serial.write(c);
  //}


  // if the server's disconnected, stop the client:
  //if (!client.connected()) {
  //  Serial.println();
  //  Serial.println("disconnecting from server.");
  //  client.stop();

    // do nothing forevermore:
  //  while (true);
  //}

}
