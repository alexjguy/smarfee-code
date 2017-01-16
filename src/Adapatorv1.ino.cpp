# 1 "/var/folders/gd/1l6q66k50_g7fr5cdr3dyzbh0000gn/T/tmpXlGC2D"
#include <Arduino.h>
# 1 "/Users/alexandru/Documents/Arduino/arduinofeeder/src/Adapatorv1.ino"
#include <FS.h>

#include <Arduino.h>

#include <WiFiServer.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);
#include <ESP8266mDNS.h>
#include <Servo.h>
Servo servoMain;

#include <WiFiUdp.h>
#include <SPI.h>
#include <PubSubClient.h>

#include <pgmspace.h>
#include <TimeAlarms.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>



const int IN1 = 16;
const int IN2 = 5;
const int IN3 = 4;
const int IN4 = 0;


const int button1 = 12;
boolean buttonActive = false;
boolean longPressActive = false;
boolean button1Active = false;
long buttonTimer = 0;
long buttonTime = 3000;



int buttonState = 0;
int lastButtonState = 0;
int startPressed = 0;
int endPressed = 0;
int timeHold = 0;
int timeReleased = 0;



const int ledPin = 14;



unsigned int localPort = 2390;




IPAddress timeServerIP;
const char* ntpServerName = "ro.pool.ntp.org";

const int NTP_PACKET_SIZE = 48;

byte packetBuffer[ NTP_PACKET_SIZE];


WiFiUDP udp;


const char* ssid = "athome_24";
const char* password = "athome#$";

const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

char mqtt_server[40] = "192.168.0.44";
char mqtt_port[6] = "8080";



bool shouldSaveConfig = true;
void saveConfigCallback ();
void setup(void);
void loop(void);
void MorningAlarm();
void EveningAlarm();
void digitalClockDisplay();
void printDigits(int digits);
void ledState(const char* action);
void httpServerSetup();
void handleLogin();
void handleRoot();
void handleConfig();
void handleTools();
void handleNotFound();
void addHeader(boolean showMenu, String& str);
void addFooter(String& str);
bool is_authentified();
void callback(char* topic, byte* payload, unsigned int length);
void mqttSetup();
boolean reconnect();
void mqtt_loop();
void mqtt_send(char* message);
void servoFeed(void);
void stepperFeed(int i, int j);
unsigned long sendNTPpacket(IPAddress& address);
void do_sendNTPpacket();
#line 84 "/Users/alexandru/Documents/Arduino/arduinofeeder/src/Adapatorv1.ino"
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

extern "C" {
#include "user_interface.h"
}


void setup(void) {

  system_update_cpu_freq(160);

  pinMode(ledPin, OUTPUT);
  ledState("blinkOnce");

  Serial.begin(115200);

  Serial.println("");






    Serial.println("mounting FS...");

    if (SPIFFS.begin()) {
      Serial.println("mounted file system");
      if (SPIFFS.exists("/config.json")) {

        Serial.println("reading config file");
        File configFile = SPIFFS.open("/config.json", "r");
        if (configFile) {
          Serial.println("opened config file");
          size_t size = configFile.size();

          std::unique_ptr<char[]> buf(new char[size]);

          configFile.readBytes(buf.get(), size);
          DynamicJsonBuffer jsonBuffer;
          JsonObject& json = jsonBuffer.parseObject(buf.get());
          json.printTo(Serial);
          if (json.success()) {
            Serial.println("\nparsed json");

            strcpy(mqtt_server, json["mqtt_server"]);
            strcpy(mqtt_port, json["mqtt_port"]);

          } else {
            Serial.println("failed to load json config");
          }
        }
      }
    } else {
      Serial.println("failed to mount FS");
    }







    WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
    WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);



    WiFiManager wifiManager;


    wifiManager.setSaveConfigCallback(saveConfigCallback);





    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_port);
