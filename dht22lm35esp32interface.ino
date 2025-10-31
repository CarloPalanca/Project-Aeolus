#include <MQ135.h>
#include <DHT.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Fonts/Org_01.h>
#define ADC_VREF_mV           3300.0
#define ADC_RESOLUTION        4096.0
#define PIN_LM35              33
#define PIN_MQ135             32
#define PIN_DHT22             26
#define i2c_Address           0x3c
#define SCREEN_WIDTH          128
#define SCREEN_HEIGHT         64
#define OLED_RESET            -1
#define mq135calibrationvalue 40 //100, 627.54
#define tempL                 28
#define tempH                 29
#define humidityL             50
#define humidityH             60
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT dht(PIN_DHT22, DHT22);  MQ135 mq135_sensor(PIN_MQ135, mq135calibrationvalue); 
#include <FluxGarage_RoboEyes.h>
roboEyes roboEyes; int startupTime = 0; int count = 0; int relayFan = 18; int relayHum = 19;

void setup()
{
  Serial.begin(115200); dht.begin(); Wire.begin();
  delay(1000);
  display.begin(i2c_Address, true); roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);
  roboEyes.setHFlicker(ON, 3);
  delay(500);
  roboEyes.setHFlicker(OFF, 3); roboEyes.anim_laugh();
  delay(1000);
  roboEyes.setAutoblinker(ON, 3, 2); roboEyes.setIdleMode(ON, 2, 2);
  //roboEyes.setMood(DEFAULT); //TIRED, ANGRY, HAPPY, DEFAULT
  //roboEyes.setPosition(DEFAULT); //N, NE, E, SE, S, SW, W, NW, DEFAULT
  roboEyes.setCuriosity(ON);
  for (startupTime = 0; startupTime < 5000; startupTime++)
  {
    roboEyes.update();
    delay(1);
  }
  pinMode(relayFan, OUTPUT); pinMode(relayHum, OUTPUT); pinMode(5, OUTPUT);
  digitalWrite(relayFan, HIGH); digitalWrite(relayHum, HIGH);
}

