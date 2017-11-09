/*
  Remote Switch Web Server

 A simple web server that shows the value of the digital
 output pin using an Arduino Ethernet shield. Responds to
 GET requests to turn on and off the output pin.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Digital output attached to pin 0

 created 7 Nov 2017
 by Michael Lohrer
 modified 7 Nov 2017
 */

#include <avr/wdt.h>
#include <SPI.h>
#include <Ethernet.h>

// Digital pin to use as output as well as LED to turn on
int outpin_ = 9;
bool on_ = false;

// Set the MAC address to what is printed on the shield
// Set the IP address to 10.0.0.4
byte mac[] = {
  0x90, 0xA2, 0xDA, 0x0F, 0x27, 0x38
};
IPAddress ip(10, 0, 0, 4);

// Initialize the Ethernet server library at port 80
EthernetServer server(80);

const char RESP200OFF[] PROGMEM =
  "HTTP/1.1 200 OK\r\n"
  "Content-Type: text/html\r\n"
  "Connection: close\r\n\r\n"
  "<!DOCTYPE HTML><html>"
  "Switch OFF</html>\r\n";
const char RESP200ON[] PROGMEM =
  "HTTP/1.1 200 OK\r\n"
  "Content-Type: text/html\r\n"
  "Connection: close\r\n\r\n"
  "<!DOCTYPE HTML><html>"
  "Switch ON</html>\r\n";
const char RESP405[] PROGMEM =
  "HTTP/1.1 405 Method Not Allowed\r\n"
  "Content-Type: text/html\r\n"
  "Connection: close\r\n\r\n"
  "<!DOCTYPE HTML><html>Only GET requests supported."
  "</html>\r\n";
PGM_P const RESPONSES[] PROGMEM = {RESP200OFF, RESP200ON, RESP405};

// Buffer to hold request and response
char buf_[256];

// GET path for all available requests, must end in space
const char GET_ON[]       = "/on ";
const char GET_OFF[]      = "/off ";

size_t response(char* buf, bool* on) {
  char* bufptr = buf;
  if (strncmp(bufptr, "GET ", 4)) {
    // Not a GET request, method not supported
    Serial.println(F("invalid request received"));
    strcpy(buf, RESP405);
    return size_t(RESP405);
  }
  bufptr += 4;  // Skip past the "GET "

  // Check for /on or /off and turn on/off output
  if (!strncmp(bufptr, GET_OFF, strlen(GET_OFF))) {
    *on = false;
  } else if (!strncmp(bufptr, GET_ON, strlen(GET_ON))) {
    *on = true;
  }

  int response = *on ? 1 : 0;
  PGM_P resptr = (PGM_P)pgm_read_word(&RESPONSES[response]);
  strcpy_P(buf, resptr);
  return strlen_P(resptr);
}

void setup() {
  // Initialize I/O
  pinMode(outpin_, OUTPUT);
  digitalWrite(outpin_, on_);
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial);

  // Start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print(F("server is at "));
  Serial.println(Ethernet.localIP());
  
  // Enable watchdog
  wdt_enable(WDTO_1S);
}

void loop() {
  // Listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    //Serial.println("new client");
    char* bufptr = buf_;
    while (client.connected()) {
      if(client.available()) {
        char c = client.read();
        if (c == '\n') {
          size_t len = response(buf_, &on_);
          digitalWrite(outpin_, on_); // Write the output pin
          client.write(buf_, len);    // Send response packet
          delay(1);                   // Give the web browser time to receive the data
          client.stop();              // Close the connection
          //Serial.println("client disconnected");
        } else {
          *bufptr = c;
          bufptr++;
        }
      }
    }
  }
  
  // Reset watchdog timer
  wdt_reset();
}

