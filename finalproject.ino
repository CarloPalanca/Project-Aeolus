#include "thingProperties.h"
#include "MQ135.h"
#include "DHT.h"
#include "SPI.h"
#include "Wire.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SH110X.h"
#include "Fonts/Org_01.h"

#define ADC_VREF_mV    3300.0
#define ADC_RESOLUTION 4096.0
#define PIN_LM35       33
#define PIN_MQ135      32
#define PIN_DHT22      26
#define i2c_Address 0x3c
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT dht(PIN_DHT22, DHT22);
MQ135 mq135_sensor(PIN_MQ135, 230); //100, 627.54

#include "FluxGarage_RoboEyes.h"
roboEyes roboEyes;
int startupTime = 0;

int count = 0;

int relayFan = 18;
int relayHum = 19;

void setup() {
  Serial.begin(115200);
  dht.begin();
  Wire.begin();
  //Blynk.begin (auth, ssid, pass);
  //timer.setInterval(100L, sendSensor);
  delay(1000);
  display.begin(i2c_Address, true);
  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);
  roboEyes.setHFlicker(ON, 3);
  delay(500);
  roboEyes.setHFlicker(OFF, 3);
  roboEyes.anim_laugh();
  delay(1000);
  roboEyes.setAutoblinker(ON, 3, 2);
  roboEyes.setIdleMode(ON, 2, 2);
  //roboEyes.setMood(DEFAULT); //TIRED, ANGRY, HAPPY, DEFAULT
  //roboEyes.setPosition(DEFAULT); //N, NE, E, SE, S, SW, W, NW, DEFAULT
  roboEyes.setCuriosity(ON);
  for (startupTime = 0; startupTime < 5000; startupTime++) // basta ang product is 5k
  {
    roboEyes.update();
    /*
      display.clearDisplay();
      display.setFont(&Org_01);
      display.setTextSize(1);
      display.setTextColor(SH110X_WHITE);
      display.setCursor(0, 55);
      display.print("Warming.....");
      display.print(startupTime, 1);
      display.print("%");
    */
    delay(1); //1 or 50
  }
  pinMode(relayFan, OUTPUT);
  pinMode(relayHum, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(relayFan, HIGH);
  digitalWrite(relayHum, HIGH);

  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(1500);

  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  /*
     The following function allows you to obtain more information
     related to the state of network and IoT Cloud connection and errors
     the higher number the more granular information you’ll get.
     The default is 0 (only errors).
     Maximum is 4
  */
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

void loop() {
  ArduinoCloud.update();
  // Your code here
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
  bool logicFan;
  bool logicHumidifier;
  air_concentration = correctedPPM;
  relative_humidity = humidity;
  temperature_average = avetemp;
  relay_fan = logicFan;
  relay_humidifier = logicHumidifier;

  Serial.print("Temperature of LM35: ");
  Serial.print(tempLM35);   // print the temperature in °C
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
    if ((avetemp > 30.5) && (humidity > 60))
    {
      digitalWrite(relayFan, LOW);
      Serial.println("FAN: ON");
      digitalWrite(relayHum, HIGH);
      Serial.println("HUMIDIFIER: OFF");
      logicFan = true;
      logicHumidifier = false;
    }
    else if ((avetemp < 29.5) && (humidity > 60))
    {
      digitalWrite(relayFan, HIGH);
      Serial.println("FAN: OFF");
      digitalWrite(relayHum, HIGH);
      Serial.println("HUMIDIFIER: OFF");
      logicFan = false;
      logicHumidifier = false;
    }
    else if ((avetemp > 30.5) && (humidity < 50))
    {
      digitalWrite(relayFan, LOW);
      Serial.println("FAN: ON");
      digitalWrite(relayHum, LOW);
      Serial.println("HUMIDIFIER: ON");
      logicFan = true;
      logicHumidifier = true;
    }
    else if ((avetemp < 29.5) && (humidity < 50))
    {
      digitalWrite(relayFan, HIGH);
      Serial.println("FAN: OFF");
      digitalWrite(relayHum, LOW);
      Serial.println("HUMIDIFIER: ON");
      logicFan = false;
      logicHumidifier = true;
    }
  }
  else if ((correctedPPM >= 400) && (correctedPPM < 1000))
  {
    Serial.println("Normal");
    display.setCursor(70, 55);
    display.println("NORMAL");
    if ((avetemp > 30.5) && (humidity > 60))
    {
      digitalWrite(relayFan, LOW);
      Serial.println("FAN: ON");
      digitalWrite(relayHum, HIGH);
      Serial.println("HUMIDIFIER: OFF");
      logicFan = true;
      logicHumidifier = false;
    }
    else if ((avetemp < 29.5) && (humidity > 60))
    {
      digitalWrite(relayFan, HIGH);
      Serial.println("FAN: OFF");
      digitalWrite(relayHum, HIGH);
      Serial.println("HUMIDIFIER: OFF");
      logicFan = false;
      logicHumidifier = false;
    }
    else if ((avetemp > 30.5) && (humidity < 50))
    {
      digitalWrite(relayFan, LOW);
      Serial.println("FAN: ON");
      digitalWrite(relayHum, LOW);
      Serial.println("HUMIDIFIER: ON");
      logicFan = true;
      logicHumidifier = true;
    }
    else if ((avetemp < 29.5) && (humidity < 50))
    {
      digitalWrite(relayFan, HIGH);
      Serial.println("FAN: OFF");
      digitalWrite(relayHum, LOW);
      Serial.println("HUMIDIFIER: ON");
      logicFan = false;
      logicHumidifier = true;
    }
  }
  else if ((correctedPPM >= 1000) && (correctedPPM < 2000))
  {
    Serial.println("Poor");
    display.setCursor(70, 55);
    display.println("POOR");
    if ((avetemp > 30.5) && (humidity > 60))
    {
      digitalWrite(relayFan, LOW);
      Serial.println("FAN: ON");
      digitalWrite(relayHum, HIGH);
      Serial.println("HUMIDIFIER: OFF");
      logicFan = true;
      logicHumidifier = false;
    }
    else if ((avetemp < 29.5) && (humidity > 60))
    {
      digitalWrite(relayFan, HIGH);
      Serial.println("FAN: OFF");
      digitalWrite(relayHum, HIGH);
      Serial.println("HUMIDIFIER: OFF");
      logicFan = false;
      logicHumidifier = false;
    }
    else if ((avetemp > 30.5) && (humidity < 50))
    {
      digitalWrite(relayFan, LOW);
      Serial.println("FAN: ON");
      digitalWrite(relayHum, LOW);
      Serial.println("HUMIDIFIER: ON");
      logicFan = true;
      logicHumidifier = true;
    }
    else if ((avetemp < 29.5) && (humidity < 50))
    {
      digitalWrite(relayFan, HIGH);
      Serial.println("FAN: OFF");
      digitalWrite(relayHum, LOW);
      Serial.println("HUMIDIFIER: ON");
      logicFan = false;
      logicHumidifier = true;
    }
    tone(4, 1000);
    delay(250);
    tone(4, 0);
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
    logicFan = false;
    logicHumidifier = false;
    tone(4, 1000);
    delay(500);
    tone(4, 0);
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
    logicFan = false;
    logicHumidifier = false;
    tone(4, 1000);
  }

  Serial.println(" ");
  delay(1);
  display.display();
  //Blynk.run();
  delay(2000); //vary to your liking

}

