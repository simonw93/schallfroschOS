/**
   Schallfrosch OS by Simon Welzel on 2018/11/06.

*/


#include <SPI.h>
#include <MFRC522.h> //https://www.arduinolibraries.info/libraries/mfrc522
#include <Wire.h>
#include <LiquidCrystal_I2C.h> //https://github.com/marcoschwartz/LiquidCrystal_I2C/
#include <Rotary.h> //http://www.buxtronix.net/2011/10/rotary-encoders-done-properly.html
#include "LCDMenu.h"
#include "SchallfroschBackend.h"
#include "Constants.h"

/**
   I2C mapping: SDA -> Pin 21
                SCL -> Pin 22
*/

/**
  ************************************************
   Object instances
*/
// Set the LCD address to 0x27 for a 16 chars and 4 line display
LiquidCrystal_I2C lcd(0x27, 16, 4);
//MFRC522 mfrc522 = MFRC522(ssPin, resetPin); // Create instance

Rotary rotary = Rotary(Constants::ENCODER_PIN_A, Constants::ENCODER_PIN_B);


LCDMenu lcdMenu = LCDMenu();

SchallfroschBackend sfBackend = SchallfroschBackend();


/**
   End of object instances
  ************************************************
*/



void setup() {
  // PIN MODE SETUP
  pinMode(Constants::ONBOARD_LED, OUTPUT); // set onboard LED pin to output

  // INITIALIZATIONS
  Wire.begin(); // initialize I2C
  Serial.begin(115200); // serial output for debugging
  lcd.begin(); // initialize the LCD
  lcdMenu.init(&lcd, &rotary, &sfBackend);


  // ROTARY ENCODER SETUP
  pinMode(Constants::ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(Constants::ENCODER_PIN_B, INPUT_PULLUP);
  pinMode(Constants::ENCODER_BUTTON, INPUT_PULLUP);
  attachInterrupt(Constants::ENCODER_PIN_A, encoderRotate, CHANGE);
  attachInterrupt(Constants::ENCODER_PIN_B, encoderRotate, CHANGE);

  // RFID522
  //SPI.begin();
  //mfrc522.PCD_Init();

}


void loop() {
  sfBackend.loop();
  lcdMenu.loop();

}

// handler for rotary encoder. TODO: Make nicer
void encoderRotate() {
  lcdMenu.rotate();
}