void loop()
{
  float humidity = dht.readHumidity();
  float tempDHT22 = dht.readTemperature();
  int adcVal = analogRead(PIN_LM35);
  float milliVolt = adcVal * (ADC_VREF_mV / ADC_RESOLUTION);
  float tempLM35 = (milliVolt / 10) + 10; // adjusted temperature offset of 10C
  float avetemp = (tempDHT22 + tempLM35) / 2;
  float rzero = mq135_sensor.getRZero();
  float correctedRZero = mq135_sensor.getCorrectedRZero(tempDHT22, humidity);
  float resistance = mq135_sensor.getResistance();
  float ppm = mq135_sensor.getPPM();
  float correctedPPM = mq135_sensor.getCorrectedPPM(tempDHT22, humidity);
  float offsetTemp = tempDHT22 - tempLM35;
  Serial.print("Temperature of LM35: ");
  Serial.print(tempLM35); 
  Serial.println("°C");
  Serial.print("Temperature of DHT22: ");
  Serial.print(tempDHT22);
  Serial.println("°C");
  Serial.print("Average Temperature: ");
  Serial.print(avetemp);
  Serial.println("°C");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
  Serial.print("MQ135 RZero: ");
  Serial.println(rzero);
  Serial.print("Corrected RZero: ");
  Serial.println(correctedRZero);
  Serial.print("Resistance: ");
  Serial.print(resistance);
  Serial.println(" ohms");
  Serial.print("Air Concentration: ");
  Serial.print(ppm);
  Serial.println("PPM");
  Serial.print("Corrected Air Concentration: ");
  Serial.print(correctedPPM);
  Serial.println("PPM");
  Serial.print("Air Quality: ");
  display.clearDisplay();
  dht.read();
  display.setFont(&Org_01);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 5);
  display.print("LM35");
  display.setCursor(70, 5);
  display.print("DHT22");
  display.setCursor(0, 15);
  display.print("T:");
  display.setCursor(15, 15);
  display.print(tempLM35, 1);
  display.setCursor(0, 25);
  display.print("O:");
  display.setCursor(15, 25);
  display.print(offsetTemp, 1);
  display.println("*C");
  display.setCursor(70, 15);
  display.print("T:");
  display.setCursor(85, 15);
  display.print(tempDHT22, 1);
  display.println("*C");
  display.setCursor(0, 35);
  display.print("AVE TEMP:");
  display.setCursor(55, 35);
  display.print(avetemp, 1);
  display.println("*C");
  display.setCursor(70, 25);
  display.print("H:");
  display.setCursor(85, 25);
  display.print(humidity, 1);
  display.println("%");
  display.setCursor(0, 45);
  display.print("Co:");
  display.setCursor(20, 45);
  display.print(correctedPPM, 1);
  display.println(" PPM");
  display.setCursor(0, 55);
  display.print("AIR QUALITY:");
  if ((correctedPPM >= 0) && (correctedPPM < 400))
  {
    Serial.println("Good");
    display.setCursor(70, 55);
    display.println("GOOD");
    if ((avetemp > tempH) && (humidity > humidityH))
    {
      digitalWrite(relayFan, LOW);
      Serial.println("FAN: ON");
      digitalWrite(relayHum, HIGH);
      Serial.println("HUMIDIFIER: OFF");
    }
    else if ((avetemp < tempL) && (humidity > humidityH))
    {
      digitalWrite(relayFan, HIGH);
      Serial.println("FAN: OFF");
      digitalWrite(relayHum, HIGH);
      Serial.println("HUMIDIFIER: OFF");
    }
    else if ((avetemp > tempH) && (humidity < humidityL))
    {
      digitalWrite(relayFan, LOW);
      Serial.println("FAN: ON");
      digitalWrite(relayHum, LOW);
      Serial.println("HUMIDIFIER: ON");
    }
    else if ((avetemp < tempL) && (humidity < humidityL))
    {
      digitalWrite(relayFan, HIGH);
      Serial.println("FAN: OFF");
      digitalWrite(relayHum, LOW);
      Serial.println("HUMIDIFIER: ON");
    }
  }
  else if ((correctedPPM >= 400) && (correctedPPM < 1000))
  {
    Serial.println("Normal");
    display.setCursor(70, 55);
    display.println("NORMAL");
    if ((avetemp > tempH) && (humidity > humidityH))
    {
      digitalWrite(relayFan, LOW);
      Serial.println("FAN: ON");
      digitalWrite(relayHum, HIGH);
      Serial.println("HUMIDIFIER: OFF");
    }
    else if ((avetemp < tempL) && (humidity > humidityH))
    {
      digitalWrite(relayFan, HIGH);
      Serial.println("FAN: OFF");
      digitalWrite(relayHum, HIGH);
      Serial.println("HUMIDIFIER: OFF");
    }
    else if ((avetemp > tempH) && (humidity < humidityL))
    {
      digitalWrite(relayFan, LOW);
      Serial.println("FAN: ON");
      digitalWrite(relayHum, LOW);
      Serial.println("HUMIDIFIER: ON");
    }
    else if ((avetemp < tempL) && (humidity < humidityL))
    {
      digitalWrite(relayFan, HIGH);
      Serial.println("FAN: OFF");
      digitalWrite(relayHum, LOW);
      Serial.println("HUMIDIFIER: ON");
    }
  }
  else if ((correctedPPM >= 1000) && (correctedPPM < 2000))
  {
    Serial.println("Poor");
    display.setCursor(70, 55);
    display.println("POOR");
    if ((avetemp > tempH) && (humidity > humidityH))
    {
      digitalWrite(relayFan, LOW);
      Serial.println("FAN: ON");
      digitalWrite(relayHum, HIGH);
      Serial.println("HUMIDIFIER: OFF");
    }
    else if ((avetemp < tempL) && (humidity > humidityH))
    {
      digitalWrite(relayFan, HIGH);
      Serial.println("FAN: OFF");
      digitalWrite(relayHum, HIGH);
      Serial.println("HUMIDIFIER: OFF");
    }
    else if ((avetemp > tempH) && (humidity < humidityL))
    {
      digitalWrite(relayFan, LOW);
      Serial.println("FAN: ON");
      digitalWrite(relayHum, LOW);
      Serial.println("HUMIDIFIER: ON");
    }
    else if ((avetemp < tempL) && (humidity < humidityL))
    {
      digitalWrite(relayFan, HIGH);
      Serial.println("FAN: OFF");
      digitalWrite(relayHum, LOW);
      Serial.println("HUMIDIFIER: ON");
    }
    tone(5, 1000);
    delay(250);
    tone(5, 0);
    delay(5000);
  }
  else if ((correctedPPM >= 2000) && (correctedPPM < 5000))
  {
    Serial.println("Unhealthy");
    display.setCursor(70, 55);
    display.println("UNHEALTHY");
    digitalWrite(relayFan, HIGH);
    Serial.println("FAN: OFF");
    digitalWrite(relayHum, HIGH);
    Serial.println("HUMIDIFIER: OFF");
    tone(5, 1000);
    delay(500);
    tone(5, 0);
    delay(500);
  }
  else if (correctedPPM >= 5000)
  {
    Serial.println("HAZARDOUS");
    display.setCursor(70, 55);
    display.println("HAZARDOUS");
    digitalWrite(relayFan, HIGH);
    Serial.println("FAN: OFF");
    digitalWrite(relayHum, HIGH);
    Serial.println("HUMIDIFIER: OFF");
    tone(5, 1000);
  }
  Serial.println(" "); delay(1);
  display.display(); delay(2000);
}