# 176 "/Users/alexandru/Documents/Arduino/arduinofeeder/src/Adapatorv1.ino"
    wifiManager.setTimeout(300);





    if (!wifiManager.autoConnect("SmartFeeder","1234567890")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);

      ESP.reset();
      delay(5000);
    }


      Serial.println("connected...yeey :)");


      strcpy(mqtt_server, custom_mqtt_server.getValue());
      strcpy(mqtt_port, custom_mqtt_port.getValue());


      if (shouldSaveConfig) {
        Serial.println("saving config");
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.createObject();
        json["mqtt_server"] = mqtt_server;
        json["mqtt_port"] = mqtt_port;

        File configFile = SPIFFS.open("/config.json", "w");
        if (!configFile) {
          Serial.println("failed to open config file for writing");
        }

        json.printTo(Serial);
        json.printTo(configFile);
        configFile.close();

      }

      Serial.println("local ip");
      Serial.println(WiFi.localIP());


  httpServerSetup();



  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
  do_sendNTPpacket();

  mqttSetup();

  Alarm.alarmRepeat(7,00,0, MorningAlarm);
  Alarm.alarmRepeat(17,30,0,EveningAlarm);
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);

  pinMode(button1, INPUT);
  ledState("blinkTwice");
  ledState("On");


}

void loop(void) {




  for (int timeCount = 0; timeCount < 100; timeCount++){
    server.handleClient();
    delay(100);




     buttonState = digitalRead(button1);


     if (buttonState != lastButtonState) {


         if (buttonState == HIGH) {
             startPressed = millis();
             timeReleased = startPressed - endPressed;

             if (timeReleased >= 500 && timeReleased < 1000) {
                 Serial.println("Button idle for half a second");
             }

             if (timeReleased >= 2800 && timeReleased <=6000) {
                 Serial.println("Button idle for 3 seconds");
             }

             if (timeReleased >= 9000 && timeReleased <= 10500) {
                 Serial.println("Button idle for 10 seconds");
             }


         } else {
             endPressed = millis();
             timeHold = endPressed - startPressed;

             if (timeHold >= 100 && timeHold < 1000) {
                 Serial.println("Button hold for less than a second");
                 stepperFeed(2048,1);
             }

             if (timeHold >= 2800 && timeHold <= 6000) {
                 Serial.println("Button hold for 3 seconds");
                 yield();
             }

             if (timeHold >= 9000 && timeHold <= 10500) {
                 Serial.println("Button hold for 10 seconds");
             }

         }

     }



     lastButtonState = buttonState;



  }
  ESP.getCpuFreqMHz();


}
# 1 "/Users/alexandru/Documents/Arduino/arduinofeeder/src/Alarms.ino"
#include <Arduino.h>



void MorningAlarm(){
  Serial.println("Alarm: - Mio morning Feed");

  stepperFeed(2048,1);
}

void EveningAlarm(){
  Serial.println("Alarm: - Mio evening Feed");

  stepperFeed(2048,1);
}
# 33 "/Users/alexandru/Documents/Arduino/arduinofeeder/src/Alarms.ino"
void digitalClockDisplay()
{

  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println();
}

void printDigits(int digits)
{
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
# 1 "/Users/alexandru/Documents/Arduino/arduinofeeder/src/Led.ino"
#include <Arduino.h>

void ledState(const char* action){
if (action == "blinkOnce"){
  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);
} else if(action == "blinkTwice"){
  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);
  delay(1000);
  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);
} else if(action == "blinkTrice"){
  ledState("blinkOnce");
  delay(1000);
  ledState("blinkTwice");
  delay(1000);
} else if(action == "On"){
  digitalWrite(ledPin, HIGH);
} else if(action == "Off"){
  digitalWrite(ledPin, HIGH);
}

}
# 1 "/Users/alexandru/Documents/Arduino/arduinofeeder/src/WebServer.ino"
#include <Arduino.h>

void httpServerSetup(){

  server.on("/", handleRoot);
  server.on("/login", handleLogin);
  server.on("/tools", handleTools);
  server.on("/config", handleConfig);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works without need of authentification");
  });


  server.on("/firmware", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/html", serverIndex);
  });

  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.setDebugOutput(true);
      WiFiUDP::stopAll();
      Serial.printf("Update: %s\n", upload.filename.c_str());
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      if (!Update.begin(maxSketchSpace)) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
      Serial.setDebugOutput(false);
    }
    yield();
  });

  server.onNotFound(handleNotFound);

  const char * headerkeys[] = {"User-Agent", "Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);

  server.collectHeaders(headerkeys, headerkeyssize );

  server.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.println("HTTP server started");
}


