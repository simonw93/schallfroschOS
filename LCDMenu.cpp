#include "LCDMenu.h"
#include <LiquidCrystal_I2C.h>

/**
  Menüstruktur
  • Lautstärke
  ° Lautstärke ändern
  • Modus
  ° Eco (an/aus)
  • Geräte
  ° 12V an/aus
  ° USB-Lader an/aus
  ° Soundboard an/aus
  • Akku
  ° Statistiken anzeigen?
  ° Prozentanzeige, Spannung
  ° Restlaufzeit
  ° Energiesparmodus aktivieren
  • Signalquellen
  ° Bluetooth
  ° AUX
  ° Radio
  ° Raspberry Pi
  • Radio
  ° Frequenz anzeigen
  ° Frequenz ändern
  ° Senderliste
  • Beleuchtung
  ° Ein-/ Ausschalten
  ° Modi
      Automatisch
      Sound2Light
      Fade (Farbübergänge durchgehend)
      Statisch
      Programme (vordefinierte Abfolgen)
      Helligkeit
  • Schließfach
  ° Öffnen (braucht Authorisierung)
  • Alarm
  ° Bewegungsalarm Aktivieren (braucht Authorisierung)
  ° Standortalarm aktivieren (braucht Authorisierung)
  ° Deaktivieren (braucht Authorisierung)
  ° Einstellungen
      Alarmlautstärke
  • DSP
  • Audioprofile
    ° Indoor
    ° Outdoor
    ° Subwoofer aktiv (wenn Subs angeschlossen)
    ° MaxSPL
  • WiFi
  • Ein-/Ausschalten
  • verbundene Geräte anzeigen?
  • Verbindungsinformationen anzeigen
    ° erfordert Authorisierung
  • Einschränkungen
  ° Maximallautstärke
  ° USB-Lader
  ° Minimaler Ladestand
  • Debug
  ° Sensoren
      Temperaturen
      Accelerometer
      Ladestrom
      Laufzeit
  ° Version
  • Administration
  ° Token hinzufügen (Authorisierung)
  ° Token entfernen
  • Sperren

*/

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
  int menPos[4];
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


LCDMenu::LCDMenu() {
}

void LCDMenu::init(LiquidCrystal_I2C *pLcd) {
  myLcd = pLcd;
  myLcd->createChar(6, cross); // create cursor character

}

/**
   Refresh the stored displayData.
*/
void LCDMenu::refreshData(struct displayData newData) {
  dData = newData;
}

void LCDMenu::updateDisplay() {
  if (dData.menPos[1] == -1) { // menu is inactive -> screensaver mode
    displayScreensaver();
  }
  else if (dData.menPos[2] == -1) { //sub menu is inactive, but menu is active -> main menu
    displayMainMenu();
  }
  else if (dData.menPos[3] == -1) { // sub-submenu is inactive, but submenu is active -> submenu
    displaySubMenu();
  }

}

void LCDMenu::displayScreensaver() {
  switch (dData.menPos[0]) { //
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
  int indexToDisplay = dData.menPos[1];
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

  int indexToDisplay = dData.menPos[2];
  //myLcd->setCursor(0, 0); // print cursor
  //myLcd->print("#"); // TODO: change this to the cursor character
  if (subMenuSizes[dData.menPos[1]] >= 4) {
  myLcd->setCursor(0, 0); // print cursor
    myLcd->print("#"); // TODO: change this to the cursor character
    for (int i = 0; i < 4; i++) { //display four entries
      myLcd->setCursor(1, i);
      myLcd->print(subMenuEntries[dData.menPos[1]][(indexToDisplay + i) % subMenuSizes[dData.menPos[1]]]);
    }
  }
  else if (subMenuSizes[dData.menPos[1]] > 0) {
  for (int i = 0; i < subMenuSizes[dData.menPos[1]]; i++) {
      if (i == indexToDisplay) {
        myLcd->setCursor(0, 0); // print cursor
        myLcd->print("#"); // TODO: change this to the cursor character
      }
      myLcd->setCursor(1, i);
      myLcd->print(subMenuEntries[dData.menPos[1]][i]);
    }
  }
  else {
    // do when no sub menu
  }

}
