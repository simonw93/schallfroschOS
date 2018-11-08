#include "LCDMenu.h"
#include <LiquidCrystal_I2C.h>
#include <Rotary.h> //http://www.buxtronix.net/2011/10/rotary-encoders-done-properly.html
#include <Arduino.h>
#include "Constants.h"

/**
   Rotary encoder variables
*/

// encoder position
volatile int encoderCount = 0;
boolean button_set = false;
boolean buttonLongPressed = false;
long encoderButtonTimer = 0; //Timer for when the encoder button was pressed the last time
const long encoderButtonLongPressTime = 500;
const int encoderPinA = 18;   // right
const int encoderPinB = 17;   // left
const int encoderButton = 19; // switch


/**
   Constants
*/

const int mainMenuSize = 15;
// main menu entry names
String mainMenuEntries[mainMenuSize] = {
  "Lautstaerke",
  "Energiemodus",
  "Geraete",
  "Akku",
  "Signalquelle",
  "Radio",
  "Beleuchtung",
  "Schliessfach",
  "Alarm",
  "DSP",
  "WiFi",
  "Berechtigungen",
  "Debug",
  "Administration",
  "Sperren"
};

const int subMenuSizes[mainMenuSize] = {1, 2, 3, 4, 4, 2, 2, 1, 4, 1, 3, 3, 2, 2, 0};

String subMenuEntries[mainMenuSize][4] = {
  {"Lautstaerke: "},
  {"Eco Modus", "SPL Modus"},
  {"12V: ", "USB-Lader: ", "Soundboard: "},
  {"Ladestand: ", "Rest: ", "", "Statistik"}, // [3][2] wird geladen/-
  {"Bluetooth", "AUX", "Radio", "Raspberry"},
  {"Frequenz: ", "Senderliste"},
  {"", "Modus"}, // [6][0] Beleuchtung an/aus
  {"Fach oeffnen"},
  {"Bewegungsalarm", "Standortalarm", "Deaktivieren", "Einstellungen"},
  {"Audioprofile"},
  {"", "Verb. Geraete", "Passw. zeigen"}, // [10][0] WiFi an/ WiFi aus
  {"Max. Lautst.", "USB-Lader", "Min. Akkustand"},
  {"Sensoren", "Version"},
  {"Nutzer hinzuf.", "Nutzer entf."},
  {}
};


// character to indicate cursor position
uint8_t cross[8] = {0x0, 0x1b, 0xe, 0x4, 0xe, 0x1b, 0x0};

/**
   Variables
*/
// if true, display content has changed and will be rewritten
bool hasChanged = false;

// true if display is on
bool displayActive = false;

// true if a submenu entry has been clicked
bool entryClicked = false;

int menPos[3] = {0, -1, -1};

/**
   Timers
*/
// When value is reached, display can be refreshed again
long display_refresh_timer = 0;

//When value is reached, go back to the dashboard
long menuScreensaver = Constants::MENU_SCREENSAVER_RESET;

// When value is reached, turn off display
long display_on_timer = Constants::DISPLAY_ON_TIMER_RESET;

// When value is reached, display is no longer locked to displaying volume
long volumeDisplayTimer = 0;



/**
   Enum definitions, struct definitions
*/
enum chargeMode {
  CHARGE_NORMAL, // Charge up to 80% of the maximum battery capacity
  CHARGE_MAX,    // Charge to 100% of the maximum battery capacity
  CHARGE_NONE    // Do not charge at all
};

enum signalSource {
  SOURCE_BLUETOOTH,
  SOURCE_AUX,
  SOURCE_RADIO,
  SOURCE_RASPBERRY
};

/**
   Variables, constants and timers
*/
LiquidCrystal_I2C *myLcd;
Rotary *myRotary;




LCDMenu::LCDMenu() {
}

void LCDMenu::init(LiquidCrystal_I2C *pLcd, Rotary *pRotary) {
  myLcd = pLcd;
  myRotary = pRotary;
  myLcd->createChar(6, cross); // create cursor character
  printBootScreen();
}


void LCDMenu::loop() {
  handleEncoder();
  checkEncoderButton();

  if ((millis() > display_refresh_timer) && (millis() > Constants::BOOT_SCREEN_DURATION) && hasChanged) {
    updateDisplay();
    hasChanged = false;
    display_refresh_timer = millis() + Constants::DISPLAY_REFRESH_RESET;
  }
}

void LCDMenu::setChanged() {
  hasChanged = true;
}

void LCDMenu::updateDisplay() {
  myLcd->clear();
  if (menPos[1] == -1) { // menu is inactive -> screensaver mode
    displayScreensaver();
  }
  else if (menPos[2] == -1) { //sub menu is inactive, but menu is active -> main menu
    displayMainMenu();
  }
  else { //  -> submenu
    displaySubMenu();
  }
}

