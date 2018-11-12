#include "LCDMenu.h"
#include <LiquidCrystal_I2C.h>
#include <Rotary.h> //http://www.buxtronix.net/2011/10/rotary-encoders-done-properly.html
#include <Arduino.h>
#include <string> // for string class 
#include "Constants.h"
#include "SchallfroschBackend.h"

using namespace std;

/**
   TODO: Create a more flexible menu.
*/

/**
   Rotary encoder variables
*/

// encoder position
volatile int encoderCount = 0;
boolean button_set = false;
boolean buttonLongPressed = false;
long encoderButtonTimer = 0; //Timer for when the encoder button was pressed the last time
const long encoderButtonLongPressTime = 500;


/**
   Linked list
*/
struct menItem {
  struct menItem *parent, *child, *next, *prev;
  String text;
  void (*func)();
};

/**
   List menu entries.
*/
struct menItem screensaver = {0, 0, 0, 0, "Screensaver", 0}; // screensaver dummy node
struct menItem *current; // current menu item to display

struct menItem m0lautstaerke = {0, 0, 0, 0, "Lautstaerke", 0};
struct menItem m1_0lautstaerke = {0, 0, 0, 0, "", 0}; // volume

struct menItem m0energie = {0, 0, 0, 0, "Energie", 0};
struct menItem m1_0energie = {0, 0, 0, 0, "ECO-Modus", 0};
struct menItem m1_1energie = {0, 0, 0, 0, "SPL-Modus", 0};

struct menItem m0geraete = {0, 0, 0, 0, "Geraete", 0};
struct menItem m1_0geraete = {0, 0, 0, 0, "12V: ", 0};
struct menItem m1_1geraete = {0, 0, 0, 0, "USB-Lader: ", 0};
struct menItem m1_2geraete = {0, 0, 0, 0, "Soundboard: ", 0};

struct menItem m0akku = {0, 0, 0, 0, "Akku", 0};
struct menItem m1_0akku = {0, 0, 0, 0, "Ladestand: ", 0};
struct menItem m1_1akku = {0, 0, 0, 0, "Rest: ", 0};
struct menItem m1_2akku = {0, 0, 0, 0, "Ladestand: ", 0};
struct menItem m1_3akku = {0, 0, 0, 0, "Lader: ", 0};
struct menItem m1_4akku = {0, 0, 0, 0, "Statistik: ", 0};
struct menItem m2_0akku = {0, 0, 0, 0, "Laufzeit: ", 0};

struct menItem m0signal = {0, 0, 0, 0, "Signalquelle", 0};
struct menItem m1_0signal = {0, 0, 0, 0, "Bluetooth", 0};
struct menItem m1_1signal = {0, 0, 0, 0, "AUX", 0};
struct menItem m1_2signal = {0, 0, 0, 0, "Radio", 0};
struct menItem m1_3signal = {0, 0, 0, 0, "Raspberry", 0};
struct menItem m1_4signal = {0, 0, 0, 0, "Mikrofon", 0};

struct menItem m0radio = {0, 0, 0, 0, "Radio", 0};
struct menItem m1_0radio = {0, 0, 0, 0, "Frequenz", 0};
struct menItem m2_0radio = {0, 0, 0, 0, "", 0}; // Frequenz aendern
struct menItem m1_1radio = {0, 0, 0, 0, "Senderliste", 0};

struct menItem m0beleuchtung = {0, 0, 0, 0, "Beleuchtung", 0};
struct menItem m1_0beleuchtung = {0, 0, 0, 0, "Einschalten", 0}; // an/aus
struct menItem m1_1beleuchtung = {0, 0, 0, 0, "Statisch", 0};
struct menItem m1_2beleuchtung = {0, 0, 0, 0, "Sound2Light", 0};
struct menItem m1_3beleuchtung = {0, 0, 0, 0, "Farbwechsel", 0};
struct menItem m1_4beleuchtung = {0, 0, 0, 0, "Flutlicht", 0};

