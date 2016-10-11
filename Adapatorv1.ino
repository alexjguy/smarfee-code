//#include <WiFiMDNSResponder.h>
#include <WiFiServer.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);
#include <DS1307RTC.h>
#include <ESP8266mDNS.h> //for_update
#include <Servo.h>  //for_servo
Servo servoMain;
#include <Wire.h> // I2C
#include <WiFiUdp.h>
#include <SPI.h>
#include <PubSubClient.h>
///
#include <pgmspace.h>
#include <RtcDS3231.h>
RtcDS3231 Rtc;

#define RtcSquareWavePin 13 // Mega2560

#define RtcSquareWaveInterrupt 13 // Mega2560

// marked volatile so interrupt can safely modify them and
// other code can safely read and modify them
volatile uint16_t interuptCount = 0;
volatile bool interuptFlag = false;

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
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  httpServerSetup();
  Wire.begin();


  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
  do_sendNTPpacket();


  rtcAlarmSetup();
  I2C_Scanner();
  mqttSetup(); 
}

void loop(void) {
  
  // wait ten seconds before asking for the time again
  digitalClockDisplay();
  //  delay(10000);

  if (!Rtc.IsDateTimeValid())
  {
    Serial.println("RTC lost confidence in the DateTime!");
    do_sendNTPpacket();
  }

  RtcDateTime now = Rtc.GetDateTime();
  RtcTemperature temp = Rtc.GetTemperature();
  Serial.print(temp.AsFloat());
  Serial.println("C");

  printDateTime(now);
  Serial.println();

  // we only want to show time every 10 seconds
  // but we want to show responce to the interupt firing
  for (int timeCount = 0; timeCount < 20; timeCount++)
  {
    if (Alarmed())
    {
      Serial.print(">>Interupt Count: ");
      Serial.print(interuptCount);
      Serial.println("<<");
    }

    server.handleClient();

    delay(500);
    mqtt_loop();
  }
  

}

