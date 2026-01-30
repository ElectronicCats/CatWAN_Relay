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
#include <Wire.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <SPIFFS.h>
#include "config.h"
#include "nfc_manager.h"
#include "relay_manager.h"
#include "card_manager.h"
#include "sheets_manager.h"
#include "feedback_manager.h"
#include "oled_manager.h"

char n8n_url[150] = "";
char n8n_time_url[150] = "";

// Instancias globales
RelayManager relayManager;
CardManager cardManager;
SheetsManager sheetsManager;
FeedbackManager feedbackManager;
WiFiManager wifiManager;
OLEDManager oledManager;
EventData queuedEvent;
NFCManager nfcManager; 
// 255 = pin dummy (no usado físicamente)


// Variables de estado
String pendingCardName = "";
bool waitingForCardName = false;
bool systemReady = false;
int currentRelayIndex = 0; // Relé activo actualmente (0 = primer cautín)
bool waitingForCard = false; // Flag para modo de espera de tarjeta
unsigned long waitStartTime = 0; // Tiempo de inicio de espera
unsigned long lastWifiCheck = 0;
bool pendingEvent = false;
unsigned long n8nEpoch = 0;
unsigned long n8nMillis = 0;
bool timeSynced = false;
bool triedTimeSync = false;
bool nfcBusy = false;
String lastScannedUID = "";      // Stores the last card ID read
unsigned long lastCardTime = 0;  // Stores the timestamp of the last read


// Prototipos de funciones
void setupWiFi();
void processNFC();
void handleSerialCommands();
void toggleSolderingIron(int relayIndex, String cardUID);
void printHelp();
void printStatus();
void reconnectWiFi();
void handleWiFi();
void handleAddCard(const String& uidStr);
void handleAccessCard(const String& uidStr);


String getStationIdByRelay(int relayIndex) {
    if (relayIndex >= 0 && relayIndex < NUM_STATIONS) {
        return String(stations[relayIndex].stationId);
    }
    return "DESCONOCIDA";
}

void processCard(String uid) {
    Serial.print("Tarjeta detectada: ");
    Serial.println(uid);

    if (cardManager.isCardAuthorized(uid)) {
        int relay = cardManager.getCardRelay(uid);
        toggleSolderingIron(relay, uid);
    } else {
        feedbackManager.showError();
        oledManager.showMessage("Acceso denegado");
    }
}


void saveN8NURL() {
  File f = SPIFFS.open("/n8n.txt", "w");
  if (f) {
    f.println(n8n_url);
    f.println(n8n_time_url);
    f.close();
  }
}