struct menItem m0schliessfach = {0, 0, 0, 0, "Schliessfach", 0};
struct menItem m1_0schliessfach = {0, 0, 0, 0, "Oeffnen", 0};

struct menItem m0Alarm = {0, 0, 0, 0, "Alarmoptionen", 0};
struct menItem m1_0Alarm = {0, 0, 0, 0, "Bewegungsalarm", 0};
struct menItem m1_1Alarm = {0, 0, 0, 0, "Standortalarm", 0};
struct menItem m1_2Alarm = {0, 0, 0, 0, "Deaktivieren", 0};
struct menItem m1_3Alarm = {0, 0, 0, 0, "Einstellungen", 0};
struct menItem m2_0Alarm = {0, 0, 0, 0, "Sens.: ", 0};
struct menItem m2_1Alarm = {0, 0, 0, 0, "Radius: ", 0};

struct menItem m0DSP = {0, 0, 0, 0, "DSP-Optionen", 0};
struct menItem m1_0DSP = {0, 0, 0, 0, "Bass Boost", 0};
struct menItem m1_1DSP = {0, 0, 0, 0, "Max SPL", 0};
struct menItem m1_2DSP = {0, 0, 0, 0, "Subwoofer", 0};

struct menItem m0WiFi = {0, 0, 0, 0, "WiFi-Optionen", 0};
struct menItem m1_0WiFi = {0, 0, 0, 0, "Einschalten", 0};
struct menItem m1_1WiFi = {0, 0, 0, 0, "Verb. Geraete", 0};
struct menItem m1_2WiFi = {0, 0, 0, 0, "Passwort", 0};

struct menItem m0Berechtigungen = {0, 0, 0, 0, "Berechtigungen", 0};

struct menItem m0Debug = {0, 0, 0, 0, "Debug", 0};

struct menItem m0Administration = {0, 0, 0, 0, "Administration", 0};

struct menItem m0Soundboard = {0, 0, 0, 0, "Soundboard", 0};

struct menItem m0Sperren = {0, 0, 0, 0, "Sperren", 0};


