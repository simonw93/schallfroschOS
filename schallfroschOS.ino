/**
   Schallfrosch OS by Simon Welzel on 2018/11/01.

*/


#include <SPI.h>
#include <MFRC522.h> //https://www.arduinolibraries.info/libraries/mfrc522
#include <Wire.h>
#include <LiquidCrystal_I2C.h> //https://github.com/marcoschwartz/LiquidCrystal_I2C/
#include <Rotary.h> //http://www.buxtronix.net/2011/10/rotary-encoders-done-properly.html



/**
 * ************************************************
   Constants
*/
// Defines how many times a measurement is repeated in order to stabilize the result.
int battery_voltage_iterations = 20;

// Size of the battery_voltages array.
const int battery_array_size = 50;

// Values for mapping the battery values.
const int battery_map_100 = 29; // When the voltage is at this level, it is mapped to 100%.

const int battery_map_80 = 27; // When the voltage is at this level, it is mapped to 80%.

const int battery_map_20 = 24; // When the voltage is at this level, it is mapped to 20%.

const int battery_map_0 = 20; // When the voltage is at this level, it is mapped to 0%.

// Value for how frequent the battery value should be refreshed. 10000 = refresh every 10 seconds.
const long battery_refresh_timer_period = 10000;

// Value for how frequent the RFID reader checks for new tags in proximity. 2000 = check every 2 seconds.
const long rfid_check_timer_period = 2000;

// Value for how long the display stays on.
const long display_on_timer_period = 30000;

const long menuScreensaver_period = 10000;

// Value for how long the volume should be displayed
const long volumeDisplay_period = 2000;

const String arrow = " <-";

const long rotaryButtonTimer_period = 1500;

const long rotaryLeftCooldown_period = 500;
const long rotaryRightCooldown_period = 500;
/**
   End of constants
  ************************************************
*/


/**
   End of object instances
  ************************************************
*/


/**
  ************************************************
   Enumerators
*/
/**
   There are different modes for charging the device.
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
   End of enumerators
  ************************************************
*/

/**
  ************************************************
   Variables for saving states
*/
// True if top is closed.
bool topClosed;

// True if the alarm is active.
bool alarm_active;

// Saves the last 50 battery voltages in Volts.
float battery_voltages [battery_array_size];

// Saves the average value of the battery voltage, based on the recent values.
float batteryVoltageAverage = 0;

// battery level in percent.
int batteryLevel = 0;

// charge mode, see declaration of enum chargeMode.
chargeMode currentChargeMode = CHARGE_NORMAL;

bool charging = false; // true if charging
bool chargerConnected = false; // true if charger is connected

// Charging current in mA
int chargeCurrent = 0;

signalSource source = SOURCE_BLUETOOTH;

//flag for when source was changed
bool sourceChanged = false;

bool onboardLedState = false;

/**
  Saves the position in the battery_voltages array.
  This is necessary to have an efficient way of buffering recent values without having to shift around the array.
*/
int battery_voltages_position = 0;

// Saves the state of the sound system. True if on.
bool soundsystem_state = false;

// Volume from 0 to 100
int volume = 0;

bool displayActive = false;

// Saves the state of the 12V converter. True if on.
bool converter12v = false;

// Encoder flags
//True if the encoder was recently turned
bool encoderTurnedLeft = false;
bool encoderTurnedRight = false;
bool encoderPressedShort = false;
bool encoderPressedLong = false;

/**
   End of variables for saving states
   ************************************************
*/



/**
  ************************************************
   Timers
*/
// When value is reached, refresh battery value
long battery_refresh_timer = 0;

// When value is reached, check for new RFID tag.
long rfid_check_timer = 0;

// When value is reached, turn off display
long display_on_timer = display_on_timer_period;

// When value is reached, display can be refreshed again
long display_refresh_timer = 0;

//When value is reached, go back to the dashboard
long menuScreensaver = menuScreensaver_period;

// When value is reached, counts as a long press
long rotaryButtonTimer = 0;

// When value is reached, display is no longer locked to displaying volume
long volumeDisplayTimer = 0;

/**
   End of timers
   ************************************************
*/



/**
  ************************************************
   Pin mappings
*/
// THE FOLLOWING PINS ARE DUMMY PINS
// buttons that are pressed down when the top is closed
const int top_closed_button = 5;