void LCDMenu::displayScreensaver() {
  switch (menPos[0]) { //
    case 0:
      myLcd->setCursor(0, 0);
      myLcd->print("Screensaver 1");
      break;
    default:
      break;
  }
}

/**
   Displays the main menu.
*/
void LCDMenu::displayMainMenu() {
  int indexToDisplay = menPos[1];
  myLcd->setCursor(0, 0); // print cursor
  myLcd->print("#"); // TODO: change this to the cursor character
  for (int i = 0; i < 4; i++) { //display four entries
    myLcd->setCursor(1, i);
    myLcd->print(mainMenuEntries[(indexToDisplay + i) % mainMenuSize]);
  }
}

void LCDMenu::displaySubMenu() {
  // Set variable menu content

  if (false) { // Laden
    subMenuEntries[3][2] = "Wird geladen";
  }
  else {
    subMenuEntries[3][2] = "Akkubetrieb";
  }
  if (true) { // Beleuchtung
    subMenuEntries[6][0] = "Ausschalten";
  }
  else {
    subMenuEntries[6][0] = "Einschalten";
  }
  if (true) { // WiFi
    subMenuEntries[10][0] = "WiFi Aussch.";
  }
  else {
    subMenuEntries[10][0] = "WiFi Einsch.";
  }

  int indexToDisplay = menPos[2];

  String cursorChar = "#";
  if (entryClicked) {
    cursorChar = ">";
  }

  //myLcd->setCursor(0, 0); // print cursor
  //myLcd->print("#"); // TODO: change this to the cursor character
  if (subMenuSizes[menPos[1]] >= 4) {
    myLcd->setCursor(0, 0); // print cursor
    myLcd->print(cursorChar); // TODO: change this to the cursor character
    for (int i = 0; i < 4; i++) { //display four entries
      myLcd->setCursor(1, i);
      myLcd->print(subMenuEntries[menPos[1]][(indexToDisplay + i) % subMenuSizes[menPos[1]]]);
    }
  }
  else if (subMenuSizes[menPos[1]] > 0) {
    for (int i = 0; i < subMenuSizes[menPos[1]]; i++) {
      if (i == indexToDisplay) {
        myLcd->setCursor(0, i); // print cursor
        myLcd->print(cursorChar); // TODO: change this to the cursor character
      }
      myLcd->setCursor(1, i);
      myLcd->print(subMenuEntries[menPos[1]][i]);
    }
  }
  else {
    // do when no sub menu
  }
}

void LCDMenu::printBootScreen() {
  myLcd->setCursor(0, 0);
  myLcd->print("Schallfrosch OS");
  myLcd->setCursor(0, 1);
  myLcd->print(Constants::SCHALLFROSCH_VERSION);
  myLcd->setCursor(0, 2);
  myLcd->print("von Simon Welzel");
}

/**
   Rotary encoder section
*/

void LCDMenu::handleShortPress() {
  if (menPos[1] == -1) { // menu is inactive -> screensaver mode
    menPos[1] = 0;
    menPos[0] = -1;
    setChanged();
  }
  else if (menPos[2] == -1) { //sub menu is inactive, but menu is active -> main menu
    if (subMenuSizes[menPos[1]] > 0) { // check if submenu can be displayed
      menPos[2] = 0;
      setChanged();
    }
    else { // main menu
      switch (menPos[1]) {
        case 14:
          //lock system
          //setChanged();
          break;
        default:
          break;
      }
    }
  }
  else { // -> submenu
    // handle sub menu function calls
    /**
       "Lautstaerke",
      "Energiemodus",
      "Geraete",
      "Akku",
      "Signalquelle",
      "Radio",
      "Beleuchtung",
      "Schliessfach",
      "Alarm",
      "DSP",
      "WiFi",
      "Berechtigungen",
      "Debug",
      "Administration",
      "Sperren"
    */
    switch (menPos[1]) {
      case 1: // Energy
        switch (menPos[2]) {
          case 0:
            // go into eco mode
            break;
          case 1:
            // go into spl mode
            break;
        }
        break;
      case 2: // Devices
        switch (menPos[2]) {
          case 0:
            // toggle 12V
            break;
          case 1:
            // toggle USB charger
            break;
          case 2:
            // toggle soundboard
            break;
        }
        break;
      case 3: // Battery
        if (menPos[2] == 2) {
          // show statistics
        }
        break;
      case 4: // Signal source
        switch (menPos[2]) {
          case 0:
            //switch to bluetooth
            break;
          case 1:
            //switch to AUX
            break;
          case 2:
            //switch to radio
            break;
          case 3:
            //switch to raspberry
            break;
        }
        break;
      case 5: // Radio
        switch (menPos[2]) {
          case 0:
            entryClicked = true;
            break;
        }
        break;
      case 6: // Lighting
        break;
      case 7: // Locker
        break;
      case 8: // Alarm
        break;
      case 9: // DSP
        break;
      case 10: // WiFi
        break;
      case 11: // Berechtigungen
        break;
      case 12: // Debug
        break;
      case 13: // Admin
        break;
      default:
        break;
    }
    //setChanged();
  }
}

