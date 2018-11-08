#ifndef SF_BACKEND_h
#define SF_BACKEND_h

#include "Arduino.h"

class SchallfroschBackend {
  public:
    SchallfroschBackend();
    void init();
    void loop();
    struct displayData getDisplayData();
    void cb_dec_vol();
    void cb_inc_vol();
    void cb_enable_12v();
    void cb_disable_12v();
    void cb_set_source_bt();
    void cb_set_source_aux();
    void cb_set_source_radio();
    void cb_set_source_rpi();
    void cb_inc_radio_freq();
    void cb_dec_radio_freq();
    void cb_set_led_s2l();
    void cb_set_led_off();
    void cb_open_locker();
    void cb_alarm_activate();


  private:
    void initializeBatteryValues(float value);
    float measureBatteryVoltage(int count);
    void updateBatteryVoltages();
    void updateBatteryLevel();
    void updateBattery();
    void openLocker();
    void switchConverter12v(bool val);
    void checkTimers();
    void checkRFID();
    void switchSoundSystem(bool value);
    void incVol(int val);
    void decVol(int val);
    void toggleOnboardLED();

};


#endif