// battery voltage divider
const int vbat_pin = 6;

// Pin that is attached to the MOSFET that enables the unlock mechanism.
const int lockPin = 7;

// The RFID522 pins can be changed to any other pin!
//const int resetPin = 16; // Reset pin for RFID522 module
//const int ssPin = 17; // Slave select pin for RFID522 module

// Pin that is connected to the gate of the 12V converter
const int pin12v = 8;
// end of dummy pins


// rotary encoder pins
const int encoderPinA = 18;   // right
const int encoderPinB = 17;   // left
const int encoderButton = 19; // switch
// middle pin is connected to ground

const int onboardLED = 2;


/**
   I2C mapping: SDA -> Pin 21
                SCL -> Pin 22
*/
/**
   End of pin mappings
  ************************************************
*/


/**
  ************************************************
   Object instances
*/
// Set the LCD address to 0x27 for a 16 chars and 4 line display
LiquidCrystal_I2C lcd(0x27, 16, 4);
//MFRC522 mfrc522 = MFRC522(ssPin, resetPin); // Create instance

// Rotary encoder is wired with the common to ground and the two
// outputs to pins 18 and 17.
Rotary rotary = Rotary(encoderPinA, encoderPinB);


/**
   End of object instances
  ************************************************
*/

/**
  ************************************************
   Variables for lcd menu
   Top level menus:
   Lautstaerke
   Akku
   Signalquelle
   Beleuchtung
   Schliessfach oeffnen
   Alarm aktivieren
*/
bool menuActive = false; // Menu can be active or inactive
const int menuCount = 6;
const int subMenuCount = 4;
int menuPosition = 0; // position in the top menu
int subMenuPosition = -1; // position in the sub menu. If not in a sub menu, this is default to -1.
bool displayHasChanged = false; // flag for a change in the menu

/**
   End of variables for lcd menu
  ************************************************
*/

// encoder position
volatile int encoderCount = 0;
boolean button_set = false;
boolean buttonLongPressed = false;
long encoderButtonTimer = 0; //Timer for when the encoder button was pressed the last time
const long encoderButtonLongPressTime = 500;

void setup() {
  // rotary encoder pin config
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  pinMode(encoderButton, INPUT_PULLUP);
  pinMode(onboardLED, OUTPUT);
  attachInterrupt(encoderPinA, rotate, CHANGE);
  attachInterrupt(encoderPinB, rotate, CHANGE);

  Wire.begin(); // initialize I2C
  Serial.begin(115200); // serial output for debugging

  initializeBatteryValues(measureBatteryVoltage(battery_voltage_iterations)); //initialize the battery values
  updateBattery(); // call updateBattery() to initialize all values


  // RFID522
  //SPI.begin();
  //mfrc522.PCD_Init();

  // LCD

  lcd.begin(); // initialize the LCD

  //print boot screen
  lcd.setCursor(0, 0);
  lcd.print("Schallfrosch OS");
  lcd.setCursor(0, 1);
  lcd.print("v0.1");
  lcd.setCursor(0, 2);
  lcd.print("von Simon Welzel");
}


void loop() {
  //rotating = true;  // reset the debouncer
  checkTimers(); // Check for timers that are due
  handleEncoder();
  checkEncoderButton();
  updateDisplay();
}

// rotate is called anytime the rotary inputs change state.
void rotate() {
  unsigned char result = rotary.process();
  if (result == DIR_CCW) {
    encoderCount++;
  } else if (result == DIR_CW) {
    encoderCount--;
  }
}

void handleEncoder() {

  if (encoderCount > 0) { // if more right turns than left turns
    handleRight();
  }
  else if (encoderCount < 0) { // if more left turns than right turns
    handleLeft();
  }
  encoderCount = 0; // reset counters

}

// rotary encoder button handling
void checkEncoderButton() {
  if (digitalRead(encoderButton) == LOW) {

    if (button_set == false) {

      button_set = true;
      encoderButtonTimer = millis(); // start timer

    }

    if ((millis() - encoderButtonTimer > encoderButtonLongPressTime) && (buttonLongPressed == false)) {
      // button is hold for a long time
      buttonLongPressed = true;
      Serial.println("Long press!");
      handleLongPress();

    }

  } else { //if button is not pressed

    if (button_set == true) {

      if (buttonLongPressed == true) {

        buttonLongPressed = false;

      } else if (millis() - encoderButtonTimer > 50)  { //debounce
        // do short press routine stuff here
        Serial.println("Short press!");
        handleShortPress();
      }
      button_set = false;
    }
  }
}


