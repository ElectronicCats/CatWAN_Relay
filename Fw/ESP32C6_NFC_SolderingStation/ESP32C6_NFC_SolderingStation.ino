/*
 * CatWAN Soldering Station - Control de Cautines con NFC
 * ESP32-C6 con PN532 para autenticación MIFARE S50
 * 
 * Funcionalidades:
 * - Control de cautines mediante autenticación NFC
 * - Registro de eventos en Google Sheets o n8n
 * - Gestión de tarjetas autorizadas (SPIFFS)
 * - Feedback visual y sonoro
 * - WiFi Manager para configuración de red
 */

#include <WiFi.h>
#include <WiFiManager.h>
#include <SPIFFS.h>
#include "config.h"
#include "nfc_manager.h"
#include "relay_manager.h"
#include "card_manager.h"
#include "sheets_manager.h"
#include "feedback_manager.h"

// Instancias globales
NFCManager nfcManager;
RelayManager relayManager;
CardManager cardManager;
SheetsManager sheetsManager;
FeedbackManager feedbackManager;
WiFiManager wifiManager;

// Variables de estado
bool systemReady = false;
String lastScannedUID = "";
unsigned long lastCardTime = 0;
int currentRelayIndex = 0; // Relé activo actualmente (0 = primer cautín)
bool waitingForCard = false; // Flag para modo de espera de tarjeta
unsigned long waitStartTime = 0; // Tiempo de inicio de espera

// Prototipos de funciones
void setupWiFi();
void handleSerialCommands();
void processNFC();
void toggleSolderingIron(int relayIndex, String cardUID);
void printHelp();
void printStatus();

void setup() {
    Serial.begin(SERIAL_BAUD);
    while(!Serial);
    
    #if DEBUG_SERIAL
    Serial.println("\nCatWAN Soldering Station\n");
    #endif
    
    feedbackManager.begin();
    feedbackManager.showWaiting();
    
    if (!SPIFFS.begin(true)) {
        #if DEBUG_SERIAL
        Serial.println("SPIFFS ERROR");
        #endif
        feedbackManager.showError();
        while(1) delay(1000);
    }
    
    setupWiFi();
    
    if (!nfcManager.begin()) {
        #if DEBUG_SERIAL
        Serial.println("NFC ERROR");
        #endif
        feedbackManager.showError();
        delay(2000);
    }
    
    relayManager.begin();
    
    if (!cardManager.begin()) {
        #if DEBUG_SERIAL
        Serial.println("Card Manager ERROR");
        #endif
    }
    
    sheetsManager.begin();
    
    #if USE_GOOGLE_APPS_SCRIPT
        if (String(GOOGLE_SCRIPT_URL).length() == 0) {
            #if DEBUG_SERIAL
            Serial.println("WARN: No URL config");
            #endif
        }
    #endif
    
    #if USE_N8N
        if (String(N8N_WEBHOOK_URL).length() == 0) {
            #if DEBUG_SERIAL
            Serial.println("WARN: No URL config");
            #endif
        }
    #endif
    
    systemReady = true;
    #if DEBUG_SERIAL
    Serial.println("Ready");
    #endif
    
    feedbackManager.showSuccess();
    delay(500);
    feedbackManager.showWaiting();
}

void loop() {
    // Verificar timeout de seguridad
    relayManager.checkSafetyTimeout();
    
    // Procesar comandos seriales
    if (Serial.available()) {
        handleSerialCommands();
    }
    
    // Procesar NFC
    if (systemReady && nfcManager.isInitialized()) {
        if (!waitingForCard) {
            processNFC();
        } else {
            // Modo de espera para agregar tarjeta
            if (waitStartTime == 0) {
                waitStartTime = millis();
            }
            
            // Verificar timeout (30 segundos)
            if (millis() - waitStartTime > 30000) {
                Serial.println("Timeout: No se detecto tarjeta");
                waitingForCard = false;
                waitStartTime = 0;
            } else {
                // Intentar leer tarjeta
                if (nfcManager.isCardPresent()) {
                    String uid = nfcManager.readCardUID();
                    if (uid.length() > 0) {
                        // Tarjeta detectada, agregarla
                        if (cardManager.addCard(uid)) {
                            Serial.println("Tarjeta agregada exitosamente");
                        } else {
                            Serial.println("Error al agregar tarjeta");
                        }
                        waitingForCard = false;
                        waitStartTime = 0;
                        nfcManager.reset(); // Resetear para evitar lectura repetida
                    }
                }
            }
        }
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        #if DEBUG_SERIAL
        Serial.println("WiFi reconnect");
        #endif
        WiFi.reconnect();
        delay(1000);
    }
    
    delay(100); // Pequeño delay para evitar saturación
}

void setupWiFi() {
    #if DEBUG_SERIAL
    Serial.println("WiFi...");
    #endif
    
    wifiManager.setConfigPortalTimeout(180);
    wifiManager.setAPStaticIPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
    
    if (!wifiManager.autoConnect(WIFI_AP_NAME, WIFI_AP_PASS)) {
        #if DEBUG_SERIAL
        Serial.println("WiFi FAIL - Restart");
        #endif
        delay(3000);
        ESP.restart();
    }
    
    #if DEBUG_SERIAL
    Serial.print("WiFi OK: ");
    Serial.println(WiFi.localIP());
    #endif
}

