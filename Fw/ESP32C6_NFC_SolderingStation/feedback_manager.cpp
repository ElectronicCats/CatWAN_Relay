#include "feedback_manager.h"

FeedbackManager::FeedbackManager() : ledGreenPin(LED_GREEN) {
}

void FeedbackManager::begin() {
    pinMode(ledGreenPin, OUTPUT);
    digitalWrite(ledGreenPin, LOW);
    
    #if DEBUG_SERIAL
    Serial.println("Feedback OK");
    #endif
}

void FeedbackManager::showSuccess() {
    setGreenLED(true);
}

void FeedbackManager::showError() {
    // Sin LED rojo ni buzzer, solo apagar LED verde
    setGreenLED(false);
}

void FeedbackManager::showWaiting() {
    setGreenLED(false);
}

void FeedbackManager::setGreenLED(bool state) {
    digitalWrite(ledGreenPin, state ? HIGH : LOW);
}