/**
   Initialize battery level array.
*/
void initializeBatteryValues(float value) {
  for (int i = 0; i < battery_array_size; i++) {
    battery_voltages[i] = value;
  }
  battery_voltages_position = 0;
}

/**
   Measures the current battery voltage.
   Returns the current value.
   NOTE: DO NOT call this outside of updateBatteryVoltages() and setup()!
   This can slow down the controller.
*/
float measureBatteryVoltage(int count) {

  float result = 0;
  long r1 = 100000; //resistance of the first resistor of the voltage divider
  long r2 = 10000;  //resistance of the second resistor of the voltage divider
  float vref = 3.3;
  for (int i = 0; i < count; i++) {
    // x = 1023*(V_bat*(r2/(r1+r2)))/vref
    // -> V_bat = x*vref*(r1+r2/r2)/1023
    float vbat = analogRead(vbat_pin) * vref * (r1 + r2 / r2) / 1023;
    result += vbat;
    delay(2); // wait 2 ms to stabilize the result
  }

  return result;
}

/**
   Refreshes the battery voltages array. Measures a current value.
*/
void updateBatteryVoltages() {
  // If iteration counter is at the maximum, refresh it
  if (battery_voltages_position >= battery_array_size) {
    battery_voltages_position = 0;
  }

  battery_voltages[battery_voltages_position] = measureBatteryVoltage(battery_voltage_iterations);
  battery_voltages_position += 1;

  float result = 0;
  for (int i = 0; i < battery_array_size; i++) {
    result += battery_voltages[i];
  }
  batteryVoltageAverage = result / battery_array_size;

}

/**
   Calculates the battery level.
   Uses map function to map the value to a percent scale.
   Since the discharge function of lithium ion batteries is not proportional,
   a simple map will not result in good results. Thus, the map is done based
   on the region of the battery voltage.
   DO NOT call this, as it is already called in updateBattery().
*/
void updateBatteryLevel() {
  if (batteryVoltageAverage >= battery_map_100) {
    batteryLevel = 100;
  }
  else if (batteryVoltageAverage >= battery_map_80) {
    batteryLevel =  map(batteryVoltageAverage, battery_map_80, battery_map_100, 80, 100); // Map the voltage to battery level.
  }
  else if (batteryVoltageAverage >= battery_map_20) {
    batteryLevel =  map(batteryVoltageAverage, battery_map_20, battery_map_80, 20, 80); // Map the voltage to battery level.
  }
  else if (batteryVoltageAverage >= battery_map_0) {
    batteryLevel =  map(batteryVoltageAverage, battery_map_0, battery_map_20, 0, 20); // Map the voltage to battery level.
  }
  else if (batteryVoltageAverage < battery_map_0) {
    batteryLevel =  0;

  }
}

/**
   THIS IS ALL YOU NEED TO CALL to refresh the battery level.
   Measures a current value, averages the last values, updates batteryVoltageAverage, maps and updates batteryLevel.
   Call this function every once in a while to stay updated on the battery state!
   CAUTION: Due to delay(), this function takes AT LEAST (battery_voltage_iterations * 2) milliseconds!
*/
void updateBattery() {
  updateBatteryVoltages();
  updateBatteryLevel();
}

/**
   Opens the top.
   CAUTION: This function takes at least 500 ms!
   TODO: Use Timer1/ Timer2 for non-blocking functionality
*/
void openLocker() {
  // NEVER LEAVE THE UNLOCK MECHANISM ACTIVATED FOR MORE THAN 2 SECONDS to prevent hardware damage!
  bool converter12vOld = converter12v; //save state to later restore it

  switchConverter12v(true);
  digitalWrite(lockPin, HIGH);
  delay(500);
  digitalWrite(lockPin, LOW);
  switchConverter12v(converter12vOld);
}

void switchConverter12v(bool val) {
  if (val) {
    digitalWrite(pin12v, HIGH);
    converter12v = true;
  }
  else {
    digitalWrite(pin12v, LOW);
    converter12v = false;
  }
}

