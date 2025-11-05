#ifndef SHEETS_MANAGER_H
#define SHEETS_MANAGER_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"

struct EventData {
    String timestamp;
    String deviceId;    // Identificador único del dispositivo
    String cardUID;
    String action;      // "ENCENDER" o "APAGAR"
    String status;      // "EXITO" o "FALLO"
    int relayIndex;
    unsigned long duration;  // Tiempo de uso en ms
};

class SheetsManager {
private:
    String scriptURL;
    String n8nURL;
    bool useGoogleScript;
    bool useN8N;
    bool connected;
    
    String getCurrentTimestamp();
    String createJSONPayload(EventData event);
    bool sendToGoogleScript(EventData event);
    bool sendToN8N(EventData event);
    
public:
    SheetsManager();
    bool begin();
    bool logEvent(EventData event);
    bool isConnected() { return connected; }
    void setScriptURL(String url);
    void setN8NURL(String url);
};

#endif // SHEETS_MANAGER_H


