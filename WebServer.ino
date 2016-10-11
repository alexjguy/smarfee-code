void httpServerSetup(){

  server.on("/", handleRoot);
  server.on("/login", handleLogin);
  server.on("/tools", handleTools);
  server.on("/config", handleConfig);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works without need of authentification");
  });

  //for_update
  server.on("/firmware", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/html", serverIndex);
  });
  //
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
      if (!Update.begin(maxSketchSpace)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
      Serial.setDebugOutput(false);
    }
    yield();
  });
///
  server.onNotFound(handleNotFound);
  //here the list of headers to be recorded
  const char * headerkeys[] = {"User-Agent", "Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
  //ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize );
  
  server.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.println("HTTP server started");
}

//login page, also called for disconnect
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
    if (server.arg("USERNAME") == "admin" &&  server.arg("PASSWORD") == "start123" ) {
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
//  String content = "<html>";
//  content += "<head>";
//  content += "</head>";
//  content += "<body";
  content += "<form action='/login' method='POST'>Please login to access Smart Feeder:<br>";
  content += "User:<input type='text' name='USERNAME' placeholder='user name'><br>";
  content += "Password:<input type='password' name='PASSWORD' placeholder='password'><br>";
  content += "<input type='submit' name='SUBMIT' value='Submit'></form>" + msg + "<br>";
  content += "</body></html>";
  server.send(200, "text/html", content);
}


//root page can be accessed only if authentification is ok
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
      servoFeed();
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

 // String webrequest = WebServer.arg("cmd");

  String reply = "";
  addHeader(true, reply);
 
  reply += F("<form>");
  reply += F("<table><TH>Tools<TH>");
          reply += F("<TR><TD>Connected to:<TD>");
        reply += ssid;
        reply += F("<TR><TD>IP:<TD>");
        reply += WiFi.localIP();

  //reply += F("<TR><TD>Tools<TD><a class=\"button-link\" href=\"/servo\">Feed Pet</a>");
  //reply += F("<a class=\"button-link\" href=\"log\">Log</a>");
  //reply += F("<a class=\"button-link\" href=\"advanced\">Advanced</a><BR><BR>");
  //reply += F("<TR><TD>Wifi<TD><a class=\"button-link\" href=\"/?cmd=wificonnect\">Connect</a>");
  //reply += F("<a class=\"button-link\" href=\"/?cmd=wifidisconnect\">Disconnect</a>");
  //reply += F("<a class=\"button-link\" href=\"/wifiscanner\">Scan</a><BR><BR>");
  //reply += F("<TR><TD>Interfaces<TD><a class=\"button-link\" href=\"/i2cscanner\">I2C Scan</a><BR><BR>");
  reply += F("<TR><TD>Firmware<TD><a class=\"button-link\" href=\"/firmware\">Update</a>");
  //reply += F("<a class=\"button-link\" href=\"/download\">Save</a>");
        reply += F("<TR><TD>Temperature:<TD>");
      RtcTemperature temp = Rtc.GetTemperature();
      reply += temp.AsFloat();
      reply += F("C");


  reply += F("<TR><TD>Command<TD>");
  reply += F("<input type='text' name='cmd' value='");
 // reply += webrequest;
  reply += F("'><TR><TD><TD><input class=\"button-link\" type='submit' value='Submit'><TR><TD>");
    reply += F("<TR><TD>Boot cause:<TD>");
  

  reply += F("</table></form>");
  addFooter(reply);
  server.send(200, "text/html", reply);

}

//no need authentification
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

// Add top menu
//********************************************************************************
void addHeader(boolean showMenu, String& str)
{
  boolean cssfile = false;

  str += F("<script language=\"javascript\"><!--\n");
  str += F("function dept_onchange(frmselect) {frmselect.submit();}\n");
  str += F("//--></script>");
  str += F("<head><title>");
 // str += Settings.Name;
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
 // str += Settings.Name;

  str += F("</h1>");

  if (showMenu)
  {
    str += F("<BR><a class=\"button-menu\" href=\".\">Main</a>");
    str += F("<a class=\"button-menu\" href=\"config\">Config</a>");
    str += F("<a class=\"button-menu\" href=\"hardware\">Status</a>");
//    if (Settings.UseRules)
//      str += F("<a class=\"button-menu\" href=\"rules\">Rules</a>");
    str += F("<a class=\"button-menu\" href=\"tools\">Tools</a><BR><BR>");
  }
}


//********************************************************************************
// Add footer to web page
//********************************************************************************
void addFooter(String& str)
{
  str += F("<br>v0.7b<br>Press here to <a href=\"/login?DISCONNECT=YES\">logout</a></body></html>");
  
  //<a href=\"www.altech-inspired.ro\">Powered by Altech Inspired</a>
}

//Check if header is present and correct
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