void processNFC() {
    // Detectar tarjeta
    if (nfcManager.isCardPresent()) {
        String uid = nfcManager.readCardUID();
        
        if (uid.length() > 0) {
            // Nueva tarjeta detectada
            lastScannedUID = uid;
            lastCardTime = millis();
            
            #if DEBUG_SERIAL
            Serial.print("Card: ");
            Serial.println(uid);
            #endif
            
            if (cardManager.isCardAuthorized(uid)) {
                #if DEBUG_SERIAL
                Serial.println("OK");
                #endif
                feedbackManager.showSuccess();
                
                // Obtener el relé asignado a esta tarjeta
                int assignedRelay = cardManager.getCardRelay(uid);
                
                // Si la tarjeta no tiene asignación, usar el relé por defecto (0)
                if (assignedRelay < 0) {
                    assignedRelay = currentRelayIndex; // Usar relé por defecto
                    #if DEBUG_SERIAL
                    Serial.println("Tarjeta sin asignación, usando relé por defecto");
                    #endif
                }
                
                // Toggle del cautín asignado (encender/apagar)
                toggleSolderingIron(assignedRelay, uid);
                
            } else {
                #if DEBUG_SERIAL
                Serial.println("DENIED");
                #endif
                feedbackManager.showError();
                
                // Registrar intento fallido
                EventData event;
                event.timestamp = "";
                event.deviceId = String(DEVICE_ID);
                event.cardUID = uid;
                event.action = "ACCESO DENEGADO";
                event.status = "FALLO";
                event.relayIndex = -1;
                event.duration = 0;
                
                sheetsManager.logEvent(event);
            }
            
            delay(1000); // Delay para evitar lectura múltiple
            feedbackManager.showWaiting();
        }
    } else {
        // No hay tarjeta presente - resetear después de un tiempo
        if (lastScannedUID.length() > 0 && (millis() - lastCardTime) > 3000) {
            nfcManager.reset();
            lastScannedUID = "";
        }
    }
}

void toggleSolderingIron(int relayIndex, String cardUID) {
    // No procesar si estamos en modo de espera de tarjeta
    if (waitingForCard) {
        return;
    }
    bool currentState = relayManager.getRelayState(relayIndex);
    bool newState = !currentState;
    
    String action = newState ? "ENCENDER" : "APAGAR";
    unsigned long duration = newState ? 0 : relayManager.getRelayUptime(relayIndex);
    
    // Cambiar estado del relé
    relayManager.setRelay(relayIndex, newState);
    
    // Preparar evento para logging
    EventData event;
    event.timestamp = "";
    event.deviceId = String(DEVICE_ID);  // Identificador único del dispositivo
    event.cardUID = cardUID;
    event.action = action;
    event.status = "EXITO";
    event.relayIndex = relayIndex;
    event.duration = duration;
    
    // Registrar en Google Sheets / n8n
    sheetsManager.logEvent(event);
    
    #if DEBUG_SERIAL
    Serial.print("Cautín ");
    Serial.print(relayIndex + 1);
    Serial.print(": ");
    Serial.println(newState ? "ON" : "OFF");
    #endif
}

