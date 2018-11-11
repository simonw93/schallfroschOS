#include "Arduino.h"
#include "SchallfroschBackend.h"
#include "Constants.h"
#include <TEA5767Radio.h>

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

static const int BATTERY_ARRAY_SIZE = 50;

/**
   Variables for saving states/ values.
*/

// true if observers are supposed to be notfied about changes
bool notify = false;

// True if the alarm is active.
bool alarm_active;

// Saves the last 50 battery voltages in Volts.
float battery_voltages [BATTERY_ARRAY_SIZE];

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

// frequency of the radio module
float freq = Constants::RADIO_FREQ_BEGIN;

// true if frequency has changed
bool freqChanged = false;


// Saves the state of the 12V converter. True if on.
bool converter12v = false;

/**
  ************************************************
   Timers
*/
// When value is reached, refresh battery value
long battery_refresh_timer = 0;

// When value is reached, check for new RFID tag.
long rfid_check_timer = 0;

// When value is reached, the onboard LED can be toggled again.
long onboardLedTimer = 0;



TEA5767Radio *myRadio;


SchallfroschBackend::SchallfroschBackend() {

}

void SchallfroschBackend::init(TEA5767Radio *pRadio) {
  initializeBatteryValues(measureBatteryVoltage(Constants::BATTERY_VOLTAGE_ITERATIONS)); //initialize the battery values
  updateBattery(); // call updateBattery() to initialize all values
  myRadio = pRadio;
  myRadio->setFrequency(Constants::RADIO_FREQ_BEGIN);
}

void SchallfroschBackend::loop() {
  if (freqChanged) {
    myRadio->setFrequency(freq); // refresh radio frequency
  }
  checkTimers(); // Check for timers that are due
  toggleOnboardLED(); // Toogle LED to indicate that the board is not stuck/ crashed
}

/**
   Return true if data has changed, then reset notify flag
*/
bool SchallfroschBackend::getNotify() {
  bool oldNotify = notify;
  notify = false;
  return oldNotify;
}

