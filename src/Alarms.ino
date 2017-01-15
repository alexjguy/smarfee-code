#include <Arduino.h>


// functions to be called when an alarm triggers:
void MorningAlarm(){
  Serial.println("Alarm: - Mio morning Feed");
  //servoFeed();
  stepperFeed(2048,1);
}

void EveningAlarm(){
  Serial.println("Alarm: - Mio evening Feed");
  //servoFeed();
  stepperFeed(2048,1);
}

//void WeeklyAlarm(){
//  Serial.println("Alarm: - its Monday Morning");
//}
//
//void ExplicitAlarm(){
//  Serial.println("Alarm: - this triggers only at the given date and time");
//}
//
//void Repeats(){
//  Serial.println("15 second timer");
//}
//
//void OnceOnly(){
//  Serial.println("This timer only triggers once");
//}

void digitalClockDisplay()
{
  // digital clock display of the time
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
