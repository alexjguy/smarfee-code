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
