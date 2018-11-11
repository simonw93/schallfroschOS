#ifndef LCD_MENU_h
#define LCD_MENU_h

#include "Arduino.h"
#include <LiquidCrystal_I2C.h>
#include <Rotary.h> //http://www.buxtronix.net/2011/10/rotary-encoders-done-properly.html
#include "Constants.h"
#include "SchallfroschBackend.h"

class LCDMenu
{
  public:
    LCDMenu();
    void init(LiquidCrystal_I2C *pLcd, Rotary *pRotary, SchallfroschBackend *pSf);
    void refreshData(struct displayData newData);
    void setChanged();
    void updateDisplay();
    void loop();
    void rotate();
    void printBootScreen();

  private:
    void linkNodes();
    void printMenu();
    int getDepth(struct menItem *node);
    void displayScreensaver();
    void displayMainMenu();
    void displaySubMenu();
    void displaySubSubMenu();
    void handleEncoder();
    void checkEncoderButton();
    void handleShortPress();
    void handleLongPress();
    void handleLeft();
    void handleRight();
    void decMenu();
    void incMenu();
    void switchDisplay(bool value);
};



#endif
