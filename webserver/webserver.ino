// Basic Arduino Web Server version 0.1 
// 27 May 2015
// References and further information at:
// http://startingelectronics.org/software/arduino/web-server/basic-01/
#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

// maximum length of file name including path
#define FILE_NAME_LEN  20
// HTTP request type
#define HTTP_invalid   0
#define HTTP_GET       1
#define HTTP_POST      2

// file types
#define FT_HTML       0
#define FT_ICON       1
#define FT_CSS        2
#define FT_JAVASCRIPT 3
#define FT_JPG        4
#define FT_PNG        5
#define FT_GIF        6
#define FT_TEXT       7
#define FT_INVALID    8

// pin used for Ethernet chip SPI chip select
#define PIN_ETH_SPI   10

#define REQ_BUF_SZ    50

int up = 12;
int down = 11;
int left = 2;
int right = 9;
int ch1 = 8;
int ch2 = 7;
int ch3 = 6;
int ch4 = 5;
int ch5 = 3;
int cmd1 = 21;
int cmd2 = 23;
int cmd3 = 25;
int cmd4 = 27;
int cmd5 = 29;
int in1 = 31;
int in2 = 33;
int in3 = 35;
int in4 = 37;
int in5 = 39;
int in6 = 41;
int in7 = 43;
int in8 = 45;
int chFS = 47;
int chVH = 40;
int pcb = 46;

// the media access control (ethernet hardware) address for the shield:
//const byte mac[] PROGMEM = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//the IP address for the shield:
//const byte ip[] = { 192, 168, 5, 178 };  // does not work if this is put into Flash
// the router's gateway address:
//const byte gateway[] PROGMEM = { 192, 168, 5, 1 };
// the subnet:
//const byte subnet[] PROGMEM = { 255, 255, 255, 0 };

byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};
IPAddress ip(192, 168, 5, 177);
IPAddress myDns(192,168,5, 1);
IPAddress gateway(192, 168, 5, 1);
IPAddress subnet(255, 255, 255, 0);

EthernetServer server(80);

void setup() {
  // deselect Ethernet chip on SPI bus
  pinMode(PIN_ETH_SPI, OUTPUT);
  digitalWrite(PIN_ETH_SPI, HIGH);
  
  Serial.begin(9600);       // for debugging

  pinMode(up, OUTPUT);
  pinMode(left, OUTPUT);
  pinMode(right, OUTPUT);
  pinMode(down, OUTPUT);
  pinMode(ch1, OUTPUT);
  pinMode(ch2, OUTPUT);
  pinMode(ch3, OUTPUT);
  pinMode(ch4, OUTPUT);
  pinMode(ch5, OUTPUT);
  pinMode(cmd1, OUTPUT);
  pinMode(cmd2, OUTPUT);
  pinMode(cmd3, OUTPUT);
  pinMode(cmd4, OUTPUT);
  pinMode(cmd5, OUTPUT);
  pinMode(chVH, OUTPUT);
  pinMode(chFS, OUTPUT);
  pinMode(pcb, OUTPUT);
  pinMode(in1, INPUT_PULLUP);
  pinMode(in2, INPUT_PULLUP);
  pinMode(in3, INPUT_PULLUP);
  pinMode(in4, INPUT_PULLUP);
  pinMode(in5, INPUT_PULLUP);
  pinMode(in6, INPUT_PULLUP);
  pinMode(in7, INPUT_PULLUP);
  pinMode(in8, INPUT_PULLUP);
  
  if (!SD.begin(4)) {
    return;  // SD card initialization failed
  }

  // start the Ethernet connection:
  Serial.println("Trying to get an IP address using DHCP");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // initialize the Ethernet device not using DHCP:
    Ethernet.begin(mac, ip, myDns, gateway, subnet);
  }
  // print your local IP address:
  Serial.print("My IP address: ");
  ip = Ethernet.localIP();
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(ip[thisByte], DEC);
    Serial.print(".");
  }
  Serial.println();
  // start listening for clients
  server.begin();

  
  //Ethernet.begin((uint8_t*)mac, ip, gateway, subnet);
  //server.begin();  // start listening for clients

  initialization();
}

