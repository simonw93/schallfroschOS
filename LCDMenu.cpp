#include "LCDMenu.h"
#include <LiquidCrystal_I2C.h>
#include <Rotary.h> //http://www.buxtronix.net/2011/10/rotary-encoders-done-properly.html
#include <Arduino.h>

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

String schallfroschOS_version;

/**
   Constants
*/

const long menuScreensaver_period = 10000;

// Value for how long the volume should be displayed
const long volumeDisplay_period = 2000;

const long displayRefreshTimer_period = 50;

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

const int subMenuSizes[mainMenuSize] = {1, 2, 3, 4, 4, 3, 2, 1, 4, 1, 3, 3, 2, 2, 0};

String subMenuEntries[mainMenuSize][4] = {
  {"Lautstaerke: "},
  {"Eco Modus", "SPL Modus"},
  {"12V: ", "USB-Lader: ", "Soundboard: "},
  {"Ladestand: ", "Rest: ", "", "Statistik"}, // [3][2] wird geladen/-
  {"Bluetooth", "AUX", "Radio", "Raspberry"},
  {"Frequenz: ", "F. aendern", "Senderliste"},
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

int menPos[4] = {0,-1,-1,-1};

/**
   Timers
*/
// When value is reached, display can be refreshed again
long display_refresh_timer = 0;

//When value is reached, go back to the dashboard
long menuScreensaver = menuScreensaver_period;

//Amount of milliseconds that the bootscreen will be displayed
long bootScreenDuration = 3000;


/**
   Callback functions
*/
void (* menu_cb_refresh_data)();

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
    Contains information to display on the LCD. Is put together before updating the screen and handed over to the LCDMenu library.
*/
struct displayData
{
  int batteryLevel; // battery level in percent
  float batteryVoltage; // battery voltage in Volts
  int volume; // volume
  signalSource source; // signal source
  bool alarm; // true if alarm is active
  chargeMode currentChargeMode; // charge mode
  bool charging; // true if charging
  bool chargerConnected; // true if charger is connected
  int chargeCurrent; // charge current in mA
};



/**
   Variables, constants and timers
*/
struct displayData dData; // stores the latest received displayData
LiquidCrystal_I2C *myLcd;
Rotary *myRotary;




LCDMenu::LCDMenu() {
}

void LCDMenu::init(LiquidCrystal_I2C *pLcd, Rotary *pRotary, String pVersion) {
  schallfroschOS_version = pVersion;
  myLcd = pLcd;
  myRotary = pRotary;
  myLcd->createChar(6, cross); // create cursor character
  printBootScreen();
}

void LCDMenu::registerCallbacks(void (*refresh_data)()) {
  menu_cb_refresh_data = refresh_data;
}

void LCDMenu::loop() {
  handleEncoder();
  checkEncoderButton();

  if ((millis() > display_refresh_timer) && (millis() > bootScreenDuration) && hasChanged) {
    updateDisplay();
    hasChanged = false;
    display_refresh_timer = millis() + displayRefreshTimer_period;
  }
}

/**
   Refresh the stored displayData.
*/
void LCDMenu::refreshData(struct displayData newData) {
  dData = newData;
}

void LCDMenu::setChanged() {
  hasChanged = true;
}

void LCDMenu::updateDisplay() {
  if (menPos[1] == -1) { // menu is inactive -> screensaver mode
    displayScreensaver();
  }
  else if (menPos[2] == -1) { //sub menu is inactive, but menu is active -> main menu
    displayMainMenu();
  }
  else if (menPos[3] == -1) { // sub-submenu is inactive, but submenu is active -> submenu
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
  if (dData.charging) { // Laden
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
  //myLcd->setCursor(0, 0); // print cursor
  //myLcd->print("#"); // TODO: change this to the cursor character
  if (subMenuSizes[menPos[1]] >= 4) {
    myLcd->setCursor(0, 0); // print cursor
    myLcd->print("#"); // TODO: change this to the cursor character
    for (int i = 0; i < 4; i++) { //display four entries
      myLcd->setCursor(1, i);
      myLcd->print(subMenuEntries[menPos[1]][(indexToDisplay + i) % subMenuSizes[menPos[1]]]);
    }
  }
  else if (subMenuSizes[menPos[1]] > 0) {
    for (int i = 0; i < subMenuSizes[menPos[1]]; i++) {
      if (i == indexToDisplay) {
        myLcd->setCursor(0, 0); // print cursor
        myLcd->print("#"); // TODO: change this to the cursor character
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
  myLcd->print(schallfroschOS_version);
  myLcd->setCursor(0, 2);
  myLcd->print("von Simon Welzel");
}

/**
   Rotary encoder section
*/

void LCDMenu::handleShortPress() {
  // handle short press
}

void LCDMenu::handleLongPress() {
  // handle long press
}

void LCDMenu::handleLeft() {
  // handle left
}

void LCDMenu::handleRight() {
  // handle right
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