/**
   Initialize battery level array.
*/
void SchallfroschBackend::initializeBatteryValues(float value) {
  for (int i = 0; i < BATTERY_ARRAY_SIZE; i++) {
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
float SchallfroschBackend::measureBatteryVoltage(int count) {

  float result = 0;
  long r1 = 100000; //resistance of the first resistor of the voltage divider
  long r2 = 10000;  //resistance of the second resistor of the voltage divider
  float vref = 3.3;
  for (int i = 0; i < count; i++) {
    // x = 1023*(V_bat*(r2/(r1+r2)))/vref
    // -> V_bat = x*vref*(r1+r2/r2)/1023
    float vbat = analogRead(Constants::VBAT_PIN) * vref * (r1 + r2 / r2) / 1023;
    result += vbat;
    delay(2); // wait 2 ms to stabilize the result
  }

  return result;
}

/**
   Refreshes the battery voltages array. Measures a current value.
*/
void SchallfroschBackend::updateBatteryVoltages() {
  // If iteration counter is at the maximum, refresh it
  if (battery_voltages_position >= BATTERY_ARRAY_SIZE) {
    battery_voltages_position = 0;
  }

  battery_voltages[battery_voltages_position] = measureBatteryVoltage(Constants::BATTERY_VOLTAGE_ITERATIONS);
  battery_voltages_position += 1;

  float result = 0;
  for (int i = 0; i < BATTERY_ARRAY_SIZE; i++) {
    result += battery_voltages[i];
  }
  batteryVoltageAverage = result / BATTERY_ARRAY_SIZE;

}

/**
   Calculates the battery level.
   Uses map function to map the value to a percent scale.
   Since the discharge function of lithium ion batteries is not proportional,
   a simple map will not result in good results. Thus, the map is done based
   on the region of the battery voltage.
   DO NOT call this, as it is already called in updateBattery().
*/
void SchallfroschBackend::updateBatteryLevel() {
  if (batteryVoltageAverage >= Constants::BATTERY_MAP_100) {
    batteryLevel = 100;
  }
  else if (batteryVoltageAverage >= Constants::BATTERY_MAP_80) {
    batteryLevel =  map(batteryVoltageAverage, Constants::BATTERY_MAP_80, Constants::BATTERY_MAP_100, 80, 100); // Map the voltage to battery level.
  }
  else if (batteryVoltageAverage >= Constants::BATTERY_MAP_20) {
    batteryLevel =  map(batteryVoltageAverage, Constants::BATTERY_MAP_20, Constants::BATTERY_MAP_80, 20, 80); // Map the voltage to battery level.
  }
  else if (batteryVoltageAverage >= Constants::BATTERY_MAP_0) {
    batteryLevel =  map(batteryVoltageAverage, Constants::BATTERY_MAP_0, Constants::BATTERY_MAP_20, 0, 20); // Map the voltage to battery level.
  }
  else if (batteryVoltageAverage < Constants::BATTERY_MAP_0) {
    batteryLevel =  0;

  }
}

/**
   THIS IS ALL YOU NEED TO CALL to refresh the battery level.
   Measures a current value, averages the last values, updates batteryVoltageAverage, maps and updates batteryLevel.
   Call this function every once in a while to stay updated on the battery state!
   CAUTION: Due to delay(), this function takes AT LEAST (battery_voltage_iterations * 2) milliseconds!
*/
void SchallfroschBackend::updateBattery() {
  updateBatteryVoltages();
  updateBatteryLevel();
}

/**
   Opens the top.
   CAUTION: This function takes at least 500 ms!
   TODO: Use Timer1/ Timer2 for non-blocking functionality
*/
void SchallfroschBackend::openLocker() {
  // NEVER LEAVE THE UNLOCK MECHANISM ACTIVATED FOR MORE THAN 2 SECONDS to prevent hardware damage!
  bool converter12vOld = converter12v; //save state to later restore it

  switchConverter12v(true);
  digitalWrite(Constants::LOCK_PIN, HIGH);
  delay(500);
  digitalWrite(Constants::LOCK_PIN, LOW);
  switchConverter12v(converter12vOld);
}

void SchallfroschBackend::switchConverter12v(bool val) {
  if (val) {
    digitalWrite(Constants::PIN_12V, HIGH);
    converter12v = true;
  }
  else {
    digitalWrite(Constants::PIN_12V, LOW);
    converter12v = false;
  }
}

/**
   Checks the various timers and calls the assigned actions when the timer is due.
   A timer is due when the current system runtime (millis()) is greater than the timer value.
   When an action is executed, the timer is reset. Resetting means that it will be assigned
   a value of the current system time + the predefined period time.
*/
void SchallfroschBackend::checkTimers() {
  long currentMillis = millis();
  if (currentMillis > battery_refresh_timer) {
    updateBattery();
    battery_refresh_timer = Constants::BATTERY_REFRESH_TIMER_RESET + currentMillis;
  }
  if (currentMillis > rfid_check_timer) {
    checkRFID();
    rfid_check_timer = Constants::RFID_CHECK_TIMER_RESET + currentMillis;
  }
}

void SchallfroschBackend::checkRFID() {
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
void SchallfroschBackend::switchSoundSystem(bool value) {
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




/** Increase volume by val.
*/
void SchallfroschBackend::incVol(int val) {
  if (val > 0) {
    int oldVolume = volume;
    if ((volume + val) <= 100) {
      volume += val;
    }
  }
}


/** Decrease volume by val.
*/
void SchallfroschBackend::decVol(int val) {
  if (val > 0) {
    int oldVolume = volume;
    if ((volume - val) >= 0) {
      volume -= val;
    }
  }
}


void SchallfroschBackend::toggleOnboardLED() {
  if (millis() > onboardLedTimer) {
    onboardLedState = !onboardLedState;
    digitalWrite(Constants::ONBOARD_LED, onboardLedState);
    onboardLedTimer = millis() + 1000;
  }
}

/**
   Callback functions for menu
*/


bool SchallfroschBackend::isCharging() {
  return charging;
}

void SchallfroschBackend::enable12v() {
  // enable 12V converter
}

void SchallfroschBackend::disable12v() {
  // disable 12V converter
}

void SchallfroschBackend::setSourceBluetooth() {
  source = SOURCE_BLUETOOTH;
}

void SchallfroschBackend::setSourceAux() {
  source = SOURCE_AUX;
}

void SchallfroschBackend::setSourceRadio() {
  source = SOURCE_RADIO;
}

void SchallfroschBackend::setSourcePi() {
  source = SOURCE_RASPBERRY;
}

void SchallfroschBackend::incFreq() {
  freq += 0.1;
  if (freq > Constants::RADIO_FREQ_MAX) {
    freq = Constants::RADIO_FREQ_MIN;
  }
  freqChanged = true;
}

void SchallfroschBackend::decFreq() {
  freq -= 0.1;
  if (freq < Constants::RADIO_FREQ_MIN) {
    freq = Constants::RADIO_FREQ_MAX;
  }
  freqChanged = true;
}

void SchallfroschBackend::setLedS2L() {
  // set LED mode to sound2light
}

void SchallfroschBackend::turnOffLed() {
  // turn off LEDs
}

void SchallfroschBackend::activateAlarm() {
  // activate alarm
}
