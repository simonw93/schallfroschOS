#ifndef CONSTANTS_SCHALLFROSCH_h
#define CONSTANTS_SCHALLFROSCH_h


class Constants {
  public:
    static const String SCHALLFROSCH_VERSION;
    static const int BATTERY_VOLTAGE_ITERATIONS;
    static const int BATTERY_MAP_100;
    static const int BATTERY_MAP_80;
    static const int BATTERY_MAP_20;
    static const int BATTERY_MAP_0;
    static const int VBAT_PIN;
    static const int LOCK_PIN;
    static const int PIN_12V;
    static const int ENCODER_PIN_A;
    static const int ENCODER_PIN_B;
    static const int ENCODER_BUTTON;
    static const int ONBOARD_LED;

    static const long MENU_SCREENSAVER_RESET;
    static const long DISPLAY_VOLUME_RESET;
    static const long DISPLAY_REFRESH_RESET;

    static const long DISPLAY_ON_TIMER_RESET;
    static const long BATTERY_REFRESH_TIMER_RESET;
    static const long RFID_CHECK_TIMER_RESET;
    static const long BOOT_SCREEN_DURATION;
    

  private:
};

#endif
