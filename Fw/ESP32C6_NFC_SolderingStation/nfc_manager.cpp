#include "nfc_manager.h"

NFCManager::NFCManager() : initialized(false), lastScanTime(0), lastUID(""), lastUIDTime(0) {
    // Para I2C, usar constructor con IRQ y RESET (no SDA/SCL)
    // Si IRQ y RESET no están conectados, usar -1
    nfc = new Adafruit_PN532(PN532_IRQ, PN532_RESET);
}

NFCManager::~NFCManager() {
    if (nfc != nullptr) {
        delete nfc;
    }
}

bool NFCManager::begin() {
    // Inicializar I2C con los pines específicos para ESP32-C6
    // Si no se especifican pines, usar los pines por defecto del ESP32-C6
    #if PN532_SDA != -1 && PN532_SCL != -1
    Wire.begin(PN532_SDA, PN532_SCL);
    #else
    Wire.begin(); // Usar pines por defecto
    #endif
    
    // Inicializar PN532 con I2C
    // El begin() de Adafruit_PN532 detectará automáticamente I2C
    nfc->begin();
    
    // Intentar obtener versión del firmware
    uint32_t versiondata = nfc->getFirmwareVersion();
    if (!versiondata) {
        #if DEBUG_SERIAL
        Serial.println("PN532 no encontrado");
        #endif
        initialized = false;
        return false;
    }
    
    #if DEBUG_SERIAL
    Serial.print("PN532 v");
    Serial.print((versiondata >> 24) & 0xFF, DEC);
    Serial.print('.');
    Serial.println((versiondata >> 16) & 0xFF, DEC);
    #endif
    
    // Configurar SAM (Secure Access Module) para modo normal
    nfc->SAMConfig();
    
    initialized = true;
    #if DEBUG_SERIAL
    Serial.println("NFC OK");
    #endif
    return true;
}

bool NFCManager::isCardPresent() {
    if (!initialized || nfc == nullptr) return false;
    
    unsigned long currentTime = millis();
    if (currentTime - lastScanTime < NFC_SCAN_INTERVAL) {
        return false;
    }
    
    lastScanTime = currentTime;
    
    // Intentar leer tarjeta MIFARE ISO14443A
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
    uint8_t uidLength;
    
    bool success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100);
    
    return success;
}

String NFCManager::readCardUID() {
    if (!initialized || nfc == nullptr) return "";
    
    // Verificar debounce para evitar lectura repetida
    unsigned long currentTime = millis();
    if (!lastUID.isEmpty() && (currentTime - lastUIDTime) < NFC_DEBOUNCE_TIME) {
        return ""; // Aún en período de debounce
    }
    
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
    uint8_t uidLength;
    
    if (!nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100)) {
        // Si no se detecta tarjeta, resetear lastUID después de un tiempo
        if (!lastUID.isEmpty() && (currentTime - lastUIDTime) > NFC_DEBOUNCE_TIME * 2) {
            lastUID = "";
        }
        return "";
    }
    
    // Convertir UID a String hexadecimal
    String uidString = "";
    for (uint8_t i = 0; i < uidLength; i++) {
        if (uid[i] < 0x10) uidString += "0";
        uidString += String(uid[i], HEX);
    }
    uidString.toUpperCase();
    
    // Si es la misma tarjeta, no retornar
    if (uidString == lastUID) {
        return "";
    }
    
    lastUID = uidString;
    lastUIDTime = currentTime;
    
    #if DEBUG_SERIAL
    Serial.print("UID: ");
    Serial.println(uidString);
    #endif
    
    return uidString;
}

void NFCManager::reset() {
    lastUID = "";
    lastUIDTime = 0;
}