/**
   Links the existing nodes to create the menu structure.
*/
void LCDMenu::linkNodes() {
  current = &screensaver;
  screensaver.prev = &screensaver;
  screensaver.next = &screensaver;
  screensaver.parent = &screensaver;
  screensaver.child = &m0lautstaerke;

  m0lautstaerke.parent = &screensaver;
  m0lautstaerke.child = &m1_0lautstaerke;
  m0lautstaerke.next = &m0energie;
  m0lautstaerke.prev = &m0geraete; //&m0Sperren; //DEBUG
  m1_0lautstaerke.parent = &m0lautstaerke;
  m1_0lautstaerke.next = &m1_0lautstaerke;
  m1_0lautstaerke.prev = &m1_0lautstaerke;


  m0energie.parent = &screensaver;
  m0energie.child = &m1_0energie;
  m0energie.next = &m0geraete;
  m0energie.prev = &m0lautstaerke;
  m1_0energie.parent = &m0energie;
  m1_0energie.next = &m1_1energie;
  m1_0energie.prev = &m1_1energie;
  m1_1energie.parent = &m0energie;
  m1_1energie.next = &m1_0energie;
  m1_1energie.prev = &m1_0energie;

  m0geraete.parent = &screensaver;
  m0geraete.child = &m1_0geraete;
  m0geraete.next = &m0akku; // DEBUG
  m0geraete.prev = &m0energie;
  m1_0geraete.parent = &m0geraete;
  m1_0geraete.next = &m1_1geraete;
  m1_0geraete.prev = &m1_2geraete;
  m1_1geraete.parent = &m0geraete;
  m1_1geraete.next = &m1_2geraete;
  m1_1geraete.prev = &m1_0geraete;
  m1_2geraete.parent = &m0geraete;
  m1_2geraete.next = &m1_0geraete;
  m1_2geraete.prev = &m1_1geraete;

  m0akku.parent = &screensaver;
  m0akku.child = &m1_0akku;
  m0akku.next = &m0signal;
  m0akku.prev = &m0geraete;
  m1_0akku.parent = &m0akku;
  m1_0akku.next = &m1_1akku;
  m1_0akku.prev = &m1_4akku;
  m1_1akku.parent = &m0akku;
  m1_1akku.next = &m1_2akku;
  m1_1akku.prev = &m1_0akku;
  m1_2akku.parent = &m0akku;
  m1_2akku.next = &m1_3akku;
  m1_2akku.prev = &m1_1akku;
  m1_3akku.parent = &m0akku;
  m1_3akku.next = &m1_4akku;
  m1_3akku.prev = &m1_2akku;
  m1_4akku.parent = &m0akku;
  m1_4akku.next = &m1_0akku;
  m1_4akku.prev = &m1_3akku;
  m1_4akku.child = &m2_0akku;
  m2_0akku.parent = &m1_4akku;
  m2_0akku.next = &m2_0akku;
  m2_0akku.prev = &m2_0akku;

  m0signal.parent = &screensaver;
  m0signal.next = &m0radio;
  m0signal.prev = &m0akku;
  m0signal.child = &m1_0signal;
  m1_0signal.parent = &m0signal;
  m1_0signal.next = &m1_1signal;
  m1_0signal.prev = &m1_4signal;
  m1_1signal.parent = &m0signal;
  m1_1signal.next = &m1_2signal;
  m1_1signal.prev = &m1_0signal;
  m1_2signal.parent = &m0signal;
  m1_2signal.next = &m1_3signal;
  m1_2signal.prev = &m1_1signal;
  m1_3signal.parent = &m0signal;
  m1_3signal.next = &m1_4signal;
  m1_3signal.prev = &m1_2signal;
  m1_4signal.parent = &m0signal;
  m1_4signal.next = &m1_0signal;
  m1_4signal.prev = &m1_3signal;

  m0radio.parent = &screensaver;
  m0radio.next = &m0beleuchtung;
  m0radio.prev = &m0signal;
  m0radio.child = &m1_0radio;
  m1_0radio.parent = &m0radio;
  m1_0radio.next = &m1_1radio;
  m1_0radio.prev = &m1_1radio;
  m2_0radio.parent = &m1_0radio;
  m2_0radio.next = &m2_0radio;
  m2_0radio.prev = &m2_0radio;
  m1_1radio.parent = &m0radio;
  m1_1radio.next = &m1_0radio;
  m1_1radio.prev = &m1_0radio;

  m0beleuchtung.parent = &screensaver;
  m0beleuchtung.next = &m0schliessfach;
  m0beleuchtung.prev = &m0radio;
  m0beleuchtung.child = &m1_0beleuchtung;
  m1_0beleuchtung.parent = &m0beleuchtung;
  m1_0beleuchtung.next = &m1_1beleuchtung;
  m1_0beleuchtung.prev = &m1_4beleuchtung;
  m1_1beleuchtung.parent = &m0beleuchtung;
  m1_1beleuchtung.next = &m1_2beleuchtung;
  m1_1beleuchtung.prev = &m1_0beleuchtung;
  m1_2beleuchtung.parent = &m0beleuchtung;
  m1_2beleuchtung.next = &m1_3beleuchtung;
  m1_2beleuchtung.prev = &m1_1beleuchtung;
  m1_3beleuchtung.parent = &m0beleuchtung;
  m1_3beleuchtung.next = &m1_4beleuchtung;
  m1_3beleuchtung.prev = &m1_2beleuchtung;
  m1_4beleuchtung.parent = &m0beleuchtung;
  m1_4beleuchtung.next = &m1_0beleuchtung;
  m1_4beleuchtung.prev = &m1_3beleuchtung;

  m0schliessfach.parent = &screensaver;
  m0schliessfach.next = &m0Alarm;
  m0schliessfach.prev = &m0beleuchtung;
  m0schliessfach.child = &m1_0schliessfach;
  m1_0schliessfach.parent = &m0schliessfach;
  m1_0schliessfach.next = &m1_0schliessfach;
  m1_0schliessfach.prev = &m1_0schliessfach;

  m0Alarm.parent = &screensaver;
  m0Alarm.next = &m0DSP;
  m0Alarm.prev = &m0schliessfach;
  m0Alarm.child = &m1_0Alarm;
  m1_0Alarm.parent = &m0Alarm;
  m1_0Alarm.next = &m1_1Alarm;
  m1_0Alarm.prev = &m1_3Alarm;
  m1_1Alarm.parent = &m0Alarm;
  m1_1Alarm.next = &m1_2Alarm;
  m1_1Alarm.prev = &m1_0Alarm;
  m1_2Alarm.parent = &m0Alarm;
  m1_2Alarm.next = &m1_3Alarm;
  m1_2Alarm.prev = &m1_1Alarm;
  m1_3Alarm.parent = &m0Alarm;
  m1_3Alarm.next = &m1_0Alarm;
  m1_3Alarm.prev = &m1_2Alarm;
  m2_0Alarm.parent = &m1_3Alarm;
  m2_0Alarm.next = &m2_1Alarm;
  m2_0Alarm.prev = &m2_1Alarm;
  m2_1Alarm.parent = &m1_3Alarm;
  m2_1Alarm.next = &m2_0Alarm;
  m2_1Alarm.prev = &m2_0Alarm;

  m0DSP.parent = &screensaver;
  m0DSP.next = &m0WiFi;
  m0DSP.prev = &m0Alarm;
  m0DSP.child = &m1_0DSP;
  m1_0DSP.parent = &m0DSP;
  m1_0DSP.next = &m1_1DSP;
  m1_0DSP.prev = &m1_2DSP;
  m1_1DSP.parent = &m0DSP;
  m1_1DSP.next = &m1_2DSP;
  m1_1DSP.prev = &m1_0DSP;
  m1_2DSP.parent = &m0DSP;
  m1_2DSP.next = &m1_0DSP;
  m1_2DSP.prev = &m1_1DSP;

  m0WiFi.parent = &screensaver;
  m0WiFi.next = &m0Berechtigungen;
  m0WiFi.prev = &m0DSP;
  m0WiFi.child = &m1_0WiFi;
  m1_0WiFi.parent = &m0WiFi;
  m1_0WiFi.next = &m1_1WiFi;
  m1_0WiFi.prev = &m1_2WiFi;
  m1_1WiFi.parent = &m0WiFi;
  m1_1WiFi.next = &m1_2WiFi;
  m1_1WiFi.prev = &m1_0WiFi;
  m1_2WiFi.parent = &m0WiFi;
  m1_2WiFi.next = &m1_0WiFi;
  m1_2WiFi.prev = &m1_1WiFi;

  m0Berechtigungen.parent = &screensaver;
  m0Berechtigungen.next = &m0Debug;
  m0Berechtigungen.prev = &m0WiFi;

  m0Debug.parent = &screensaver;
  m0Debug.next = &m0Administration;
  m0Debug.prev = &m0Berechtigungen;

  m0Administration.parent = &screensaver;
  m0Administration.next = &m0Soundboard;
  m0Administration.prev = &m0Debug;

  m0Soundboard.parent = &screensaver;
  m0Soundboard.next = &m0Sperren;
  m0Soundboard.prev = &m0Administration;

  m0Sperren.parent = &screensaver;
  m0Sperren.next = &m0lautstaerke;
  m0Sperren.prev = &m0Soundboard;
}

