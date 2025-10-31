int sensorValue; (int variable to read analogue output reading)
int digitalValue; (int variable to read digital output reading)

void setup()
{
  Serial.begin(9600); // sets the serial port to 9600 (sets the serial communication to 9600 baud rate)
  pinMode(13, OUTPUT);
  pinMode(2, INPUT);
}

void loop()
{
  sensorValue = analogRead(A0);
  if (sensorValue > 400)
  {
    digitalWrite(13, HIGH); (if the analogue reading is greater than 400, then the LED turns ON)
  }
  else
  {
    digitalWrite(13, LOW); (if the analogue reading is less than 400, the LED turns OFF)
    Serial.println(sensorValue, DEC); // prints the value read
  }
  Serial.println(digitalValue, DEC);
  delay(1000); // wait 100ms for the next reading   (analogue and digital output readings are displayed on the monitor)
}
