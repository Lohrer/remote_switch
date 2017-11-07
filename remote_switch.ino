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
  RESPONSE
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
  int linecnt = 0;
  
  // Listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");

    state = GET;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);

        switch (state) {
          case GET:
          {
            int num_requests = sizeof(REQUEST_LINES) / sizeof(char*);

            // Increment counter if it matches one of the supported requests.
            request = NONE;
            for (int i = 0; i < num_requests; i++) {
              if (c == REQUEST_LINES[i][linecnt]) {
                linecnt++;
                request = (requests)i;
                break;
              }
            }

            // If no matching request line was found reject the request.
            if (request == NONE) {
              state = WAITFOREND;
              Serial.println("invalid request");
              break;
            }

            // If at the end of the request strings, update state
            if (linecnt >= REQUEST_LEN-1) {
              //Serial.printf("Request %d\n", (int)request);
            }
            break;
          }
          case WAITFOREND:
            
            break;
          case RESPONSE:
            
            break;
        }
        // If you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
//        if (c == '\n' && currentLineIsBlank) {
//          // Send a standard http response header
//          client.println("HTTP/1.1 200 OK");
//          client.println("Content-Type: text/html");
//          client.println("Connection: close");  // The connection will be closed after completion of the response
//          client.println();
//          client.println("<!DOCTYPE HTML><html>");
//          if (switchOn) {
//            client.println("Switch ON");
//          } else {
//            client.println("Switch OFF");
//          }
//          client.println("</html>");
//          break;
//        }

        
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

