// SPDX-FileCopyrightText: 2022 Limor Fried for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#include <Arduino.h>
#include "Adafruit_MAX1704X.h"
//The MAX17048 LiPoly Battery Monitor reports the voltage and charge percent over I2C. Connect it to your Lipoly or LiIon battery and it will let you know the voltage of the cell, and it does the annoying math of decoding the non-linear voltage to get you a valid percentage as well! The battery monitor is available over I2C on address 0x36
#include "Adafruit_LC709203F.h" //Batterie Monitor
//The Adafruit LC709203F LiPoly / LiIon Fuel Gauge and Battery Monitor reports the voltage and charge percent over I2C. Connect it to your Lipoly or LiIon battery and it will let you know the voltage of the cell, and it does the annoying math of decoding the non-linear voltage to get you a valid percentage as well! The battery monitor is available over I2C on address 0x0B.
#include <Adafruit_NeoPixel.h>
#include "Adafruit_TestBed.h"
#include <Adafruit_BME280.h>
//BME280 Temperature, Humidity and Barometric Pressure Sensor.
//There is currently no BME280 sensor shipped on the ESP32-S2 TFT Feather - only a space for it!
#include <Adafruit_ST7789.h> //Color 1.14" IPS TFT with 240x135 pixels 
#include <Fonts/FreeSans12pt7b.h>

Adafruit_BME280 bme; // I2C
bool bmefound = false;
extern Adafruit_TestBed TB;

Adafruit_LC709203F lc_bat; //Batterie Monitor
Adafruit_MAX17048 max_bat;

Adafruit_ST7789 display = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

GFXcanvas16 canvas(240, 135);

bool maxfound = false;
bool lcfound = false;

void setup() {
  Serial.begin(115200);
  // while (! Serial) delay(10);
  
  delay(100);
  
  // turn on the TFT / I2C power supply
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);

  pinMode(NEOPIXEL_POWER, OUTPUT);
  digitalWrite(NEOPIXEL_POWER, HIGH);
  delay(10);
  
  TB.neopixelPin = PIN_NEOPIXEL;
  TB.neopixelNum = 1; 
  TB.begin();
  TB.setColor(WHITE);

  display.init(135, 240);           // Init ST7789 240x135
  display.setRotation(3);
  canvas.setFont(&FreeSans12pt7b);
  canvas.setTextColor(ST77XX_WHITE); 

   if (lc_bat.begin()) {
    Serial.println("Found LC709203F");
    Serial.print("Version: 0x"); Serial.println(lc_bat.getICversion(), HEX);
    lc_bat.setPackSize(LC709203F_APA_500MAH);
    lcfound = true;
  }
  else {
    Serial.println(F("Couldnt find Adafruit LC709203F?\nChecking for Adafruit MAX1704X.."));
    delay(200);
    if (!max_bat.begin()) {
      Serial.println(F("Couldnt find Adafruit MAX1704X?\nMake sure a battery is plugged in!"));
      while (1) delay(10);
    }
    Serial.print(F("Found MAX17048"));
    Serial.print(F(" with Chip ID: 0x")); 
    Serial.println(max_bat.getChipID(), HEX);
    maxfound = true;
    
  } 

  if (TB.scanI2CBus(0x77)) {
    Serial.println("BME280 address");

    unsigned status = bme.begin();  
    if (!status) {
      Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
      Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
      Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
      Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
      Serial.print("        ID of 0x60 represents a BME 280.\n");
      Serial.print("        ID of 0x61 represents a BME 680.\n");
      return;
    }
    Serial.println("BME280 found OK");
    bmefound = true;
  }
}

uint8_t j = 0;

void loop() {

  if (j % 5 == 0) {
    Serial.println("**********************");

    TB.printI2CBusScan();
    canvas.fillScreen(ST77XX_BLACK);
    canvas.setCursor(0, 25);
    canvas.setTextColor(ST77XX_RED);
    canvas.println("Adafruit Feather !!");
    canvas.setTextColor(ST77XX_YELLOW);
    canvas.println("ESP32-S2 TFT Demo");
    canvas.setTextColor(ST77XX_GREEN); 
    canvas.print("Battery: ");
    canvas.setTextColor(ST77XX_WHITE);
    if (lcfound == true) {
      canvas.print(lc_bat.cellVoltage(), 1);
      canvas.print(" V  /  ");
      canvas.print(lc_bat.cellPercent(), 0);
      canvas.println("%");
    }
    else {
      canvas.print(max_bat.cellVoltage(), 1);
      canvas.print(" V  /  ");
      canvas.print(max_bat.cellPercent(), 0);
      canvas.println("%");
    }
    canvas.setTextColor(ST77XX_BLUE); 
    canvas.print("I2C: ");
    canvas.setTextColor(ST77XX_WHITE);
    for (uint8_t a=0x01; a<=0x7F; a++) {
      if (TB.scanI2CBus(a, 0))  {
        canvas.print("0x");
        canvas.print(a, HEX);
        canvas.print(", ");
      }
    }
    display.drawRGBBitmap(0, 0, canvas.getBuffer(), 240, 135);
    pinMode(TFT_BACKLITE, OUTPUT);
    digitalWrite(TFT_BACKLITE, HIGH);
  }
  
  TB.setColor(TB.Wheel(j++));
  return;
}