void setup() {
    Serial.begin(115200);
    delay(1000);

    Wire.begin();
    delay(100);

    oledManager.begin();
    oledManager.showWelcome();

    feedbackManager.begin();
    delay(2000);
    feedbackManager.showWaiting();

   
    if (!SPIFFS.begin(true)) {
        
        Serial.println("SPIFFS ERROR");

    }

    loadN8NURL();      // <-- cargar URL guardada
    setupWiFi();       // <-- aquí se mostrará el campo para n8n
    saveN8NURL();      // <-- guardar la nueva si el usuario la cambió

    sheetsManager.setN8NURL(String(n8n_url));  // <-- usar URL configurada
    sheetsManager.setN8NTimeURL(String(n8n_time_url));

    // Sincronizar hora si estamos conectados
    if (WiFi.status() == WL_CONNECTED) {
        sheetsManager.syncTime();
    }


    WiFi.setSleep(false);


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


void loadN8NURL() {
  if (SPIFFS.exists("/n8n.txt")) {
    File f = SPIFFS.open("/n8n.txt", "r");
    if (f) {
      String line = f.readStringUntil('\n');
      line.trim();
      line.toCharArray(n8n_url, sizeof(n8n_url));
      
      if (f.available()) {
        String line2 = f.readStringUntil('\n');
        line2.trim();
        line2.toCharArray(n8n_time_url, sizeof(n8n_time_url));
      }

      f.close();
    }
  }
}



void loop() {
    handleWiFi();
    
    // Sync queued events to Sheets/n8n when online
    // Sync queued events to Sheets/n8n when online
    if (pendingEvent && WiFi.status() == WL_CONNECTED) {
        sheetsManager.logEvent(queuedEvent);
        pendingEvent = false; // Intentar solo una vez y liberar
    }

    relayManager.checkSafetyTimeout();
    
    if (Serial.available()) {
        handleSerialCommands();
    }
    
    // Core NFC Logic
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
                        if (cardManager.addCard(uid, pendingCardName)) {
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
    

    delay(10); // Stability delay
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



void handleWiFi() {

    static unsigned long lastAttempt = 0;

    static wl_status_t lastStatus = WL_IDLE_STATUS;

    wl_status_t status = WiFi.status();

    if (status != lastStatus) {
        lastStatus = status;

        if (status == WL_CONNECTED) {
            Serial.println("WiFi conectado");
            Serial.print("IP: ");
            Serial.println(WiFi.localIP());
            oledManager.showMessage("WiFi OK");
        } else {
            Serial.println("WiFi desconectado");
            oledManager.showMessage("WiFi OFF");
        }
    }

    if (status != WL_CONNECTED && millis() - lastAttempt > 10000){
        lastAttempt = millis();
        WiFi.reconnect();
    }    // Si se perdió conexión, ESP32 se reconecta solo
}


void setupWiFi() {
    
    WiFiManagerParameter custom_n8n_url(
      "n8nurl",
      "URL Webhook n8n (Logs)",
      n8n_url,
      150
    );

    WiFiManagerParameter custom_n8n_time_url(
      "n8ntimeurl",
      "URL Webhook n8n (Hora)",
      n8n_time_url,
      150
    );

    wifiManager.addParameter(&custom_n8n_url);
    wifiManager.addParameter(&custom_n8n_time_url);

    wifiManager.setConfigPortalTimeout(180);

    if (!wifiManager.autoConnect(WIFI_AP_NAME, WIFI_AP_PASS)) {
        Serial.println("WiFi FAIL - Restart");
        delay(3000);
        ESP.restart();
    }

    // Guardar lo que el usuario escribió
    strncpy(n8n_url, custom_n8n_url.getValue(), sizeof(n8n_url));
    strncpy(n8n_time_url, custom_n8n_time_url.getValue(), sizeof(n8n_time_url));

    Serial.print("n8n URL logs: ");
    Serial.println(n8n_url);
    Serial.print("n8n URL hora: ");
    Serial.println(n8n_time_url);

    WiFi.setAutoReconnect(true);
    WiFi.persistent(false);
    WiFi.setSleep(false);
}

void toggleSolderingIron(int relayIndex, String cardUID) {
    if (waitingForCard) return;

    bool currentState = relayManager.getRelayState(relayIndex);
    bool newState = !currentState;

    String action = newState ? "ENCENDER" : "APAGAR";
    unsigned long duration = newState ? 0 : relayManager.getRelayUptime(relayIndex);
    String stationId = getStationIdByRelay(relayIndex);
    String cardName = cardManager.getCardName(cardUID);

    if (cardName.length() == 0) {
    cardName = "SIN_NOMBRE";    
    }


    relayManager.setRelay(relayIndex, newState);

    EventData event;
    event.timestamp = "";
    event.deviceId = String(DEVICE_ID);
    event.stationId = stationId;     // ← estación real
    event.cardUID = cardUID;
    event.action = action;
    event.status = "EXITO";
    event.relayIndex = relayIndex;
    event.duration = duration;
    event.cardName = cardName;

    String jsonPayload =
    "{\"uid\":\"" + cardUID +
    "\", \"act\":\"" + action +
    "\", \"relay\":" + String(relayIndex) +
    "}";




    queuedEvent = event;
    pendingEvent = true;

    #if DEBUG_SERIAL
    Serial.print("Estación ");
    Serial.print(stationId);
    Serial.print(": ");
    Serial.println(newState ? "ON" : "OFF");
    #endif

    feedbackManager.showWaiting();
    delay(300);

    nfcBusy = false;
}


void handleSerialCommands() {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toUpperCase();
    
    if (command.length() == 0) return;

    oledManager.showCommand(command);
    
    #if DEBUG_SERIAL
    Serial.print("Cmd: ");
    Serial.println(command);
    #endif

    // ====== MODO CAPTURA DE NOMBRE ======
    if (waitingForCardName) {
        pendingCardName = command;
        pendingCardName.trim();

        if (pendingCardName.length() > 0) {
            Serial.print("Nombre recibido: ");
            Serial.println(pendingCardName);

            oledManager.showMessage("Acerque tarjeta NFC");

            waitingForCardName = false;
            waitingForCard = true;
            waitStartTime = 0;
        }
        return; // ← Added missing return here
    }
    
    // ====== COMANDOS NORMALES ======
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
        
    } else if (command == CMD_ADD_CARD) {
        Serial.println("Ingrese el nombre de la persona:");
        oledManager.showMessage("Ingrese nombre");

        pendingCardName = "";
        waitingForCardName = true;

        // Formato: ADD_CARD <UID> o ADD_CARD (sin UID para leer automáticamente)
        int spaceIndex = command.indexOf(' ');
        if (spaceIndex > 0) {
            // Método 1: UID proporcionado manualmente
            String uid = command.substring(spaceIndex + 1);
            uid.trim();
            if (cardManager.addCard(uid, "SIN_NOMBRE")) {
                Serial.println("Tarjeta agregada exitosamente");
            }
        } else {
            // Método 2: Esperar tarjeta NFC automáticamente
            Serial.println("Coloca una tarjeta NFC en el lector...");
            Serial.println("Tienes 60 segundos para colocar la tarjeta");
            waitingForCard = true;
            waitStartTime = 0; // Resetear tiempo

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
            } else {
                Serial.println("Formato: TEST_RELAY <index> <ON|OFF>");
            }
        } else {
            Serial.println("Formato: TEST_RELAY <index> <ON|OFF>");
        }

    } else if (command == CMD_WIFI_RECONNECT) {
        reconnectWiFi();

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
    Serial.println("WIFI_ RECONNECT   -Forzar reconexión WiFi");
    Serial.println("===========================\n");
}

void reconnectWiFi() {
    Serial.println("Reiniciando WiFi...");
    oledManager.showMessage("Reset WiFi");

    WiFi.disconnect(true);
    delay(200);
    WiFi.begin();   // usa credenciales guardadas
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