#ifndef SHEETS_MANAGER_H
#define SHEETS_MANAGER_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"


struct EventData {
    String timestamp;
    String deviceId;
    String stationId;
    String cardUID;
    String action;
    String status;
    String cardName;
    int relayIndex;
    unsigned long duration;
};


class SheetsManager {
public:
    SheetsManager();
    bool begin();
    bool logEvent(EventData event);
    bool isConnected() { return connected; }
    void setScriptURL(String url);
    void setN8NURL(String url);
    void setN8NTimeURL(String url);
    bool syncTime();

private:
    String scriptURL;
    String n8nURL;
    String n8nTimeURL;
    bool useGoogleScript;
    bool useN8N;
    bool connected;
    
    // Time control
    unsigned long epochBase;
    unsigned long millisAtSync;
    bool isTimeSynced;
    
    String getCurrentTimestamp();
    String createJSONPayload(EventData event);
    bool sendToGoogleScript(EventData event);
    bool sendToN8N(EventData event);
};

#endif // SHEETS_MANAGER_H