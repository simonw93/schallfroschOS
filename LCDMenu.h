#ifndef LCD_MENU_h
#define LCD_MENU_h

#include "Arduino.h"
#include <LiquidCrystal_I2C.h>

class LCDMenu
{
  public:
    LCDMenu();
    void init(LiquidCrystal_I2C *pLcd);
    void refreshData(struct displayData newData);
    void updateDisplay();
    
  private:
    void displayScreensaver();
    void displayMainMenu();
    void displaySubMenu();
};



#endif