void handleSerialCommands() {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toUpperCase();
    
    if (command.length() == 0) return;
    
    #if DEBUG_SERIAL
    Serial.print("Cmd: ");
    Serial.println(command);
    #endif
    
    if (command == CMD_HELP) {
        printHelp();
        
    } else if (command == CMD_STATUS) {
        printStatus();
        
    } else if (command == CMD_LIST_CARDS) {
        cardManager.listCards();
        
    } else if (command == CMD_LIST_ASSIGN) {
        cardManager.listAssignments();
        
    } else if (command.startsWith(CMD_ASSIGN_CARD)) {
        // Formato: ASSIGN_CARD <UID> <relay_index>
        // Ejemplo: ASSIGN_CARD 04A5B6C7D8 0
        int space1 = command.indexOf(' ');
        if (space1 > 0) {
            int space2 = command.indexOf(' ', space1 + 1);
            if (space2 > 0) {
                String uid = command.substring(space1 + 1, space2);
                uid.trim();
                uid.toUpperCase();
                int relayIndex = command.substring(space2 + 1).toInt();
                
                if (cardManager.assignCardToRelay(uid, relayIndex)) {
                    Serial.println("Asignación exitosa");
                }
            } else {
                Serial.println("Formato: ASSIGN_CARD <UID> <relay_index>");
                Serial.println("Ejemplo: ASSIGN_CARD 04A5B6C7D8 0");
            }
        } else {
            Serial.println("Formato: ASSIGN_CARD <UID> <relay_index>");
            Serial.println("Índices de relé: 0, 1, 2 (Cautín 1, 2, 3)");
        }
        
    } else if (command.startsWith(CMD_ADD_CARD)) {
        // Formato: ADD_CARD <UID> o ADD_CARD (sin UID para leer automáticamente)
        int spaceIndex = command.indexOf(' ');
        if (spaceIndex > 0) {
            // Método 1: UID proporcionado manualmente
            String uid = command.substring(spaceIndex + 1);
            uid.trim();
            if (cardManager.addCard(uid)) {
                Serial.println("Tarjeta agregada exitosamente");
            }
        } else {
            // Método 2: Esperar tarjeta NFC automáticamente
            Serial.println("Coloca una tarjeta NFC en el lector...");
            Serial.println("Tienes 30 segundos para colocar la tarjeta");
            waitingForCard = true;
            waitStartTime = 0; // Resetear tiempo
            nfcManager.reset(); // Resetear estado NFC
        }
        
    } else if (command.startsWith(CMD_REMOVE_CARD)) {
        // Formato: REMOVE_CARD <UID>
        int spaceIndex = command.indexOf(' ');
        if (spaceIndex > 0) {
            String uid = command.substring(spaceIndex + 1);
            uid.trim();
            if (cardManager.removeCard(uid)) {
                Serial.println("Tarjeta removida exitosamente");
            }
        } else {
            Serial.println("Formato: REMOVE_CARD <UID>");
        }
        
    } else if (command == CMD_CLEAR_CARDS) {
        Serial.println("¿Eliminar todas las tarjetas? Escribe 'YES' para confirmar");
        delay(2000);
        if (Serial.available()) {
            String confirm = Serial.readStringUntil('\n');
            confirm.trim();
            confirm.toUpperCase();
            if (confirm == "YES") {
                cardManager.clearAllCards();
            } else {
                Serial.println("Operación cancelada");
            }
        }
        
    } else if (command.startsWith(CMD_TEST_RELAY)) {
        // Formato: TEST_RELAY <index> <state>
        // Ejemplo: TEST_RELAY 0 ON
        int space1 = command.indexOf(' ');
        if (space1 > 0) {
            int space2 = command.indexOf(' ', space1 + 1);
            if (space2 > 0) {
                int relayIndex = command.substring(space1 + 1, space2).toInt();
                String state = command.substring(space2 + 1);
                state.toUpperCase();
                
                bool relayState = (state == "ON" || state == "1");
                relayManager.setRelay(relayIndex, relayState);
                Serial.print("Relé ");
                Serial.print(relayIndex);
                Serial.print(" configurado a: ");
                Serial.println(relayState ? "ON" : "OFF");
            }
        } else {
            Serial.println("Formato: TEST_RELAY <index> <ON|OFF>");
        }
        
    } else {
        Serial.println("Comando no reconocido. Escribe 'HELP' para ver comandos disponibles");
    }
}


void printHelp() {
    Serial.println("\n=== Comandos Disponibles ===");
    Serial.println("HELP              - Mostrar esta ayuda");
    Serial.println("STATUS            - Estado del sistema");
    Serial.println("LIST_CARDS        - Listar tarjetas autorizadas");
    Serial.println("ADD_CARD          - Agregar tarjeta (coloca tarjeta NFC)");
    Serial.println("ADD_CARD <UID>    - Agregar tarjeta con UID manual");
    Serial.println("ASSIGN_CARD <UID> <relay> - Asignar tarjeta a cautín");
    Serial.println("  Ejemplo: ASSIGN_CARD 04A5B6C7D8 0");
    Serial.println("  Relés: 0=Cautín1, 1=Cautín2, 2=Cautín3");
    Serial.println("LIST_ASSIGN       - Listar asignaciones tarjeta-cautín");
    Serial.println("REMOVE_CARD <UID> - Remover tarjeta autorizada");
    Serial.println("CLEAR_CARDS       - Eliminar todas las tarjetas");
    Serial.println("TEST_RELAY <i> <ON|OFF> - Probar relé");
    Serial.println("===========================\n");
}

void printStatus() {
    Serial.println("\n=== Estado del Sistema ===");
    Serial.print("Device ID: ");
    Serial.println(DEVICE_ID);
    
    Serial.print("WiFi: ");
    Serial.println(WiFi.status() == WL_CONNECTED ? "Conectado" : "Desconectado");
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    }
    
    Serial.print("NFC: ");
    Serial.println(nfcManager.isInitialized() ? "OK" : "ERROR");
    
    Serial.print("Tarjetas autorizadas: ");
    Serial.println(cardManager.getCardCount());
    
    Serial.println("Estado de relés:");
    for (int i = 0; i < relayManager.getNumRelays(); i++) {
        Serial.print("  Relé ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(relayManager.getRelayState(i) ? "ON" : "OFF");
        if (relayManager.getRelayState(i)) {
            unsigned long uptime = relayManager.getRelayUptime(i);
            Serial.print(" (Tiempo: ");
            Serial.print(uptime / 1000);
            Serial.print("s)");
        }
        Serial.println();
    }
    
    Serial.print("Google Sheets: ");
    Serial.println(sheetsManager.isConnected() ? "Conectado" : "Desconectado");
    
    Serial.println("===========================\n");
}