void handleLogin() {
  String msg;
  if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
  }
  if (server.hasArg("DISCONNECT")) {
    Serial.println("Disconnection");
    String header = "HTTP/1.1 301 OK\r\nSet-Cookie: ESPSESSIONID=0\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }
  if (server.hasArg("USERNAME") && server.hasArg("PASSWORD")) {
    if (server.arg("USERNAME") == "admin" && server.arg("PASSWORD") == "start123" ) {
      String header = "HTTP/1.1 301 OK\r\nSet-Cookie: ESPSESSIONID=1\r\nLocation: /\r\nCache-Control: no-cache\r\n\r\n";
      server.sendContent(header);
      Serial.println("Log in Successful");
      return;
    }
    msg = "Wrong username/password! try again.";
    Serial.println("Log in Failed");
  }
    String content = "";
    addHeader(false, content);




  content += "<form action='/login' method='POST'>Please login to access Smart Feeder:<br>";
  content += "User:<input type='text' name='USERNAME' placeholder='user name'><br>";
  content += "Password:<input type='password' name='PASSWORD' placeholder='password'><br>";
  content += "<input type='submit' name='SUBMIT' value='Submit'></form>" + msg + "<br>";
  content += "</body></html>";
  server.send(200, "text/html", content);
}



void handleRoot() {
  Serial.println("Enter handleRoot");
  String header;
  if (!is_authentified()) {
    String header = "HTTP/1.1 301 OK\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }

  String cmd;
  if (server.hasArg("cmd")){
    if (server.arg("cmd") == "feed")
    {
      String header = "HTTP/1.1 301 OK\r\nCmd: /\r\nCache-Control: no-cache\r\n\r\n";

      stepperFeed(2048,1);
      server.sendContent(header);
      return;
    }
  }
    String reply = "";
    addHeader(true, reply);
    reply += F("<form>");
    reply += F("<table><TH>System Info<TH><TH>");

      reply += F("<TR><TD>System Time:<TD>");
      reply += hour();
      reply += ":";
      if (minute() < 10)
        reply += "0";
      reply += minute();


     reply += F("<TR><TD>Tools<TD><a class=\"button-link\" href=\"?cmd=feed\">Feed Pet</a>");

     reply += F("</table></form>");

   addFooter(reply);



  server.send(200, "text/html", reply);
}


void handleConfig() {
  Serial.println("Enter handleConfig");
  String header;
  if (!is_authentified()) {
    String header = "HTTP/1.1 301 OK\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }

  String cmd;
  if (server.hasArg("cmd") && server.hasArg("hour") && server.hasArg("minute") ){
    if (server.arg("cmd") == "alarm1Set" && server.arg("hour") != "" && server.arg("cmd") != "")
    {
      String header = "HTTP/1.1 301 OK\r\nCmd: /\r\nCache-Control: no-cache\r\n\r\n";
      servoFeed();

      server.sendContent(header);
      return;
    }
  }
    String reply = "";
    addHeader(true, reply);
    reply += F("<form>");
    reply += F("<table><TH>Feeder configuration<TH><TH>");

    reply += F("<TR><TD>First Feed Time:<TD>");
    reply += F("<input type='text' name='cmd' value='");
  reply += F("'><TR><TD><TD><input class=\"button-link\" type='submit' value='Submit'><TR><TD>");

     reply += F("<TR><TD>Tools<TD><a class=\"button-link\" href=\"?cmd=feed\">Feed Pet</a>");

     reply += F("</table></form>");

   addFooter(reply);



  server.send(200, "text/html", reply);
}

void handleTools(){

   if (!is_authentified()) return;



  String reply = "";
  addHeader(true, reply);

  reply += F("<form>");
  reply += F("<table><TH>Tools<TH>");
          reply += F("<TR><TD>Connected to:<TD>");
        reply += ssid;
        reply += F("<TR><TD>IP:<TD>");
        reply += WiFi.localIP();
# 210 "/Users/alexandru/Documents/Arduino/arduinofeeder/src/WebServer.ino"
  reply += F("<TR><TD>Firmware<TD><a class=\"button-link\" href=\"/firmware\">Update</a>");







  reply += F("<TR><TD>Command<TD>");
  reply += F("<input type='text' name='cmd' value='");

  reply += F("'><TR><TD><TD><input class=\"button-link\" type='submit' value='Submit'><TR><TD>");
    reply += F("<TR><TD>Boot cause:<TD>");


  reply += F("</table></form>");
  addFooter(reply);
  server.send(200, "text/html", reply);

}


void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}



void addHeader(boolean showMenu, String& str)
{
  boolean cssfile = false;

  str += F("<script language=\"javascript\"><!--\n");
  str += F("function dept_onchange(frmselect) {frmselect.submit();}\n");
  str += F("//--></script>");
  str += F("<head><title>");

  str += F("</title>");

  if (!cssfile)
  {
    str += F("<style>");
    str += F("* {font-family:sans-serif; font-size:12pt;}");
    str += F("h1 {font-size:16pt; color:black;}");
    str += F("h6 {font-size:10pt; color:black; text-align:center;}");
    str += F(".button-menu {background-color:#ffffff; color:blue; margin: 10px; text-decoration:none}");
    str += F(".button-link {padding:5px 15px; background-color:#0077dd; color:#fff; border:solid 1px #fff; text-decoration:none}");
    str += F(".button-menu:hover {background:#ddddff;}");
    str += F(".button-link:hover {background:#369;}");
    str += F("th {padding:10px; background-color:black; color:#ffffff;}");
    str += F("td {padding:7px;}");
    str += F("table {color:black;}");
    str += F(".div_l {float: left;}");
    str += F(".div_r {float: right; margin: 2px; padding: 1px 10px; border-radius: 7px; background-color:#080; color:white;}");
    str += F(".div_br {clear: both;}");
    str += F("</style>");
  }
  else
    str += F("<link rel=\"stylesheet\" type=\"text/css\" href=\"esp.css\">");

  str += F("</head>");

  str += F("<h1>Welcome to Smart Feeder: ");


  str += F("</h1>");

  if (showMenu)
  {
    str += F("<BR><a class=\"button-menu\" href=\".\">Main</a>");
    str += F("<a class=\"button-menu\" href=\"config\">Config</a>");
    str += F("<a class=\"button-menu\" href=\"hardware\">Status</a>");


    str += F("<a class=\"button-menu\" href=\"tools\">Tools</a><BR><BR>");
  }
}





void addFooter(String& str)
{
  str += F("<br>v1.0a<br>Press here to <a href=\"/login?DISCONNECT=YES\">logout</a></body></html>");


}


bool is_authentified() {
  Serial.println("Enter is_authentified");
  if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
    if (cookie.indexOf("ESPSESSIONID=1") != -1) {
      Serial.println("Authentification Successful");
      return true;
    }
  }
  Serial.println("Authentification Failed");
  return false;
}
# 1 "/Users/alexandru/Documents/Arduino/arduinofeeder/src/mqtt.ino"
#include <Arduino.h>