/**
   Constants
*/
const String cursorChar = "#";

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

// const int subSubMenuSizes[mainMenuSize][];


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

// saves the menu position.
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
SchallfroschBackend *sf;

/**
   Empty constructor.
*/
LCDMenu::LCDMenu() {
}

/**
   Initialize the lcd menu. Sets pointers to instances of necessary libraries. Call this once after creating an instance in the main sketch!
*/
void LCDMenu::init(LiquidCrystal_I2C *pLcd, Rotary *pRotary, SchallfroschBackend *pSf) {
  myLcd = pLcd;
  myRotary = pRotary;
  sf = pSf;
  myLcd->createChar(6, cross); // create cursor character
  linkNodes();
  //current = &screensaver;
  myLcd->clear();
  printBootScreen();
}

/**
   Call this frequently. Does all the necessary display tasks.
*/
void LCDMenu::loop() {
  handleEncoder();
  checkEncoderButton();
  if (sf->getNotify()) {
    setChanged();
  }

  if ((millis() > display_refresh_timer) && (millis() > Constants::BOOT_SCREEN_DURATION) && hasChanged) {
    updateDisplay();
    hasChanged = false;
    //const char *cstr = current->text.c_str();
    //Serial.println(cstr);
    Serial.println("Display refreshed!");
    display_refresh_timer = millis() + Constants::DISPLAY_REFRESH_RESET;
  }
}

