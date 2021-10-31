#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

// size of buffer used to capture HTTP requests
#define REQ_BUF_SZ   40

int i;
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0F, 0x26, 0x06 };
EthernetServer server(80); 
File webFile;
char HTTP_req[REQ_BUF_SZ] = {0}; // buffered HTTP request stored as null terminated string
char req_index = 0;              // index into HTTP_req buffer

void setup()
{
    i=0;
    // disable Ethernet chip
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);
    
    Serial.begin(9600);       // for debugging
    
    Serial.println("Initializing SD card...");
    if (!SD.begin(4)) {
        Serial.println("ERROR - SD card initialization failed!");
        return;    // init failed
    }
    Serial.println("SUCCESS - SD card initialized.");

    Ethernet.begin(mac);  // initialize Ethernet device
    server.begin();        // start to listen for clients

    Serial.print("server is at ");
    Serial.println(Ethernet.localIP());
}

void loop()
{
    EthernetClient client = server.available();  // try to get client

    if (client) {  // got client?
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                // buffer first part of HTTP request in HTTP_req array (string)
                // leave last element in array as 0 to null terminate string (REQ_BUF_SZ - 1)
                if (req_index < (REQ_BUF_SZ - 1)) {
                    HTTP_req[req_index] = c;          // save HTTP request character
                    req_index++;
                }
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank) {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");

                    if (StrContains(HTTP_req, "GET /style.css"))
                    {
            client.println("Content-Type: text/css");
            client.println("Connection: keep-alive");
            client.println();
              
            loadFile("style.css", client);
                    }
                    else if (StrContains(HTTP_req, "GET /script.js"))
                    {
            client.println("Content-Type: text/javascript");
            client.println("Connection: keep-alive");
            client.println();

            loadFile("script.js", client);
                    }
                    else if (StrContains(HTTP_req, "ajaxRequest")) {
                        client.println("Content-Type: text/xml");
                        client.println("Connection: keep-alive");
                        client.println();

                        client.println("<?xml version = \"1.0\" ?>");
                        loadFile("data2.txt", client);
                    }
                    else {  
                        client.println("Content-Type: text/html");
                        client.println("Connection: keep-alive");
                        client.println();

                        loadFile("index.htm", client);
                    }
                    // display received HTTP request on serial port
                    Serial.println(HTTP_req);
                    // reset buffer index and all buffer elements to 0
                    req_index = 0;
                    StrClear(HTTP_req, REQ_BUF_SZ);
                    break;
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n') {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                } 
                else if (c != '\r') {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
        } // end while (client.connected())
        delay(1);      // give the web browser time to receive the data
        client.stop(); // close the connection
    } // end if (client)
}

void StrClear(char *str, char length)
{
    for (int i = 0; i < length; i++) {
        str[i] = 0;
    }
}

char StrContains(char *str, char *sfind)
{
    char found = 0;
    char index = 0;
    char len;

    len = strlen(str);
    
    if (strlen(sfind) > len) {
        return 0;
    }
    while (index < len) {
        if (str[index] == sfind[found]) {
            found++;
            if (strlen(sfind) == found) {
                return 1;
            }
        }
        else {
            found = 0;
        }
        index++;
    }
    return 0;
}

void loadFile(String fileName, EthernetClient cl)
{
  webFile = SD.open(fileName);

  if (webFile) 
  {
    while (webFile.available()) 
    {
       cl.write(webFile.read());
    }
    webFile.close();
  }
  else {
  Serial.println("error opening the file");
  }
}
