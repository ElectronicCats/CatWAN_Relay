#include "sheets_manager.h"

SheetsManager::SheetsManager() : useGoogleScript(USE_GOOGLE_APPS_SCRIPT), useN8N(USE_N8N), connected(false), epochBase(0), millisAtSync(0), isTimeSynced(false) {
    scriptURL = String(GOOGLE_SCRIPT_URL);
    n8nURL = String(N8N_WEBHOOK_URL);
    n8nTimeURL = String(N8N_TIME_WEBHOOK_URL);
}

bool SheetsManager::begin() {
    connected = (WiFi.status() == WL_CONNECTED);
    #if DEBUG_SERIAL
    if (connected) {
        Serial.println("Sheets Manager OK");
    } else {
        Serial.println("Sheets Manager: WiFi desconectado");
    }
    #endif
    return connected;
}

String SheetsManager::getCurrentTimestamp() {
    if (isTimeSynced) {
        // Calcular tiempo actual real
        unsigned long currentSeconds = epochBase + ((millis() - millisAtSync) / 1000);
        
        // Formato HH:MM:SS
        // Nota: Esto es UTC a menos que se ajuste el offset en epochBase
        unsigned long seconds = currentSeconds % 60;
        unsigned long minutes = (currentSeconds / 60) % 60;
        unsigned long hours = (currentSeconds / 3600) % 24;
        
        char timestamp[30];
        snprintf(timestamp, sizeof(timestamp), "%02lu:%02lu:%02lu", hours, minutes, seconds);
        return String(timestamp);
    }

    // Fallback: tiempo relativo
    unsigned long seconds = millis() / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    
    unsigned long days = hours / 24;
    hours = hours % 24;
    minutes = minutes % 60;
    seconds = seconds % 60;
    
    char timestamp[30];
    snprintf(timestamp, sizeof(timestamp), "%02lu:%02lu:%02lu", hours, minutes, seconds);
    
    return String(timestamp);
}

String SheetsManager::createJSONPayload(EventData event) {
    StaticJsonDocument<JSON_BUFFER_SIZE> doc;

    doc["dev"] = event.deviceId;
    doc["stn"] = event.stationId;   // ← estación
    doc["uid"] = event.cardUID;
    doc["act"] = event.action;
    doc["st"]  = event.status;
    doc["rel"] = event.relayIndex;
    doc["dur"] = event.duration;
    doc["Nam"] = event.cardName;

    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}


bool SheetsManager::sendToGoogleScript(EventData event) {
    if (scriptURL.length() == 0) {
        #if DEBUG_SERIAL
        Serial.println("Error: Google Script URL no configurada");
        #endif
        return false;
    }
    
    HTTPClient http;
    http.begin(scriptURL);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(5000); // Timeout de 5 segundos
    
    String jsonPayload = createJSONPayload(event);
    
    #if DEBUG_SERIAL
    Serial.print("Enviando: ");
    Serial.println(jsonPayload);
    #endif
    
    int httpResponseCode = http.POST(jsonPayload);
    
    bool success = false;
    if (httpResponseCode > 0) {
        #if DEBUG_SERIAL
        Serial.print("HTTP: ");
        Serial.println(httpResponseCode);
        #endif
        success = (httpResponseCode == 200);
        http.getString(); // Limpiar buffer
    } else {
        #if DEBUG_SERIAL
        Serial.print("HTTP Error: ");
        Serial.println(httpResponseCode);
        #endif
    }
    
    http.end();
    return success;
}

bool SheetsManager::sendToN8N(EventData event) {
    if (n8nURL.length() == 0) {
        #if DEBUG_SERIAL
        Serial.println("Error: n8n Webhook URL no configurada");
        #endif
        return false;
    }
    
    HTTPClient http;
    http.begin(n8nURL);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(5000);
    
    String jsonPayload = createJSONPayload(event);
    
    #if DEBUG_SERIAL
    Serial.print("Enviando: ");
    Serial.println(jsonPayload);
    #endif
    
    int httpResponseCode = http.POST(jsonPayload);
    
    bool success = false;
    if (httpResponseCode > 0) {
        #if DEBUG_SERIAL
        Serial.print("HTTP: ");
        Serial.println(httpResponseCode);
        #endif
        success = (httpResponseCode == 200 || httpResponseCode == 201);
        http.getString();
    } else {
        #if DEBUG_SERIAL
        Serial.print("HTTP Error: ");
        Serial.println(httpResponseCode);
        #endif
    }
    
    http.end();
    return success;
}

bool SheetsManager::logEvent(EventData event) {
    if (WiFi.status() != WL_CONNECTED) {
        #if DEBUG_SERIAL
        Serial.println("SheetsManager: WiFi no conectado");
        #endif
        return false;
    }
    
    if (event.timestamp.length() == 0) {
        event.timestamp = getCurrentTimestamp();
    }
    
    bool success = false;
    
    if (useGoogleScript) {
        success = sendToGoogleScript(event);
    } else if (useN8N) {
        success = sendToN8N(event);
    } else {
        #if DEBUG_SERIAL
        Serial.println("Error: No hay método de logging configurado");
        #endif
        return false;
    }

    connected = success;
    
    return success;
}

void SheetsManager::setScriptURL(String url) {
    scriptURL = url;
    useGoogleScript = true;
    useN8N = false;
    #if DEBUG_SERIAL
    Serial.println("Google Script URL configurada");
    #endif
}

void SheetsManager::setN8NURL(String url) {
    n8nURL = url;
    useN8N = true;
    useGoogleScript = false;
    #if DEBUG_SERIAL
    Serial.println("n8n Webhook URL configurada");
    #endif
}

void SheetsManager::setN8NTimeURL(String url) {
    n8nTimeURL = url;
}

bool SheetsManager::syncTime() {
    if (WiFi.status() != WL_CONNECTED) return false;
    if (n8nTimeURL.length() == 0) return false;

    HTTPClient http;
    http.begin(n8nTimeURL);
    // GET request para obtener la hora
    int httpResponseCode = http.GET();
    
    bool success = false;
    if (httpResponseCode > 0) {
        String response = http.getString();
        #if DEBUG_SERIAL
        Serial.println("Time Sync Response: " + response);
        #endif
        
        // Parsear JSON: {"epoch": 123456789} o {"timestamp": 123456789}
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, response);

        if (!error) {
            unsigned long epoch = 0;
            if (doc.containsKey("epoch")) {
                epoch = doc["epoch"];
            } else if (doc.containsKey("timestamp")) {
                epoch = doc["timestamp"];
            }

            if (epoch > 0) {
                epochBase = epoch;
                millisAtSync = millis();
                isTimeSynced = true;
                success = true;
                #if DEBUG_SERIAL
                Serial.print("Tiempo sincronizado (Epoch): ");
                Serial.println(epoch);
                #endif
            }
        }
    }
    http.end();
    return success;
}