#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Fonts/FreeSansBold9pt7b.h>
#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
#include <DHT.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT dht(26, DHT22);

void setup(){
  Serial.begin(115200);
  /*serial begin of the DH11*/
  dht.begin();
  delay(250);
  /*initialize the serial address of the OLED Display*/
  display.begin(i2c_Address, true);
  /*clears the display*/
  //display.clearDisplay();
}

void loop()
{
  /*clears the display*/
  //display.clearDisplay();
  /*initalize the readings of the DH11's Temperature and Humidity*/
  dht.read();
  /*Font Style*/
  display.setFont(&FreeSansBold9pt7b);
  /*size of the text*/
  display.setTextSize(1);
  display.setCursor(0,15);
  /*display's the temperature in degree celcius unit*/
  display.print(dht.readTemperature());
  display.println("  *C");

  /*delay of the loop*/
  delay(1);
  /*display buffer*/
  display.display();

}
