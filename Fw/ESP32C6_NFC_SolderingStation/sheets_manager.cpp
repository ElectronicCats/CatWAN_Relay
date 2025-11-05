#include "sheets_manager.h"

SheetsManager::SheetsManager() : useGoogleScript(USE_GOOGLE_APPS_SCRIPT), useN8N(USE_N8N), connected(false) {
    scriptURL = String(GOOGLE_SCRIPT_URL);
    n8nURL = String(N8N_WEBHOOK_URL);
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
    // Obtener timestamp desde servidor NTP o usar millis()
    // Por simplicidad, usaremos una aproximación
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
    
    doc["ts"] = event.timestamp;
    doc["uid"] = event.cardUID;
    doc["usr"] = event.userName;
    doc["act"] = event.action;
    doc["st"] = event.status;
    doc["rel"] = event.relayIndex;
    doc["dur"] = event.duration;
    
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
    if (!connected || WiFi.status() != WL_CONNECTED) {
        #if DEBUG_SERIAL
        Serial.println("WiFi desconectado");
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