/**
   Checks the various timers and calls the assigned actions when the timer is due.
   A timer is due when the current system runtime (millis()) is greater than the timer value.
   When an action is executed, the timer is reset. Resetting means that it will be assigned
   a value of the current system time + the predefined period time.
*/
void checkTimers() {
  long currentMillis = millis();
  if (currentMillis > battery_refresh_timer) {
    updateBattery();
    battery_refresh_timer = battery_refresh_timer_period + currentMillis;
  }
  if (currentMillis > rfid_check_timer) {
    checkRFID();
    rfid_check_timer = rfid_check_timer_period + currentMillis;
  }
  if (currentMillis > display_on_timer && displayActive) {
    switchDisplay(false); // turn off display
  }

  if ((currentMillis > menuScreensaver) && menuActive) {
    //updateDashboard(); // activate screensaver
    deactivateMenu();
    setDisplayChanged();
  }

  if (currentMillis > volumeDisplayTimer && volumeDisplayTimer > 0) {
    setDisplayChanged();
    volumeDisplayTimer = 0;
  }
}

void checkRFID() {
  // check if new RFID token is in proximity
}

/**
   Switches the sound system based on value.

   Following components belong to the sound system:
   Small amplifier
   Big amplifier
   DSP
   Radio

*/
void switchSoundSystem(bool value) {
  if (value) {
    soundsystem_state = true;
    // Turn amplifier on, depending on which amplifier is active
    // Turn DSP on
    // Turn Radio on
  }
  else {
    soundsystem_state = false;
    // Turn amplifiers off
    // Turn DSP off
    // Turn Radio off
  }
}

/**
   Turn display on or off.
*/
void switchDisplay(bool value) {
  if (value && !displayActive) {
    displayActive = true;
    lcd.backlight();
    display_on_timer = millis() + display_on_timer_period;
    // Turn on display
  }
  else if (!value && displayActive) {
    displayActive = false;
    display_on_timer = 0;
    lcd.noBacklight();
    // turn off display
  }
}


/** Increase volume by val.
*/
void incVol(int val) {
  if (val > 0) {
    int oldVolume = volume;
    if ((volume + val) <= 100) {
      volume += val;
    }
  }
}


/** Decrease volume by val.
*/
void decVol(int val) {
  if (val > 0) {
    int oldVolume = volume;
    if ((volume - val) >= 0) {
      volume -= val;
    }
  }
}

void changeSource(int newSource) {
  // source = newSource;
  sourceChanged = true;
}

/**
   Prints the volume screen to the display.
*/
void displayVolume() {
  lcd.clear();
  lcd.setCursor(0, 0);
  String message = "Lautstaerke:";
  message.concat(volume);
  message.concat("%");
  lcd.print(message);
  // TODO: Create custom chars to show a nice bar from left to right!

}

/**
   Prints the battery information to the display.
   Battery information contains:
    Battery level (in percent and Volts)
    Charging current (if any)
    Remaining runtime/ remaining charging time

*/
void displayBattery() {
  lcd.clear();
  lcd.setCursor(0, 0); // set cursor to first row
  String message = "Bat:";
  message.concat(batteryLevel);
  message.concat("%,");
  message.concat(batteryVoltageAverage);
  message.concat("V");
  lcd.print(message);

  lcd.setCursor(0, 1); // set cursor to 2nd row
  if (charging ) {
    String message = "Ladestrom:";
    message.concat(chargeCurrent);
    message.concat("mA");
    lcd.print(message);
  }
  else if (!chargerConnected) {
    lcd.print("Kein Ladevorgang"); // no charger connected = not charging!
  }
  else if (chargerConnected && !charging) {
    lcd.print("Laden beendet!"); // charger connected, but not charging = fully charged!
  }

  lcd.setCursor(0, 2);
  if (charging) {
    lcd.print("Laedt noch ");
  }
  else {
    lcd.print("Verbleibend: ");
  }
  lcd.setCursor(0, 3);
  switch (currentChargeMode) {
    case CHARGE_NORMAL:
      lcd.print("Modus: Normal");
      break;
    case CHARGE_MAX:
      lcd.print("Modus: Maximal");
      break;
    case CHARGE_NONE:
      lcd.print("Modus: None");
  }

}