void LCDMenu::handleLongPress() {
  if (entryClicked) {
    entryClicked = false;
  }
  else if (menPos[1] >= 0 && menPos[2] == -1) { // menu is active
    menPos[1] = -1;
    menPos[0] = 0;
    setChanged();
  }
  else if (menPos[2] >= 0) { // sub menu is active
    menPos[2] = -1;
    setChanged();
  }
}

void LCDMenu::handleLeft() {
  if (entryClicked) {
    if (menPos[1] == 5) { //radio frequency menu
      //decrease frequency
    }
  }
  else if (menPos[1] == -1 || (menPos[1] == 0 && menPos[2] == 0)) { // volume change
    //(*menu_cb_dec_vol)();
    //(*menu_cb_refresh_data)();
    setChanged();
  }
  else {
    decMenu();
    setChanged();
  }

}

void LCDMenu::handleRight() {
  if (entryClicked) {
    if (menPos[1] == 5) { //radio frequency menu
      //increase frequency
    }
  }
  else if (menPos[1] == -1 || (menPos[1] == 0 && menPos[2] == 0)) { // volume change
    //(*menu_cb_inc_vol)();
    //(*menu_cb_refresh_data)();
    setChanged();
  }
  else {
    incMenu();
    setChanged();
  }
}

void LCDMenu::handleEncoder() {
  if (encoderCount > 0) { // if more right turns than left turns
    handleRight();
  }
  else if (encoderCount < 0) { // if more left turns than right turns
    handleLeft();
  }
  encoderCount = 0; // reset counters
}

// rotary encoder button handling
void LCDMenu::checkEncoderButton() {
  if (digitalRead(encoderButton) == LOW) {
    if (button_set == false) {
      button_set = true;
      encoderButtonTimer = millis(); // start timer
    }
    if ((millis() - encoderButtonTimer > encoderButtonLongPressTime) && (buttonLongPressed == false)) {
      // button is hold for a long time
      buttonLongPressed = true;
      handleLongPress();
    }
  } else { //if button is not pressed
    if (button_set == true) {
      if (buttonLongPressed == true) {
        buttonLongPressed = false;
      } else if (millis() - encoderButtonTimer > 50)  { //debounce
        // do short press routine stuff here
        handleShortPress();
      }
      button_set = false;
    }
  }
}

// rotate is called anytime the rotary inputs change state.
void LCDMenu::rotate() {
  unsigned char result = myRotary->process();
  if (result == DIR_CCW) {
    encoderCount++;
  } else if (result == DIR_CW) {
    encoderCount--;
  }
}

void LCDMenu::decMenu() {
  if (menPos[1] >= 0 && menPos[2] == -1) { // menu is active
    menPos[1] -= 1;
    if (menPos[1] < 0) {
      menPos[1] = mainMenuSize - 1; // reset position marker
    }
  }
  else if (menPos[2] >= 0) { // sub menu is active
    menPos[2] -= 1;
    if (menPos[2] < 0) {
      menPos[2] = subMenuSizes[menPos[1]] - 1; // reset position marker
    }
  }
}

void LCDMenu::incMenu() {
  if (menPos[1] >= 0 && menPos[2] == -1) { // menu is active
    menPos[1] += 1;
    if (menPos[1] >=  mainMenuSize) {
      menPos[1] = 0; // reset position marker
    }
  }
  else if (menPos[2] >= 0) { // sub menu is active
    menPos[2] += 1;
    if (menPos[2] >=  subMenuSizes[menPos[1]]) {
      menPos[2] = 0; // reset position marker
    }
  }
}

/**
   Turn display on or off.
*/
void LCDMenu::switchDisplay(bool value) {
  if (value && !displayActive) {
    displayActive = true;
    myLcd->backlight();
    display_on_timer = millis() + Constants::DISPLAY_ON_TIMER_RESET;
    // Turn on display
  }
  else if (!value && displayActive) {
    displayActive = false;
    display_on_timer = 0;
    myLcd->noBacklight();
    // turn off display
  }
}
