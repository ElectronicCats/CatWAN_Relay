#include "nfc_manager.h"

NFCManager::NFCManager()
: initialized(false),
  lastScanTime(0),
  lastUID(""),
  lastUIDTime(0)
{
    nfc = new Adafruit_PN532(-1, -1);
}

NFCManager::~NFCManager() {
    delete nfc;
}

bool NFCManager::begin() {
    Wire.begin();
    delay(100);

    nfc->begin();
    uint32_t versiondata = nfc->getFirmwareVersion();

    if (!versiondata) {
        initialized = false;
        return false;
    }

    nfc->SAMConfig();
    initialized = true;
    return true;
}

bool NFCManager::isCardPresent() {
    if (!initialized) return false;

    unsigned long now = millis();
    if (now - lastScanTime < NFC_SCAN_INTERVAL) return false;

    lastScanTime = now;

    uint8_t uid[7];
    uint8_t uidLength;

    return nfc->readPassiveTargetID(
        PN532_MIFARE_ISO14443A,
        uid,
        &uidLength,
        100
    );
}

String NFCManager::readCardUID() {
    if (!initialized) return "";

    uint8_t uid[7];
    uint8_t uidLength;

    if (!nfc->readPassiveTargetID(
        PN532_MIFARE_ISO14443A,
        uid,
        &uidLength,
        100
    )) {
        return "";
    }

    String uidStr;
    for (uint8_t i = 0; i < uidLength; i++) {
        if (uid[i] < 0x10) uidStr += "0";
        uidStr += String(uid[i], HEX);
    }
    uidStr.toUpperCase();

    if (uidStr == lastUID) return "";

    lastUID = uidStr;
    lastUIDTime = millis();

    return uidStr;
}

void NFCManager::reset() {
    lastUID = "";
    lastUIDTime = 0;
}
