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
int outpin = 0;
int ledpin = 13;
bool switchOn = false;

// Set the MAC address to what is printed on the shield
// Set the IP address to 10.0.0.4
byte mac[] = {
  0x90, 0xA2, 0xDA, 0x0F, 0x27, 0x38
};
IPAddress ip(10, 0, 0, 4);

// Initialize the Ethernet server library at port 80
EthernetServer server(80);

enum states {
  GET,
  WAITFOREND,
  DONE
} state;

// Start of GET line for all available requests,
// these strings must be the same length
char* GET_INDEX = "GET / HT";
char* GET_ON    = "GET /on ";
char* GET_OFF   = "GET /off";
char* REQUEST_LINES[] = {GET_INDEX, GET_ON, GET_OFF};
int REQUEST_LEN = strlen(GET_INDEX);
enum requests {
  INDEX,
  ON,
  OFF,
  NONE
} request;

// Turn on/off the output
void turn_on(bool on) {
  digitalWrite(outpin, on);
  digitalWrite(ledpin, on);
}

void update_state(EthernetClient client) {
  char c = client.read();
  //Serial.write(c);
  
  switch (state) {
    case GET: {
      static int linecnt = 0;
      
      // Increment counter if it matches one of the supported requests.
      request = NONE;
      int num_request_types = sizeof(REQUEST_LINES) / sizeof(char*);
      for (int i = 0; i < num_request_types; i++) {
        if (c == REQUEST_LINES[i][linecnt]) {
          linecnt++;
          request = (requests)i;
          break;
        }
      }

      // If at the end of the request strings, update state
      if (linecnt >= REQUEST_LEN) {
        linecnt = 0;
        state = WAITFOREND;
      }
      break;
    }
    case WAITFOREND: {
      static char last[3] = {0};
      if (last[2] == '\r' && last[1] == '\n' && last[0] == '\r' && c == '\n') {
        // Send response
        switch (request) {
          case INDEX:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println("Refresh: 5");
            client.println();
            client.println("<!DOCTYPE HTML><html>");
            if (switchOn) {
              client.println("Switch ON");
            } else {
              client.println("Switch OFF");
            }
            client.println("</html>");
            Serial.println("sent webpage");
            break;
          case ON:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            client.println("<!DOCTYPE HTML><html>Switch ON</html>");
            Serial.println("turning on");
            switchOn = true;
            break;
          case OFF:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            client.println("<!DOCTYPE HTML><html>Switch OFF</html>");
            Serial.println("turning off");
            switchOn = false;
            break;
          case NONE:
            client.println("HTTP/1.1 400 Bad Request");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            client.println("<!DOCTYPE HTML><html>Invalid request.</html>");
            Serial.println("invalid request received");
            break;
        }
        state = GET;
      }
      last[2] = last[1];
      last[1] = last[0];
      last[0] = c;
      break;
    }
    case DONE: break;
  }
}

void setup() {
  // Initialize I/O
  turn_on(false);
  
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
  
  // Enable watchdog
  //wdt_enable(WDTO_1S);
}

void loop() {
  // Listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");

    while (client.connected()) {
      if (client.available()) {
        update_state(client);
      }
    }
    // Give the web browser time to receive the data
    delay(1);
    // Close the connection:
    client.stop();
    Serial.println("client disconnected");
  }

  // Write the output
  turn_on(switchOn);
  
  // Reset watchdog timer
  //wdt_reset();
}