long lastReconnectAttempt = 0;


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

WiFiClient espClient;
PubSubClient client(mqtt_server, 1883, callback, espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void mqttSetup(){
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    lastReconnectAttempt = 0;
    if (client.connect("smartfeeder0001", "smartfeeder0001", "abs0lutely")) {
      Serial.println("connected to mqtt broker");
      client.publish("/smartfeeder0001/mio", "SmartFeeder Booted");

    } else {
      Serial.print("MQTT Connection failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

    }
}


boolean reconnect() {

    if (client.connect("smartfeeder0001", "smartfeeder0001", "abs0lutely")) {
      Serial.println("connected");




      return client.connected();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

    }

  }


void mqtt_loop() {
    if (!client.connected()) {
    long now = millis();

      if (now - lastReconnectAttempt > 100) {

      lastReconnectAttempt = now;

      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {



    client.loop();
  }
}

void mqtt_send(char* message){

    if (client.connect("smartfeeder0001", "smartfeeder0001", "abs0lutely")) {
client.publish("/smartfeeder0001/mio", message);}
}
# 1 "/Users/alexandru/Documents/Arduino/arduinofeeder/src/servo.ino"
#include <Arduino.h>

void servoFeed(void) {
  servoMain.attach(12);
      for (int feedtimes = 0; feedtimes <= 5; feedtimes++)
    {
  servoMain.write(180);
  delay(250);
  servoMain.write(45);
  delay(4000);
    }

  Serial.println("Feeder is feeding");
  servoMain.detach();
}
void stepperFeed(int i, int j) {

  while (1) {
    digitalWrite(IN1, 1);
    digitalWrite(IN2, 0);
    digitalWrite(IN3, 0);
    digitalWrite(IN4, 1);
    delay(j);
    i--;
    if (i < 1) break;
    digitalWrite(IN1, 1);
    digitalWrite(IN2, 0);
    digitalWrite(IN3, 0);
    digitalWrite(IN4, 0);
    delay(j);
    i--;
    if (i < 1) break;
    digitalWrite(IN1, 1);
    digitalWrite(IN2, 1);
    digitalWrite(IN3, 0);
    digitalWrite(IN4, 0);
    delay(j);
    i--;
    if (i < 1) break;
    digitalWrite(IN1, 0);
    digitalWrite(IN2, 1);
    digitalWrite(IN3, 0);
    digitalWrite(IN4, 0);
    delay(j);
    i--;
    if (i < 1) break;
    digitalWrite(IN1, 0);
    digitalWrite(IN2, 1);
    digitalWrite(IN3, 1);
    digitalWrite(IN4, 0);
    delay(j);
    i--;
    if (i < 1) break;
    digitalWrite(IN1, 0);
    digitalWrite(IN2, 0);
    digitalWrite(IN3, 1);
    digitalWrite(IN4, 0);
    delay(j);
    i--;
    if (i < 1) break;
    digitalWrite(IN1, 0);
    digitalWrite(IN2, 0);
    digitalWrite(IN3, 1);
    digitalWrite(IN4, 1);
    delay(j);
    i--;
    if (i < 1) break;
    digitalWrite(IN1, 0);
    digitalWrite(IN2, 0);
    digitalWrite(IN3, 0);
    digitalWrite(IN4, 1);
    delay(j);
    i--;
    if (i < 1) break;

  }

}
# 1 "/Users/alexandru/Documents/Arduino/arduinofeeder/src/sync_sntp_rtc.ino"
#include <Arduino.h>



unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");

  memset(packetBuffer, 0, NTP_PACKET_SIZE);


  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;

  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;



  udp.beginPacket(address, 123);
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();


}



void do_sendNTPpacket(){

  WiFi.hostByName(ntpServerName, timeServerIP);


  delay(2000);
  sendNTPpacket(timeServerIP);
  delay(5000);

  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no packet yet");
  }
  else {
    Serial.print("packet received, length=");
    Serial.println(cb);

    udp.read(packetBuffer, NTP_PACKET_SIZE);




    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);


    unsigned long secsSince1900 = highWord << 16 | lowWord;






    const unsigned long seventyYears = 2208988800UL;

    unsigned long epoch = secsSince1900 - seventyYears;

    int timezone = +3;
    epoch += 3600*timezone;
      Serial.println(epoch);



      setTime(epoch);


    Serial.print("The Romania time is ");
    Serial.print((epoch % 86400L) / 3600);
    Serial.print(':');
    if ( ((epoch % 3600) / 60) < 10 ) {

      Serial.print('0');
    }
    Serial.print((epoch % 3600) / 60);
    Serial.print(':');
    if ( (epoch % 60) < 10 ) {

      Serial.print('0');
    }
    Serial.println(epoch % 60);
    delay(10000);
  }}