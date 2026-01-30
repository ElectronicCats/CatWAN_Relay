#ifndef OLED_MANAGER_H
#define OLED_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class OLEDManager {
private:
    Adafruit_SSD1306 display;

public:
    OLEDManager();
    bool begin();

    void showMessage(String msg);
    void showCardAdded(String uid, String name);
    void clear();
    void showWelcome();
    void showWaiting();
    void showWaitingCard();
    void showTimeout();
    void showJSON(const String& json);
    void showCardAdded(const String& uid);
    void showCommand(String cmd);
    void showWelcomeName(String name);
};

#endif
