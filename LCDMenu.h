#ifndef LCD_MENU_h
#define LCD_MENU_h

#include "Arduino.h"
#include <LiquidCrystal_I2C.h>
#include <Rotary.h> //http://www.buxtronix.net/2011/10/rotary-encoders-done-properly.html

class LCDMenu
{
  public:
    LCDMenu();
    void init(LiquidCrystal_I2C *pLcd, Rotary *pRotary);
    void refreshData(struct displayData newData);
    void updateDisplay();
    void update();
    void rotate();
    void registerCallbacks(void (*refresh_data)());

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
