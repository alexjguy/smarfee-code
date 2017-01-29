

// Update these with values suitable for your network.
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
      //mqtt_send("SmartFeeder Booted mqtt_send");
    }    else {
      Serial.print("MQTT Connection failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
    }
}


boolean reconnect() {
  // Loop until we're reconnected
    if (client.connect("smartfeeder0001", "smartfeeder0001", "abs0lutely")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      //client.subscribe("inTopic");
      return client.connected();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
    }

  }


void mqtt_loop() {
    if (!client.connected()) {
    long now = millis();
//    if (now - lastReconnectAttempt > 5000) {
      if (now - lastReconnectAttempt > 100) {

      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected
    //Serial.println("Connected to MQTT");

    client.loop();
  }
}

void mqtt_send(char* message){

    if (client.connect("smartfeeder0001", "smartfeeder0001", "abs0lutely")) {
client.publish("/smartfeeder0001/mio", message);}
}