/*
  Since TemperatureDHT22 is READ_WRITE variable, onTemperatureDHT22Change() is
  executed every time a new value is received from IoT Cloud.
*/
void onTemperatureDHT22Change()  {
  // Add your code here to act upon TemperatureDHT22 change
}


/*
  Since Humidity is READ_WRITE variable, onHumidityChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onHumidityChange()  {
  // Add your code here to act upon Humidity change
}

/*
  Since TemperatureLM35 is READ_WRITE variable, onTemperatureLM35Change() is
  executed every time a new value is received from IoT Cloud.
*/
void onTemperatureLM35Change()  {
  // Add your code here to act upon TemperatureLM35 change
}

/*
  Since AirConcentration is READ_WRITE variable, onAirConcentrationChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onAirConcentrationChange()  {
  // Add your code here to act upon AirConcentration change
}

/*
  Since Message is READ_WRITE variable, onMessageChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onMessageChange()  {
  // Add your code here to act upon Message change
}

/*
  Since TempDHT22 is READ_WRITE variable, onTempDHT22Change() is
  executed every time a new value is received from IoT Cloud.
*/
void onTempDHT22Change()  {
  // Add your code here to act upon TempDHT22 change
}
/*
  Since CorrectedPPM is READ_WRITE variable, onCorrectedPPMChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onCorrectedPPMChange()  {
  // Add your code here to act upon CorrectedPPM change
}
/*
  Since TempLM35 is READ_WRITE variable, onTempLM35Change() is
  executed every time a new value is received from IoT Cloud.
*/
void onTempLM35Change()  {
  // Add your code here to act upon TempLM35 change
}
/*
  Since RelativeHumidity is READ_WRITE variable, onRelativeHumidityChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onRelativeHumidityChange()  {
  // Add your code here to act upon RelativeHumidity change
}
/*
  Since RelayFan is READ_WRITE variable, onRelayFanChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onRelayFanChange()  {
  // Add your code here to act upon RelayFan change
}
/*
  Since RelayHumidifier is READ_WRITE variable, onRelayHumidifierChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onRelayHumidifierChange()  {
  // Add your code here to act upon RelayHumidifier change
}
/*
  Since TemperatureAverage is READ_WRITE variable, onTemperatureAverageChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onTemperatureAverageChange()  {
  // Add your code here to act upon TemperatureAverage change
}
