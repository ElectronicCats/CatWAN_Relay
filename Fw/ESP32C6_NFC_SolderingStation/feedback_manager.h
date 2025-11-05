#ifndef FEEDBACK_MANAGER_H
#define FEEDBACK_MANAGER_H

#include <Arduino.h>
#include "config.h"

class FeedbackManager {
private:
    int ledGreenPin;
    
public:
    FeedbackManager();
    void begin();
    void showSuccess();
    void showError();
    void showWaiting();
    void setGreenLED(bool state);
};

#endif // FEEDBACK_MANAGER_H


