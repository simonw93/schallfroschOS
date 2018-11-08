/**
 * Constants class implementation. This class defines constants for other classes to increase code readability.
 */
#include <Arduino.h>
#include "Constants.h"

 
// version of this project
const String Constants::SCHALLFROSCH_VERSION = "v0.2";
 
// Defines how many times a measurement is repeated in order to stabilize the result.
const int Constants::BATTERY_VOLTAGE_ITERATIONS = 20;

// Values for mapping the battery values.
const int Constants::BATTERY_MAP_100 = 29; // When the voltage is at this level, it is mapped to 100%.

const int Constants::BATTERY_MAP_80 = 27; // When the voltage is at this level, it is mapped to 80%.

const int Constants::BATTERY_MAP_20 = 24; // When the voltage is at this level, it is mapped to 20%.

const int Constants::BATTERY_MAP_0 = 20; // When the voltage is at this level, it is mapped to 0%.

// Value for how long the display stays on.
const long Constants::DISPLAY_ON_TIMER_RESET = 30000;

// Value for how frequent the battery value should be refreshed. 10000 = refresh every 10 seconds.
const long Constants::BATTERY_REFRESH_TIMER_RESET = 10000;

// Value for how frequent the RFID reader checks for new tags in proximity. 2000 = check every 2 seconds.
const long Constants::RFID_CHECK_TIMER_RESET = 2000;

// Value for how long the boot screen is displayed at startup
const long Constants::BOOT_SCREEN_DURATION = 3000;

/**
 * Menu constants
 */
 // Reset value for menu screensaver (time in millis after last action until screensaver is activated)
const long Constants::MENU_SCREENSAVER_RESET = 10000;

// Reset value for volume display (time in millis that volume is shown after it has changed)
const long Constants::DISPLAY_VOLUME_RESET = 2000;

// Amount of ms that have to pass since last refresh of the display until the display can be refreshed again
const long Constants::DISPLAY_REFRESH_RESET = 100;

/**
 * Pin mappings
 */
 // THE FOLLOWING PINS ARE DUMMY PINS


// battery voltage divider
const int Constants::VBAT_PIN = 6;

// Pin that is attached to the MOSFET that enables the unlock mechanism.
const int Constants::LOCK_PIN = 7;

// The RFID522 pins can be changed to any other pin!
//const int resetPin = 16; // Reset pin for RFID522 module
//const int ssPin = 17; // Slave select pin for RFID522 module

// Pin that is connected to the gate of the 12V converter
const int Constants::PIN_12V = 8;
// end of dummy pins

// rotary encoder pins
const int Constants::ENCODER_PIN_A = 18;   // right
const int Constants::ENCODER_PIN_B = 17;   // left
const int Constants::ENCODER_BUTTON = 19; // switch
// middle pin is connected to ground

const int Constants::ONBOARD_LED = 2;
