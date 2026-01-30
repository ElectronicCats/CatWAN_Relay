#include "feedback_manager.h"

FeedbackManager::FeedbackManager() : ledGreenPin(LED_GREEN), ledOffTime(0) {}

void FeedbackManager::begin() {
    pinMode(ledGreenPin, OUTPUT);
    digitalWrite(ledGreenPin, LOW);
}

void FeedbackManager::showSuccess() {
    digitalWrite(ledGreenPin, HIGH);
    ledOffTime = millis() + 1000; // Se apagará en 1 segundo
}

void FeedbackManager::update() {
    // Si el tiempo actual superó el tiempo de apagado, apagamos
    if (ledOffTime > 0 && millis() > ledOffTime) {
        digitalWrite(ledGreenPin, LOW);
        ledOffTime = 0;
    }
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