/**
   Call this whenever display data has been changed.
*/
void LCDMenu::setChanged() {
  hasChanged = true;
}

/**
   Updates the display.
*/
void LCDMenu::updateDisplay() {
  myLcd->clear();
  printMenu();
}

/**
   Displays the screensaver.
*/
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
/**
   TODO: Concatenate battery level string with battery value!
*/
void LCDMenu::displaySubMenu() {
  // Set variable menu content

  if (sf->isCharging()) { // Laden
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

void LCDMenu::displaySubSubMenu() {

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
  if (current == &screensaver) {
    current = &m0lautstaerke;
  }
  if (current->func != 0) {
    // call linked function
  }
  else if (current-> child != 0) {
    current = current->child;
  }
  setChanged();
}

void LCDMenu::handleLongPress() {
  if (current == &screensaver) {

  }
  else {
    current = current->parent;
  }
  setChanged();
}

void LCDMenu::handleLeft() {
  if (current == &screensaver) {
    //decrease volume
    //setChanged();
  }
  else {
    decMenu();
    setChanged();
  }

}

void LCDMenu::handleRight() {
  if (current == &screensaver) {
    //increase volume
    //setChanged
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
  if (digitalRead(Constants::ENCODER_BUTTON) == LOW) {
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
  if (current->prev != 0) {
    current = current->prev;
  }
}

void LCDMenu::incMenu() {
  if (current->next != 0) {
    current = current->next;
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

/**
   Print the menu according to current state.
*/
void LCDMenu::printMenu() {
  if (current == &screensaver) {
    // display screensaver
  }
  else {
    int cDepth = getDepth(current);
    int linesToPrint = min(4, cDepth); // calculate how many lines to print
    struct menItem *cNode = current;

    Serial.print("Active menu node is ");
    Serial.println(cNode->text);
    Serial.print("Depth of active menu layer is ");
    Serial.println(cDepth);

    myLcd->setCursor(0, 0); // print cursor
    myLcd->print(cursorChar); // TODO: change this to the cursor character
    for (int i = 0; i < linesToPrint; i++) { //display entries
      myLcd->setCursor(1, i);
      //char *cstr = &cNode->text[0u];
      if (cNode->text != 0) {
        myLcd->print(cNode->text);
      }
      if (cNode->next != 0) {
        cNode = cNode->next;
      }

      else {
        Serial.println("cNode -> next is empty!");
        Serial.print("Active menu node is ");
        Serial.println(cNode->text);
      }
    }
  }
}

/**
   Returns the depth of a node by iteration.
   @param node: The node
*/
int LCDMenu::getDepth(struct menItem *node) {
  if (node->next == node) { //node has no next nodes
    return 0;
  }
  struct menItem *cNode = node->next; // set temporary current node
  int depth = 1;

  while (cNode != node) {
    if (cNode->next != 0) {
      cNode = cNode->next;
    }
    depth ++;
    if (depth > 100) { // safety measure to not end in an endless loop
      break;
    }
  }
  return depth;
}
