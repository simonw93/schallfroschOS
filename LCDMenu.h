#ifndef LCD_MENU_h
#define LCD_MENU_h

#include "Arduino.h"
#include <LiquidCrystal_I2C.h>
#include <Rotary.h> //http://www.buxtronix.net/2011/10/rotary-encoders-done-properly.html

class LCDMenu
{
  public:
    LCDMenu();
    void init(LiquidCrystal_I2C *pLcd, Rotary *pRotary, String pVersion);
    void refreshData(struct displayData newData);
    void setChanged();
    void updateDisplay();
    void loop();
    void rotate();
    void registerCallbacks(void (*refresh_data)());
    void printBootScreen();

  private:
    void displayScreensaver();
    void displayMainMenu();
    void displaySubMenu();
    void handleEncoder();
    void checkEncoderButton();
    void handleShortPress();
    void handleLongPress();
    void handleLeft();
    void handleRight();
};



#endif
