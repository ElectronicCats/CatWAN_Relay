#include "feedback_manager.h"

FeedbackManager::FeedbackManager() : ledGreenPin(LED_GREEN), ledRedPin(LED_RED), buzzerPin(BUZZER), buzzerEndTime(0), buzzerActive(false) {
}

void FeedbackManager::begin() {
    pinMode(ledGreenPin, OUTPUT);
    pinMode(ledRedPin, OUTPUT);
    pinMode(buzzerPin, OUTPUT);
    
    digitalWrite(ledGreenPin, LOW);
    digitalWrite(ledRedPin, LOW);
    digitalWrite(buzzerPin, LOW);
    
    #if DEBUG_SERIAL
    Serial.println("Feedback OK");
    #endif
}

void FeedbackManager::showSuccess() {
    setGreenLED(true);
    setRedLED(false);
    beepSuccess();
}

void FeedbackManager::showError() {
    setGreenLED(false);
    setRedLED(true);
    beepError();
}

void FeedbackManager::showWaiting() {
    setGreenLED(false);
    setRedLED(false);
}

void FeedbackManager::setGreenLED(bool state) {
    digitalWrite(ledGreenPin, state ? HIGH : LOW);
}

void FeedbackManager::setRedLED(bool state) {
    digitalWrite(ledRedPin, state ? HIGH : LOW);
}

void FeedbackManager::beep(int duration) {
    digitalWrite(buzzerPin, HIGH);
    buzzerEndTime = millis() + duration;
    buzzerActive = true;
}

void FeedbackManager::beepSuccess() {
    beep(100); // Beep corto para éxito
}

void FeedbackManager::beepError() {
    beep(500); // Beep largo para error
}

void FeedbackManager::update() {
    if (buzzerActive && millis() >= buzzerEndTime) {
        digitalWrite(buzzerPin, LOW);
        buzzerActive = false;
    }
}

