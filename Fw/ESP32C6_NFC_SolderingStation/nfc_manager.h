#ifndef NFC_MANAGER_H
#define NFC_MANAGER_H

#include <Wire.h>
#include <Adafruit_PN532.h>
#include "config.h"

class NFCManager {
private:
    Adafruit_PN532* nfc;
    bool initialized;
    unsigned long lastScanTime;
    String lastUID;
    unsigned long lastUIDTime;
    
public:
    NFCManager();
    ~NFCManager();
    bool begin();
    bool isCardPresent();
    String readCardUID();
    bool isInitialized() { return initialized; }
    void reset();
};

#endif // NFC_MANAGER_H

