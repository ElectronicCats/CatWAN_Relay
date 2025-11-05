#ifndef RELAY_MANAGER_H
#define RELAY_MANAGER_H

#include <Arduino.h>
#include "config.h"

class RelayManager {
private:
    int relayPins[MAX_RELAYS];
    bool relayStates[MAX_RELAYS];
    unsigned long relayStartTimes[MAX_RELAYS];
    int numRelays;
    
public:
    RelayManager();
    void begin();
    bool setRelay(int relayIndex, bool state);
    bool getRelayState(int relayIndex);
    unsigned long getRelayUptime(int relayIndex);
    void turnOffAll();
    int getNumRelays() { return numRelays; }
    bool isRelayOn(int relayIndex);
    void checkSafetyTimeout();
};

#endif // RELAY_MANAGER_H


