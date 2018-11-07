/**
   Schallfrosch OS by Simon Welzel on 2018/11/06.

*/


#include <SPI.h>
#include <MFRC522.h> //https://www.arduinolibraries.info/libraries/mfrc522
#include <Wire.h>
#include <LiquidCrystal_I2C.h> //https://github.com/marcoschwartz/LiquidCrystal_I2C/
#include <Rotary.h> //http://www.buxtronix.net/2011/10/rotary-encoders-done-properly.html
#include "LCDMenu.h"

// version of this project
const String schallfroschOS_version = "v0.2";

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

// Value for how long the display stays on.
const long display_on_timer_period = 30000;

// Value for how frequent the battery value should be refreshed. 10000 = refresh every 10 seconds.
const long battery_refresh_timer_period = 10000;

// Value for how frequent the RFID reader checks for new tags in proximity. 2000 = check every 2 seconds.
const long rfid_check_timer_period = 2000;






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

Rotary rotary = Rotary(encoderPinA, encoderPinB);


LCDMenu lcdMenu = LCDMenu();


/**
   End of object instances
  ************************************************
*/



void setup() {

  pinMode(onboardLED, OUTPUT);

  Wire.begin(); // initialize I2C
  Serial.begin(115200); // serial output for debugging
  lcd.begin(); // initialize the LCD
  lcdMenu.init(&lcd, &rotary, schallfroschOS_version);


  // rotary encoder setup
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  pinMode(encoderButton, INPUT_PULLUP);
  attachInterrupt(encoderPinA, encoderRotate, CHANGE);
  attachInterrupt(encoderPinB, encoderRotate, CHANGE);

  initializeBatteryValues(measureBatteryVoltage(battery_voltage_iterations)); //initialize the battery values
  updateBattery(); // call updateBattery() to initialize all values


  // RFID522
  //SPI.begin();
  //mfrc522.PCD_Init();

  // LCD




}


void loop() {
  checkTimers(); // Check for timers that are due
  lcdMenu.loop();
  toggleOnboardLED();
}

// handler for rotary encoder. TODO: Make nicer
void encoderRotate() {
  lcdMenu.rotate();
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

long onboardLedTimer = 0;
void toggleOnboardLED() {
  if (millis() > onboardLedTimer) {
    onboardLedState = !onboardLedState;
    digitalWrite(onboardLED, onboardLedState);
    onboardLedTimer = millis() + 1000;
  }

}

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

struct displayData getDisplayData() {
  struct displayData newData;
  newData.batteryLevel = batteryLevel;
  newData.batteryVoltage = batteryVoltageAverage;
  newData.volume = volume;
  newData.source = source;
  newData.alarm = alarm_active;
  newData.currentChargeMode = currentChargeMode;
  newData.charging = charging;
  newData.chargerConnected = chargerConnected;
  newData.chargeCurrent = chargeCurrent;

  return newData;
}

/**
   Callback functions for menu
*/

void cb_refresh_data() {
  lcdMenu.refreshData(getDisplayData());
}

void cb_dec_vol() {
  decVol(1);
}

void cb_inc_vol() {
  incVol(1);
}

void cb_enable_12v() {
  // enable 12V converter
}

void cb_disable_12v() {
  // disable 12V converter
}

void cb_set_source_bt() {
  source = SOURCE_BLUETOOTH;
}

void cb_set_source_aux() {
  source = SOURCE_AUX;
}

void cb_set_source_radio() {
  source = SOURCE_RADIO;
}

void cb_set_source_rpi() {
  source = SOURCE_RASPBERRY;
}

void cb_inc_radio_freq() {
  // increase radio frequency
}

void cb_dec_radio_freq() {
  // decrease radio frequency
}

void cb_set_led_s2l() {
  // set LED mode to sound2light
}

void cb_set_led_off() {
  // turn off LEDs
}

void cb_open_locker() {
  // open locker routine
}

void cb_alarm_activate() {
  // activate alarm
}

void registerMenuCallbacks() {
  lcdMenu.registerCallbacks(&cb_refresh_data, &cb_dec_vol, &cb_inc_vol);
}