/**
   Display the available signal sources.
   Depending on the sub menu position, display a cursor next to the option
   Options are:
   Bluetooth
   AUX
   Radio
   Raspberry
*/
void displaySignalSource() {
  lcd.clear();
  String messages[] = {"Bluetooth", "AUX", "Radio", "Raspberry"};

  if (subMenuPosition >= 0 && subMenuPosition <= 3) {
    messages[subMenuPosition].concat(arrow);
  }
  else { //this should never happen
    lcd.print("Error: subMenuPosition is invalid!");
  }

  for (int i = 0; i < 4; i++) {
    lcd.setCursor(0, i);
    lcd.print(messages[i]);
  }
}

void displayLighting() {
  lcd.clear();
  String messages[] = {"Ein/Aus", "Sound2Light", "Statisch", "Flutlicht"};

  if (subMenuPosition >= 0 && subMenuPosition <= 3) {
    messages[subMenuPosition].concat(arrow);
  }
  else { //this should never happen
    lcd.print("Error: subMenuPosition is invalid!");
  }

  for (int i = 0; i < 4; i++) {
    lcd.setCursor(0, i);
    lcd.print(messages[i]);
  }
}

void displaySafe() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Authorisierung");
  lcd.setCursor(0, 1);
  lcd.print("erforderlich!");
  lcd.setCursor(0, 2);
  lcd.print("Rad druecken");
  lcd.setCursor(0, 3);
  lcd.print("u. authorisieren");
}

void displayAlarm() {
  lcd.clear();
  if (alarm_active) {
    lcd.setCursor(0, 0);
    lcd.print("Alarm ist aktiv!");
    lcd.setCursor(0, 1);
    lcd.print("Deaktivieren:");
    lcd.setCursor(0, 2);
    lcd.print("Rad druecken");
    lcd.setCursor(0, 3);
    lcd.print("u. authorisieren");
  }
  else {
    lcd.setCursor(0, 0);
    lcd.print("Authorisierung");
    lcd.setCursor(0, 1);
    lcd.print("erforderlich!");
    lcd.setCursor(0, 2);
    lcd.print("Rad druecken");
    lcd.setCursor(0, 3);
    lcd.print("u. authorisieren");
  }

}

void handleLeft() {
  wakeDisplay();
  if (!menuActive) { //case: Decrease volume
    decVol(1);
    volumeDisplayTimer = millis() + volumeDisplay_period;
  }
  else if (menuActive && subMenuPosition >= 0) {
    if (menuPosition == 0) { // volume submenu
      decVol(1);
    }
    else {
      subMenuPosition =  subMenuPosition - 1;
      if (subMenuPosition < 0) {
        subMenuPosition = subMenuCount - 1;;
      }
    }
  }
  else {
    menuPosition -= 1;
    if (menuPosition  < 0) {
      menuPosition = menuCount - 1;
    }
  }
  setDisplayChanged();

}

void handleRight() {
  wakeDisplay();
  if (!menuActive) { //case: Increase volume
    incVol(1);
    volumeDisplayTimer = millis() + volumeDisplay_period;
  }
  else if (menuActive && subMenuPosition >= 0) {
    if (menuPosition == 0) { // volume submenu
      incVol(1);
    }
    else {
      subMenuPosition += 1;
      if (subMenuPosition >= subMenuCount) {
        subMenuPosition = 0;
      }
    }
  }

  else {
    menuPosition += 1;
    if (menuPosition  >= menuCount) {
      menuPosition = 0;
    }
  }
  setDisplayChanged();

}

void toggleOnboardLED() {
  onboardLedState = !onboardLedState;
  digitalWrite(onboardLED, onboardLedState);
}

