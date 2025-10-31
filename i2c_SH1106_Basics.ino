#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include <FluxGarage_RoboEyes.h>
roboEyes roboEyes; // create RoboEyes instance


void setup()   {
  Serial.begin(115200);
  delay(250);
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
}

void loop()
{
  roboEyes.update();
}
