#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <Arduino.h>
//#include <WiFiMDNSResponder.h>
#include <WiFiServer.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);
#include <ESP8266mDNS.h> //for_update
#include <Servo.h>  //for_servo
Servo servoMain;
//#include <Wire.h> // I2C
#include <WiFiUdp.h>
#include <SPI.h>
#include <PubSubClient.h>

#include <pgmspace.h>
#include <TimeAlarms.h>
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <CMMC_OTA.h>
CMMC_OTA ota;


// defined for NTP
unsigned int localPort = 2390;      // local port to listen for UDP packets

/* Don't hardwire the IP address or we won't get the benefits of the pool.
    Lookup the IP address for the host name instead */
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "ro.pool.ntp.org";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;
// defined for NTP

const char* ssid = "athome_24";
const char* password = "athome#$";

const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

char mqtt_server[40] = "192.168.0.44";
char mqtt_port[6] = "8080";


//flag for saving data
bool shouldSaveConfig = true;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

extern "C" {
#include "user_interface.h"
}



// defined for button1
const int button1 = 12;
boolean buttonActive = false;
boolean longPressActive = false;
boolean button1Active = false;
long buttonTimer = 0;
long buttonTime = 3000;
// defined for button1

// alt button
int buttonState = 0;     // current state of the button
int lastButtonState = 0; // previous state of the button
int startPressed = 0;    // the time button was pressed
int endPressed = 0;      // the time button was released
int timeHold = 0;        // the time button is hold
int timeReleased = 0;    // the time button is released
// alt button


// defined for status LED
//const int ledPin = 14;       // the pin that the LED is attached to
//

#define ledPin 14


// for your motor
//const int IN1 = 16;
//const int IN2 = 5;
//const int IN3 = 4;
//const int IN4 = 0;
#define IN1 16
#define IN2 5
#define IN3 4
#define IN4 0


void setup(void) {
  //system_update_cpu_freq(80);
  system_update_cpu_freq(160);

  pinMode(ledPin, OUTPUT);
  ledState("blinkOnce");

  Serial.begin(115200);
//  WiFi.begin(ssid, password);
  Serial.println("");


    //clean FS, for testing
    //SPIFFS.format();

    //read configuration from FS json
    Serial.println("mounting FS...");

    if (SPIFFS.begin()) {
      Serial.println("mounted file system");
      if (SPIFFS.exists("/config.json")) {
        //file exists, reading and loading
        Serial.println("reading config file");
        File configFile = SPIFFS.open("/config.json", "r");
        if (configFile) {
          Serial.println("opened config file");
          size_t size = configFile.size();
          // Allocate a buffer to store contents of the file.
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
    //end read



    // The extra parameters to be configured (can be either global or just in the setup)
    // After connecting, parameter.getValue() will get you the configured value
    // id/name placeholder/prompt default length
    WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
    WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);

    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;

    //set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    //set static ip
    //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    //add all your parameters here
    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_port);

    //reset settings - for testing
    //wifiManager.resetSettings();

    //set minimu quality of signal so it ignores AP's under that quality
    //defaults to 8%
    //wifiManager.setMinimumSignalQuality();

    //sets timeout until configuration portal gets turned off
    //useful to make it all retry or go to sleep
    //in seconds
    wifiManager.setTimeout(300);

    //fetches ssid and pass and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    if (!wifiManager.autoConnect("SmartFeeder","1234567890")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }

      //if you get here you have connected to the WiFi
      Serial.println("connected...yeey :)");

      //read updated parameters
      strcpy(mqtt_server, custom_mqtt_server.getValue());
      strcpy(mqtt_port, custom_mqtt_port.getValue());

      //save the custom parameters to FS
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
        //end save
      }

      Serial.println("local ip");
      Serial.println(WiFi.localIP());


  httpServerSetup();
  //Wire.begin();


  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
  //do_sendNTPpacket();

  //mqttSetup();

  Alarm.alarmRepeat(7,00,0, MorningAlarm);  // 8:30am every day
  Alarm.alarmRepeat(17,30,0,EveningAlarm);  // 5:45pm every day
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);

  pinMode(button1, INPUT);
  ledState("blinkTwice");
  ledState("On");

  ota.init();


}

void loop(void) {


  // this keeps the small loop running at 100ms and returns
  // to the large loop after 10 seconds
  for (int timeCount = 0; timeCount < 100; timeCount++){
    server.handleClient();
    delay(100);
    buttonDo();
  }
  ESP.getCpuFreqMHz();
  //stepperFeed(4096,1);
  //mqtt_loop();
//delay(500);
ota.loop();

}


void buttonDo() {
// read the pushbutton input pin:
 buttonState = digitalRead(button1);

 // button state changed
 if (buttonState != lastButtonState) {

     // the button was just pressed
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

     // the button was just released
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
             //wifiManager.resetSettings();
         }

     }

 }

 // save the current state as the last state,
 //for next time through the loop
 lastButtonState = buttonState;
}
