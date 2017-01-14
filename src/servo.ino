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
void forward(int i, int j) {
  // Set both motors ON
  while (1)   {
    digitalWrite(IN1, 0);
    digitalWrite(IN2, 0);
    digitalWrite(IN3, 0);
    digitalWrite(IN4, 1);
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
    digitalWrite(IN3, 1);
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
    digitalWrite(IN2, 1);
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
    digitalWrite(IN1, 1);
    digitalWrite(IN2, 0);
    digitalWrite(IN3, 0);
    digitalWrite(IN4, 0);
    delay(j);
    i--;
    if (i < 1) break;
    digitalWrite(IN1, 1);
    digitalWrite(IN2, 0);
    digitalWrite(IN3, 0);
    digitalWrite(IN4, 1);
    delay(j);
    i--;
    if (i < 1) break;
  }

}  // end forward()