/**
  Top level menus:
   Lautstaerke
   Akku
   Signalquelle
   Beleuchtung
   Schliessfach oeffnen
   Alarm aktivieren
*/
void handleShortPress() {
  //toggleOnboardLED();
  setDisplayChanged();
  wakeDisplay();

  if (!menuActive) {
    menuActive = true;
  }
  else if (subMenuPosition == -1) { //main menu
    //TODO: remove switch case
    switch (menuPosition) {
      case 0:
        subMenuPosition = 0;
        break;
      case 1:
        subMenuPosition = 0;
        break;
      case 2:
        subMenuPosition = 0;
        break;
      case 3:
        subMenuPosition = 0;
        break;
      case 4:
        subMenuPosition = 0;
        // check RFID
        break;
      case 5:
        subMenuPosition = 0;
        // check RFID
        break;
    }
  }
  else if (subMenuPosition >= 0) { // sub Menu
    switch (menuPosition) {
      case 2: // audio source
        switch (subMenuPosition) {
          case 0:
            changeSource(0);
            break;
          case 1:
            changeSource (1);
            break;
          case 2:
            changeSource(2);
            break;
          case 3:
            changeSource(3);
            break;
          default:
            break;
        }

        break;
      case 3:
        //TODO lighting
        switch (subMenuPosition) {
          case 0:
            break;
          default:
            break;
        }
        break;
      default:
        break;

    }

  }
  // use Timers!
}

/**
  Long press goes back one menu hierarchy.
  Turns display on if it is turned off.
  Closes menu if in main menu.
*/
void handleLongPress() {
  // use Timers!
  setDisplayChanged();
  if (!displayActive) {
    wakeDisplay();
  }
  else if (!menuActive) {
    // switchDisplay(false); //turn off display
  }
  else if (subMenuPosition >= 0) {
    subMenuPosition = -1;

  }
  else {
    deactivateMenu();
  }
}


void wakeDisplay() {
  if (!displayActive) {
    switchDisplay(true);
  }
}


void setDisplayChanged() {
  menuScreensaver = millis() + menuScreensaver_period; //reset timer
  display_on_timer = millis() + display_on_timer_period;
  displayHasChanged = true;
}

void deactivateMenu() {
  menuActive = false;  // deactivate menu
  menuPosition = 0;  // reset menu positions
  subMenuPosition = -1;
}



/**
     Dashboard is the screen that is shown when not in a menu.
    Things to display:

*/
void updateDashboard() {
  if (displayHasChanged) {
    lcd.clear();
    // print battery level
    lcd.setCursor(0, 0); // set cursor to first row
    String messageBat = "Bat:";
    messageBat.concat(batteryLevel);
    messageBat.concat("%");
    lcd.print(messageBat);

    //print volume
    lcd.setCursor(0, 1);
    String messageVol = "Vol:";
    messageVol.concat(volume);
    messageVol.concat("%");
    lcd.print(messageVol);

    lcd.setCursor(0, 1);
    String messageSource = "Quelle:";

    switch (source) {
      case SOURCE_BLUETOOTH:
        messageSource.concat("BT");
        break;
      case SOURCE_AUX:
        messageSource.concat("AUX");
        break;
      case SOURCE_RADIO:
        messageSource.concat("Radio");
        break;
      case SOURCE_RASPBERRY:
        messageSource.concat("RPI");
        break;
    }
    lcd.print(messageSource);
  }

}


/**
  Updates the display.
  TODO: This seems to create crashes/ freezes, FIX!
*/
void updateDisplay() {

  if (displayHasChanged && (millis() > display_refresh_timer) && (millis() > 2000)) { // && (millis() > display_refresh_timer)
    display_refresh_timer = millis() + 500;
    lcd.clear(); // clear the display
    if (millis() < volumeDisplayTimer) { // if this is the case, always display volume
      displayVolume();
    }

    else if (menuActive) {
      if (subMenuPosition == -1) { //display main menu
        String messages[] = {"Lautstaerke", "Akku", "Signalquelle", "Beleuchtung", "Schliessfach", "Alarmanlage"};
        messages[menuPosition].concat(arrow);

        int firstEntry = 0;
        if (menuPosition >= 3) { //display last 4 entries
          firstEntry = 2;
        }
        for (int i = 0; i < 4; i++) {
          lcd.setCursor(0, i);
          lcd.print(messages[i + firstEntry]);
        }
      }
      else { //display sub menus
        switch (menuPosition) {

          case 0: // volume menu
            displayVolume();
            break;
          case 1: // battery menu
            displayBattery();
            break;
          case 2: // signal source menu
            displaySignalSource();
            break;
          case 3: // lighting menu
            displayLighting();
            break;
          case 4: // safe menu
            displaySafe();
            break;
          case 5: // alarm menu
            displayAlarm();
            break;
        }
      }
    }
    else if (!menuActive) { //if menu is not active
      updateDashboard();
    }
    displayHasChanged = false; // reset flag


  }
}
