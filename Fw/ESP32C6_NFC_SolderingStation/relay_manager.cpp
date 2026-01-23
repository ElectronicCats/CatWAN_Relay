#include "relay_manager.h"

RelayManager::RelayManager() {
    numRelays = MAX_RELAYS;
    for (int i = 0; i < numRelays; i++) {
        relayPins[i] = stations[i].relayPin;  // ← ahora desde config.h
        relayStates[i] = false;
        relayStartTimes[i] = 0;
    }
}

void RelayManager::begin() {
    for (int i = 0; i < numRelays; i++) {
        pinMode(relayPins[i], OUTPUT);
        digitalWrite(relayPins[i], LOW);
        relayStates[i] = false;
    }
    #if DEBUG_SERIAL
    Serial.print("Relays (estaciones): ");
    Serial.println(numRelays);
    #endif
}


bool RelayManager::setRelay(int relayIndex, bool state) {
    if (relayIndex < 0 || relayIndex >= numRelays) {
        return false;
    }
    
    digitalWrite(relayPins[relayIndex], state ? HIGH : LOW);
    relayStates[relayIndex] = state;
    
    if (state) {
        relayStartTimes[relayIndex] = millis();
    } else {
        relayStartTimes[relayIndex] = 0;
    }
    
    #if DEBUG_SERIAL
    Serial.print("R");
    Serial.print(relayIndex + 1);
    Serial.println(state ? " ON" : " OFF");
    #endif
    
    return true;
}

bool RelayManager::getRelayState(int relayIndex) {
    if (relayIndex < 0 || relayIndex >= numRelays) {
        return false;
    }
    return relayStates[relayIndex];
}

unsigned long RelayManager::getRelayUptime(int relayIndex) {
    if (relayIndex < 0 || relayIndex >= numRelays) {
        return 0;
    }
    if (relayStates[relayIndex] && relayStartTimes[relayIndex] > 0) {
        return millis() - relayStartTimes[relayIndex];
    }
    return 0;
}

void RelayManager::turnOffAll() {
    for (int i = 0; i < numRelays; i++) {
        setRelay(i, false);
    }
    #if DEBUG_SERIAL
    Serial.println("All relays OFF");
    #endif
}

bool RelayManager::isRelayOn(int relayIndex) {
    return getRelayState(relayIndex);
}

void RelayManager::checkSafetyTimeout() {
    for (int i = 0; i < numRelays; i++) {
        if (relayStates[i] && relayStartTimes[i] > 0) {
            unsigned long uptime = getRelayUptime(i);
            if (uptime > MAX_USAGE_TIME) {
                #if DEBUG_SERIAL
                Serial.print("Timeout R");
                Serial.println(i + 1);
                #endif
                setRelay(i, false);
            }
        }
    }
}