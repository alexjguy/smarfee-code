void InteruptServiceRoutine()
{
    // since this interupted any other running code,
    // don't do anything that takes long and especially avoid
    // any communications calls within this routine
    interuptCount++;
    interuptFlag = true;
}

void rtcAlarmSetup(void){

     // set the interupt pin to input mode
    pinMode(RtcSquareWavePin, INPUT);

    //--------RTC SETUP ------------
    Rtc.Begin();
    // if you are using ESP-01 then uncomment the line below to reset the pins to
    // the available pins for SDA, SCL
    // Wire.begin(0, 2); // due to limited pins, use pin 0 and 2 for SDA, SCL

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

    if (!Rtc.IsDateTimeValid()) 
    {
        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(compiled);
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    
    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeAlarmBoth); 

    // Alarm 1 set to trigger every day when 
    // the hours, minutes, and seconds match
    //RtcDateTime alarmTime = now + 88; // into the future
    DS3231AlarmOne alarm1(
            0,
            7,
            1,
            0, 
            DS3231AlarmOneControl_HoursMinutesSecondsMatch);
    Rtc.SetAlarmOne(alarm1);

    // Alarm 2 set to trigger at the top of the minute
    DS3231AlarmTwo alarm2(
            0,
            18,
            1, 
            DS3231AlarmTwoControl_HoursMinutesMatch);
    Rtc.SetAlarmTwo(alarm2);

    // throw away any old alarm state before we ran
    Rtc.LatchAlarmsTriggeredFlags();

    // setup external interupt 
    attachInterrupt(RtcSquareWaveInterrupt, InteruptServiceRoutine, FALLING);

}



bool Alarmed()
{
    bool wasAlarmed = false;
    if (interuptFlag)  // check our flag that gets sets in the interupt
    {
        wasAlarmed = true;
        interuptFlag = false; // reset the flag
        
        // this gives us which alarms triggered and
        // then allows for others to trigger again
        DS3231AlarmFlag flag = Rtc.LatchAlarmsTriggeredFlags();

        if (flag & DS3231AlarmFlag_Alarm1)
        {
            Serial.println("alarm one triggered");
            servoFeed();
            mqtt_send("SmartFeeder fed Mio at ");

        }
        if (flag & DS3231AlarmFlag_Alarm2)
        {
            Serial.println("alarm two triggered");
            servoFeed();
            mqtt_send("SmartFeeder fed Mio at ");
        }
    }
    return wasAlarmed;
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
  char datestring[20];

  snprintf_P(datestring, 
      countof(datestring),
      PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
      dt.Month(),
      dt.Day(),
      dt.Year(),
      dt.Hour(),
      dt.Minute(),
      dt.Second() );
    Serial.print(datestring);
}


void rtcAlarmLoop(){

}



