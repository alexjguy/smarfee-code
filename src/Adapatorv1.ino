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
///
#include <pgmspace.h>
#include <TimeAlarms.h>

#include <Stepper.h>
const int stepsPerRevolution = 32;  // change this to fit the number of steps per revolution
// for your motor
const int IN1 = 16;
const int IN2 = 5;
const int IN3 = 4;
const int IN4 = 0;
Stepper myStepper(stepsPerRevolution, IN1, IN2, IN3, IN4);
///////////////

/////////////
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
///////////////
////////////

const char* ssid = "athome_24";
const char* password = "athome#$";

const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";


void setup(void) {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Trying to connect to WiFi");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  httpServerSetup();
  //Wire.begin();


  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
  do_sendNTPpacket();

  mqttSetup();

  Alarm.alarmRepeat(7,00,0, MorningAlarm);  // 8:30am every day
  Alarm.alarmRepeat(17,30,0,EveningAlarm);  // 5:45pm every day
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);

  myStepper.setSpeed(15);

}

void loop(void) {


  Serial.println();

  // we only want to show time every 10 seconds
  // but we ndswant to show responce to the interupt firing
  for (int timeCount = 0; timeCount < 20; timeCount++)
  {


    server.handleClient();

    delay(500);

  }
  mqtt_loop();
  forward(4096,1);
  //myStepper.step(stepsPerRevolution);
delay(500);

}