void initialization() {
  digitalWrite(up, LOW);
  digitalWrite(left, LOW);
  digitalWrite(right, LOW);
  digitalWrite(down, LOW);
  digitalWrite(ch1, LOW);
  digitalWrite(ch2, LOW);
  digitalWrite(ch3, LOW);
  digitalWrite(ch4, LOW);
  digitalWrite(ch5, LOW);
  digitalWrite(cmd1, LOW);
  digitalWrite(cmd2, LOW);
  digitalWrite(cmd3, LOW);
  digitalWrite(cmd4, LOW);
  digitalWrite(cmd5, LOW);
  digitalWrite(chVH, HIGH);
  digitalWrite(chFS, LOW);
  digitalWrite(pcb, LOW);
}

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void loop() {
  // if an incoming client connects, there will be bytes available to read:
  EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (ServiceClient(&client)) {
        // received request from client and finished responding
        break;
      }
    }  // while (client.connected())
    delay(1);
    client.stop();
  }  // if (client)
}// void loop()

bool ServiceClient(EthernetClient *client)
{
  static boolean currentLineIsBlank = true;
  char cl_char;
  File webFile;
  // file name from request including path + 1 of null terminator
  char file_name[FILE_NAME_LEN + 1] = {0};  // requested file name
  char http_req_type = 0;
  char req_file_type = FT_INVALID;
  const char *file_types[] = {"text/html", "image/x-icon", "text/css", "application/javascript", "image/jpeg", "image/png", "image/gif", "text/plain"};
  
  static char req_line_1[REQ_BUF_SZ] = {0};  // stores the first line of the HTTP request
  static char HTTP_req[REQ_BUF_SZ] = {0};  // stores the first line of the HTTP request

  static unsigned char req_line_index = 0;
  static bool got_line_1 = false;

  if (client->available()) {   // client data available to read
    cl_char = client->read();
    
    if ((req_line_index < REQ_BUF_SZ-1) && (got_line_1 == false)) {
      if ((cl_char != '\r') && (cl_char != '\n')) {
        req_line_1[req_line_index] = cl_char;
        req_line_index++;
      }
      else {
        got_line_1 = true;
        req_line_1[REQ_BUF_SZ-1] = 0;
      }
    }
   
    if ((cl_char == '\n') && currentLineIsBlank) {
      // get HTTP request type, file name and file extension type index
      strncpy(HTTP_req, req_line_1, REQ_BUF_SZ);
      
      http_req_type = GetRequestedHttpResource(req_line_1, file_name, &req_file_type);
      if (http_req_type == HTTP_GET) {         // HTTP GET request
        if (req_file_type < FT_INVALID) {      // valid file type
          webFile = SD.open(file_name);        // open requested file
          if (webFile) {
            // send a standard http response header
            client->println(F("HTTP/1.1 200 OK"));
            client->print(F("Content-Type: "));
            client->println(file_types[req_file_type]);
            client->println(F("Connection: close"));
            client->println();
            // send web page
            while(webFile.available()) {
              int num_bytes_read;
              char byte_buffer[64];
              // get bytes from requested file
              num_bytes_read = webFile.read(byte_buffer, 64);
              // send the file bytes to the client
              client->write(byte_buffer, num_bytes_read);
            }
            webFile.close();
          }
          else {
          }
        }
        else {
          Serial.println(HTTP_req);
          
          client->println(F("HTTP/1.1 200 OK"));
          client->println(F("Content-Type: application/json; charset=UTF-8"));
          client->println(F("Connection: close"));
          client->println();

          if (StrContains(HTTP_req, "GET /monitor_values")) {
            int angle_left_right = analogRead(1) * 0.3519;
            int monitor_signal = analogRead(2) * 0.0977;

            client->print(F("{"));
            client->print(F("\"monitor_up_down\":"));       client->print(analogRead(0)); 
            client->print(F(",\"monitor_left_right\":\"")); client->print(angle_left_right); client->print(F("&#176;\""));
            client->print(F(",\"monitor_signal\":"));       client->print(monitor_signal);
            client->print(F(",\"port1\":"));    if (digitalRead(in1) == 0) { client->print(F("\"red\"")); } else { client->print(F("\"lightgray\"")); }
            client->print(F(",\"port2\":"));    if (digitalRead(in2) == 0) { client->print(F("\"red\"")); } else { client->print(F("\"lightgray\"")); }
            client->print(F(",\"port3\":"));    if (digitalRead(in3) == 0) { client->print(F("\"red\"")); } else { client->print(F("\"lightgray\"")); }
            client->print(F(",\"port4\":"));    if (digitalRead(in4) == 0) { client->print(F("\"red\"")); } else { client->print(F("\"lightgray\"")); }
            client->print(F(",\"port5\":"));    if (digitalRead(in5) == 0) { client->print(F("\"red\"")); } else { client->print(F("\"lightgray\"")); }
            client->print(F(",\"port6\":"));    if (digitalRead(in6) == 0) { client->print(F("\"red\"")); } else { client->print(F("\"lightgray\"")); }
            client->print(F(",\"port7\":"));    if (digitalRead(in7) == 0) { client->print(F("\"red\"")); } else { client->print(F("\"lightgray\"")); }
            client->print(F(",\"port8\":"));    if (digitalRead(in8) == 0) { client->print(F("\"red\"")); } else { client->print(F("\"lightgray\"")); }
            client->print(F(",\"channel1\":")); if (digitalRead(ch1) == HIGH) { client->print(F("\"green\"")); } else { client->print(F("\"#293F5E\"")); }
            client->print(F(",\"channel2\":")); if (digitalRead(ch2) == HIGH) { client->print(F("\"green\"")); } else { client->print(F("\"#293F5E\"")); }
            client->print(F(",\"channel3\":")); if (digitalRead(ch3) == HIGH) { client->print(F("\"green\"")); } else { client->print(F("\"#293F5E\"")); }
            client->print(F(",\"channel4\":")); if (digitalRead(ch4) == HIGH) { client->print(F("\"green\"")); } else { client->print(F("\"#293F5E\"")); }
            client->print(F(",\"channel5\":")); if (digitalRead(ch5) == HIGH) { client->print(F("\"green\"")); } else { client->print(F("\"#293F5E\"")); }
            client->print(F(",\"command1\":")); if (digitalRead(cmd1) == HIGH) { client->print(F("\"green\"")); } else { client->print(F("\"#293F5E\"")); }
            client->print(F(",\"command2\":")); if (digitalRead(cmd2) == HIGH) { client->print(F("\"green\"")); } else { client->print(F("\"#293F5E\"")); }
            client->print(F(",\"command3\":")); if (digitalRead(cmd3) == HIGH) { client->print(F("\"green\"")); } else { client->print(F("\"#293F5E\"")); }
            client->print(F(",\"command4\":")); if (digitalRead(cmd4) == HIGH) { client->print(F("\"green\"")); } else { client->print(F("\"#293F5E\"")); }
            client->print(F(",\"command5\":")); if (digitalRead(cmd5) == HIGH) { client->print(F("\"green\"")); } else { client->print(F("\"#293F5E\"")); }
            client->print(F(",\"polarV\":"));   if (digitalRead(chVH) == HIGH) { client->print(F("\"green\"")); } else { client->print(F("\"#293F5E\"")); }
            client->print(F(",\"polarH\":"));   if (digitalRead(chVH) == LOW) { client->print(F("\"green\"")); } else { client->print(F("\"#293F5E\"")); }
            client->print(F(",\"speedF\":"));   if (digitalRead(chFS) == LOW) { client->print(F("\"green\"")); } else { client->print(F("\"#293F5E\"")); }
            client->print(F(",\"speedS\":"));   if (digitalRead(chFS) == HIGH) { client->print(F("\"green\"")); } else { client->print(F("\"#293F5E\"")); }
            client->print(F(",\"angle\":")); client->print(angle_left_right);
            client->print(F("}"));          
          }
          else if (StrContains(HTTP_req, "GET /set_channel")) {
            String channel = HTTP_req;
            channel.replace("GET /set_channel?channel=", "");
            channel.replace(" HTTP/1.1", "");
            Serial.println(channel);

            if (channel == "1") { digitalWrite(ch1, HIGH); } else { digitalWrite(ch1, LOW); } delay(1);
            if (channel == "2") { digitalWrite(ch2, HIGH); } else { digitalWrite(ch2, LOW); } delay(1);
            if (channel == "3") { digitalWrite(ch3, HIGH); } else { digitalWrite(ch3, LOW); } delay(1);
            if (channel == "4") { digitalWrite(ch4, HIGH); } else { digitalWrite(ch4, LOW); } delay(1);
            if (channel == "5") { digitalWrite(ch5, HIGH); } else { digitalWrite(ch5, LOW); } delay(1);
          }
          else if (StrContains(HTTP_req, "GET /set_command")) {
            String command = HTTP_req;
            command.replace("GET /set_command?command=", "");
            command.replace(" HTTP/1.1", "");
            Serial.println(command);

            if (command == "1") { 
              if (digitalRead(cmd1) == LOW ) { digitalWrite(cmd1, HIGH); } else { digitalWrite(cmd1, LOW); } delay(1);
            }
            if (command == "2") {
              if (digitalRead(cmd2) == LOW ) { digitalWrite(cmd2, HIGH); } else { digitalWrite(cmd2, LOW); } delay(1);
            }
            if (command == "3") {
              if (digitalRead(cmd3) == LOW ) { digitalWrite(cmd3, HIGH); } else { digitalWrite(cmd3, LOW); } delay(1);
            }
            if (command == "4") {
              if (digitalRead(cmd4) == LOW ) { digitalWrite(cmd4, HIGH); } else { digitalWrite(cmd4, LOW); } delay(1);
            }
            if (command == "5") {
              if (digitalRead(cmd5) == LOW ) { digitalWrite(cmd5, HIGH); } else { digitalWrite(cmd5, LOW); } delay(1);
            }
          }else if (StrContains(HTTP_req, "GET /set_polar")) {
            String channel = HTTP_req;
            channel.replace("GET /set_polar?value=", "");
            channel.replace(" HTTP/1.1", "");
            Serial.println(channel);

            if (channel == "V") { digitalWrite(chVH, HIGH); } delay(1);
            if (channel == "H") { digitalWrite(chVH, LOW); } delay(1);            
          }
          else if (StrContains(HTTP_req, "GET /set_speed")) {
            String channel = HTTP_req;
            channel.replace("GET /set_speed?value=", "");
            channel.replace(" HTTP/1.1", "");
            Serial.println(channel);

            if (channel == "F") { digitalWrite(chFS, LOW); } delay(1);
            if (channel == "S") { digitalWrite(chFS, HIGH); } delay(1);            
          }
          else if (StrContains(HTTP_req, "GET /set_position")) {
            String position = HTTP_req;
            position.replace("GET /set_position?position=", "");
            position.replace(" HTTP/1.1", "");
            Serial.println(position);

            if (position == "up"   ) { digitalWrite(up, HIGH); } 
            if (position == "down" ) { digitalWrite(down, HIGH); } 
            if (position == "left" ) { digitalWrite(left, HIGH); } 
            if (position == "right") { digitalWrite(right, HIGH); } 
            
            delay(400);
            
            if (position == "up"   ) { digitalWrite(up, LOW); } 
            if (position == "down" ) { digitalWrite(down, LOW); } 
            if (position == "left" ) { digitalWrite(left, LOW); } 
            if (position == "right") { digitalWrite(right, LOW); } 
          }
          else if (StrContains(HTTP_req, "GET /set_multi_position")) {
            String position = HTTP_req;
            position.replace("GET /set_multi_position?position=", "");
            position.replace(" HTTP/1.1", "");
            Serial.println(position);

            if (position == "up"   ) { digitalWrite(up, HIGH); } 
            if (position == "down" ) { digitalWrite(down, HIGH); } 
            if (position == "left" ) { digitalWrite(left, HIGH); } 
            if (position == "right") { digitalWrite(right, HIGH); } 
            
            delay(1700);
            
            if (position == "up"   ) { digitalWrite(up, LOW); } 
            if (position == "down" ) { digitalWrite(down, LOW); } 
            if (position == "left" ) { digitalWrite(left, LOW); } 
            if (position == "right") { digitalWrite(right, LOW); } 
          }
          else if (StrContains(HTTP_req, "GET /set_angle")) {
            String angle = HTTP_req;
            angle.replace("GET /set_angle?angle=", "");
            angle.replace(" HTTP/1.1", "");
            Serial.println(angle);
            
            int new_angle = angle.toInt() / 0.3519;
            
            int moveto_angle = new_angle - analogRead(1);

            int originFS = digitalRead(chFS);
            digitalWrite(chFS, HIGH);
            
            int loops = 0;
            
            int angle_left_right = analogRead(1) * 0.3519;
            
            if ( angle.toInt() != angle_left_right) {
              
              if (moveto_angle > 0) {
                while (( (new_angle) >= analogRead(1)) && (loops < 68))
                  {
                    Serial.print(new_angle); Serial.print(">="); Serial.println(analogRead(1));
                    digitalWrite(right, HIGH);
                    delay(800);
                    loops ++;
                    digitalWrite(right, LOW);
                  }
                } 
                else {
                  while (( (new_angle) <= analogRead(1)) && (loops < 68))
                  {
                    Serial.print(new_angle); Serial.print("<="); Serial.println(analogRead(1));
                    digitalWrite(left, HIGH);
                    delay(800);
                    loops ++;
                    digitalWrite(left, LOW);
                  }
                }    
            }
            
            digitalWrite(chFS, originFS);
          }
          else if (StrContains(HTTP_req, "GET /reset_cpu")) {
            String channel = HTTP_req;
            channel.replace("GET /reset_cpu", "");
            channel.replace(" HTTP/1.1", "");
            Serial.println(channel);

            resetFunc();
          }
          else if (StrContains(HTTP_req, "GET /reset_pcb")) {
            String channel = HTTP_req;
            channel.replace("GET /reset_pcb", "");
            channel.replace(" HTTP/1.1", "");
            Serial.println(channel);

            digitalWrite(pcb, HIGH);
            delay(3000);
            digitalWrite(pcb, LOW);
          }

        }
      }
      else if (http_req_type == HTTP_POST) {
        // a POST HTTP request was received
      }
      else {
        // unsupported HTTP request received
      }
      //req_line_1[0] = 0;
      StrClear(req_line_1, REQ_BUF_SZ);
      req_line_index = 0;
      got_line_1 = false;
      // finished sending response and web page
      return 1;
    }
    if (cl_char == '\n') {
      currentLineIsBlank = true; 
    }
    else if (cl_char != '\r') {
      currentLineIsBlank = false;
    }
  }  // if (client.available())
  return 0;
}

