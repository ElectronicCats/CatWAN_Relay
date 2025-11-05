#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// Configuración de Hardware - ESP32-C6
// ============================================

// Pines para PN532 (I2C)
//#define PN532_SDA     6   // GPIO 6 - I2C Data
//#define PN532_SCL     7   // GPIO 7 - I2C Clock

// Pines para Relés (múltiples cautines soportados)
#define RELAY_1      8  // GPIO 8 - Relé Cautín 1
#define RELAY_2      10  // GPIO 10 - Relé Cautín 2
#define RELAY_3      1  // GPIO 1 - Relé Cautín 3
#define MAX_RELAYS    3   // Número máximo de relés

// Pines para Feedback
#define LED_GREEN     11  // GPIO 11 - LED Verde (éxito)

// ============================================
// Configuración de WiFi
// ============================================
#define WIFI_AP_NAME   "CatWAN-SolderingStation"
#define WIFI_AP_PASS   "soldering123"

// ============================================
// Configuración de NFC
// ============================================
#define NFC_SCAN_INTERVAL   500   // ms entre escaneos
#define NFC_DEBOUNCE_TIME   2000  // ms para evitar lectura repetida

// ============================================
// Identificador del Dispositivo
// ============================================
// Identificador único del dispositivo (cambiar para cada dispositivo)
// Ejemplos: ESTACION1, ESTACION2, ESTACION3
#define DEVICE_ID               "ESTACION1"  // Cambiar para cada dispositivo

// ============================================
// Configuración de Google Sheets / n8n
// ============================================
// Opción 1: Google Apps Script Web App
#define USE_GOOGLE_APPS_SCRIPT  false
#define GOOGLE_SCRIPT_URL       ""  // URL del Web App (configurar)

// Opción 2: n8n
#define USE_N8N                  false
#define N8N_WEBHOOK_URL          ""  // URL del webhook n8n (configurar)

// ============================================
// Configuración de Seguridad
// ============================================
#define MAX_USAGE_TIME      7200000  // 2 horas en ms (timeout seguridad)
#define MAX_FAILED_ATTEMPTS 5        // Intentos fallidos antes de bloquear

// ============================================
// Configuración de Almacenamiento
// ============================================
#define USE_SPIFFS            true   // Usar SPIFFS para tarjetas
#define USE_REMOTE_VALIDATION false   // Validar desde Google Sheets
#define MAX_AUTHORIZED_CARDS  20      // Máximo de tarjetas autorizadas (reducido para ahorrar memoria)
#define CARD_UID_LENGTH       8       // Longitud UID MIFARE S50

// ============================================
// Optimización de Memoria
// ============================================
#define DEBUG_SERIAL          true    // Activar/desactivar mensajes Serial
#define JSON_BUFFER_SIZE      256     // Tamaño del buffer JSON (reducido de 512)

// ============================================
// Configuración Serial
// ============================================
#define SERIAL_BAUD           115200
#define SERIAL_TIMEOUT        1000

// ============================================
// Comandos Serial
// ============================================
#define CMD_ADD_CARD      "ADD_CARD"
#define CMD_REMOVE_CARD   "REMOVE_CARD"
#define CMD_LIST_CARDS    "LIST_CARDS"
#define CMD_ASSIGN_CARD   "ASSIGN_CARD"
#define CMD_LIST_ASSIGN   "LIST_ASSIGN"
#define CMD_TEST_RELAY    "TEST_RELAY"
#define CMD_STATUS        "STATUS"
#define CMD_CLEAR_CARDS   "CLEAR_CARDS"
#define CMD_HELP          "HELP"

#endif // CONFIG_H

