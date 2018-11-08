#ifndef SF_BACKEND_h
#define SF_BACKEND_h

#include "Arduino.h"

class SchallfroschBackend {
  public:
    SchallfroschBackend();
    void init();
    void loop();
    void openLocker();
    void incVol(int val);
    void decVol(int val);
    void enable12v();
    void disable12v();
    void setSourceBluetooth();
    void setSourceAux();
    void setSourceRadio();
    void setSourcePi();
    void incFreq();
    void decFreq();
    void setLedS2L();
    void turnOffLed();
    void activateAlarm();


  private:
    void initializeBatteryValues(float value);
    float measureBatteryVoltage(int count);
    void updateBatteryVoltages();
    void updateBatteryLevel();
    void updateBattery();
    void switchConverter12v(bool val);
    void checkTimers();
    void checkRFID();
    void switchSoundSystem(bool value);
    void toggleOnboardLED();

};


#endif
