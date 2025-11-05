#ifndef FEEDBACK_MANAGER_H
#define FEEDBACK_MANAGER_H

#include <Arduino.h>
#include "config.h"

class FeedbackManager {
private:
    int ledGreenPin;
    int ledRedPin;
    int buzzerPin;
    unsigned long buzzerEndTime;
    bool buzzerActive;
    
public:
    FeedbackManager();
    void begin();
    void showSuccess();
    void showError();
    void showWaiting();
    void setGreenLED(bool state);
    void setRedLED(bool state);
    void beep(int duration = 200);
    void beepError();
    void beepSuccess();
    void update(); // Para manejar timers del buzzer
};

#endif // FEEDBACK_MANAGER_H


