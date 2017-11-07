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

#include <SPI.h>
#include <Ethernet.h>

// Digital pin to use as output.
int pin = 0;
bool switchOn = false;

// Set the MAC address to an unused RHK address
// Set the IP address to 10.0.0.4
byte mac[] = {
  0x00, 0xD0, 0x0B, 0x00, 0x0F, 0xFF
};
IPAddress ip(10, 0, 0, 4);

// Initialize the Ethernet server library at port 80
EthernetServer server(80);

enum states {
  GET,
  WAITFOREND,
  RESPONSE
} state = GET;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for serial port to connect. Needed for Leonardo only
  }

  // Start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  // Listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // An http request ends with a blank line
    boolean currentLineIsBlank = true;
    state = GET;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // If you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // Send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // The connection will be closed after completion of the response
          client.println();
          client.println("<!DOCTYPE HTML><html>");
          if (switchOn) {
            client.println("Switch ON");
          } else {
            client.println("Switch OFF");
          }
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // You're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // You've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // Give the web browser time to receive the data
    delay(1);
    // Close the connection:
    client.stop();
    Serial.println("client disconnected");
  }

  // Write the output
  digitalWrite(pin, switchOn);
}

