#ifndef SHEETS_MANAGER_H
#define SHEETS_MANAGER_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"

struct EventData {
    String timestamp;
    String deviceId;
    String stationId;   // ← NUEVO
    String cardUID;
    String action;
    String status;
    int relayIndex;
    unsigned long duration;
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