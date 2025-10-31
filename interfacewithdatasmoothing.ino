#include <MQ135.h>
#include <DHT.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Fonts/Org_01.h>

#define ADC_VREF_mV    3300.0
#define ADC_RESOLUTION 4096.0
#define PIN_LM35       33
#define PIN_MQ135      35
#define PIN_DHT22      26
#define i2c_Address 0x3c
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT dht(PIN_DHT22, DHT22);
MQ135 mq135_sensor(PIN_MQ135, 627.54); //100

#include <FluxGarage_RoboEyes.h>
roboEyes roboEyes;
int startupTime = 0;

int count = 0;

const int numValues = 5;  // Number of values to average
int values[numValues];    // Array to store the last 5 readings
int currentIndex = 0;            // currentIndex to keep track of the current position
bool isBufferFull = false; // Indicates if the buffer has filled

void setup()
{
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
}
void loop()
{
  float humidity = dht.readHumidity();
  float tempDHT22 = dht.readTemperature();
  int adcVal = analogRead(PIN_LM35);
  float milliVolt = adcVal * (ADC_VREF_mV / ADC_RESOLUTION);
  float tempLM35 = milliVolt / 10;
  float avetemp = (tempDHT22 + tempLM35) / 2;
  float rzero = mq135_sensor.getRZero();
  float correctedRZero = mq135_sensor.getCorrectedRZero(tempDHT22, humidity);
  float resistance = mq135_sensor.getResistance();
  float ppm = mq135_sensor.getPPM();
  float correctedPPM = mq135_sensor.getCorrectedPPM(tempDHT22, humidity);
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
  if ((correctedPPM >= 0) && (correctedPPM < 400))
  {
    Serial.println("Good");
  }
  if ((correctedPPM >= 400) && (correctedPPM < 1000))
  {
    Serial.println("Normal");
  }
  if ((correctedPPM >= 1000) && (correctedPPM < 2000))
  {
    Serial.println("Poor");
  }
  if ((correctedPPM >= 2000) && (correctedPPM < 5000))
  {
    Serial.println("Unhealthy");
  }
  if (correctedPPM >= 5000)
  {
    Serial.println("HAZARDOUS");
  }
  Serial.println(" ");

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

  values[currentIndex] = correctedPPM; // Store the new value in the array at the current currentIndex
  currentIndex = (currentIndex + 1) % numValues; // Update the currentIndex
  if (currentIndex == 0) // Check if the buffer is full at least once
  {
    isBufferFull = true;
  }
  if (isBufferFull) // Calculate the average only if the buffer is full
  {
    int sum = 0;
    for (int i = 0; i < numValues; i++)
    {
      sum += values[i];
    }
    float average = sum / (float)numValues;
    Serial.print("Average Corrected PPM of last 5 values: ");
    Serial.println(average);
  }
  //delay(2000);

  display.setCursor(0, 55);
  display.print("AIR QUALITY:");
  if ((correctedPPM >= 0) && (correctedPPM < 400))
  {
    display.setCursor(70, 55);
    display.println("GOOD");
  }
  if ((correctedPPM >= 400) && (correctedPPM < 1000))
  {
    display.setCursor(70, 55);
    display.println("NORMAL");
  }
  if ((correctedPPM >= 1000) && (correctedPPM < 2000))
  {
    display.setCursor(70, 55);
    display.println("POOR");
  }
  if ((correctedPPM >= 2000) && (correctedPPM < 5000))
  {
    display.setCursor(70, 55);
    display.println("UNHEALTHY");
  }
  if (correctedPPM >= 5000)
  {
    display.setCursor(70, 55);
    display.println("HAZARDOUS");
  }

  delay(1);
  display.display();
  //Blynk.run();
  delay(2000); //vary to your liking
}