void StrClear(char *str, char length)
{
    for (int i = 0; i < length; i++) {
        str[i] = 0;
    }
}

// searches for the string sfind in the string str
// returns 1 if string found
// returns 0 if string not found
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

// extract file name from first line of HTTP request
char GetRequestedHttpResource(char *req_line, char *file_name, char *file_type)
{
  char request_type = HTTP_invalid;  // 1 = GET, 2 = POST. 0 = invalid
  char *str_token;
  
  *file_type = FT_INVALID;
  
  str_token =  strtok(req_line, " ");    // get the request type
  if (strcmp(str_token, "GET") == 0) {
    request_type = HTTP_GET;
    str_token =  strtok(NULL, " ");      // get the file name
    if (strcmp(str_token, "/") == 0) {
      strcpy(file_name, "index.htm");
      *file_type = FT_HTML;
    }
    else if (strlen(str_token) <= FILE_NAME_LEN) {
      // file name is within allowed length
      strcpy(file_name, str_token);
      // get the file extension
      str_token = strtok(str_token, ".");
      str_token = strtok(NULL, ".");
      
      if      (strcmp(str_token, "htm") == 0) {*file_type = 0;}
      else if (strcmp(str_token, "ico") == 0) {*file_type = 1;}
      else if (strcmp(str_token, "css") == 0) {*file_type = 2;}
      else if (strcmp(str_token, "js")  == 0) {*file_type = 3;}
      else if (strcmp(str_token, "jpg") == 0) {*file_type = 4;}
      else if (strcmp(str_token, "png") == 0) {*file_type = 5;}
      else if (strcmp(str_token, "gif") == 0) {*file_type = 6;}
      else if (strcmp(str_token, "txt") == 0) {*file_type = 7;}
      else {*file_type = 8;}
    }
    else {
      // file name too long
    }
  }
  else if (strcmp(str_token, "POST") == 0) {
    request_type = HTTP_POST;
  }

  return request_type;
}
