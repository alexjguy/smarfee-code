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
