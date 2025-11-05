# CatWAN Soldering Station - Control de Cautines con NFC

## Descripción

Sistema de control de cautines mediante autenticación NFC MIFARE S50 para ESP32-C6. Permite controlar el encendido y apagado de cautines solo con tarjetas NFC autorizadas, registrando todos los eventos en Google Sheets o n8n.

## Características

- ✅ Autenticación NFC MIFARE S50
- ✅ Control de múltiples cautines (hasta 3 relés)
- ✅ Gestión de tarjetas autorizadas (almacenamiento local SPIFFS)
- ✅ Registro de eventos en Google Sheets o n8n
- ✅ WiFi Manager con captive portal para configuración
- ✅ Feedback visual (LED verde/rojo) y sonoro (buzzer)
- ✅ Sistema de seguridad con timeout automático
- ✅ Comandos seriales para gestión y debug

## Hardware Requerido

- **ESP32-C6** - Microcontrolador con WiFi
- **PN532** - Módulo NFC (I2C)
- **Relé/es** - Para controlar cautín/es
- **LED Verde** - Feedback de éxito
- **LED Rojo** - Feedback de error
- **Buzzer** - Feedback sonoro
- **Fuente de alimentación** - 5V o 3.3V según requerimientos

## Conexiones Hardware

### PN532 (I2C)
- VCC → 3.3V
- GND → GND
- SDA → GPIO 8 (configurable en `config.h`)
- SCL → GPIO 9 (configurable en `config.h`)

### Relés
- RELAY_1 → GPIO 12 (Cautín 1)
- RELAY_2 → GPIO 13 (Cautín 2)
- RELAY_3 → GPIO 14 (Cautín 3)

### Feedback
- LED_GREEN → GPIO 15
- LED_RED → GPIO 16
- BUZZER → GPIO 17

**Nota:** Los pines son configurables en `config.h`

## Instalación

### 1. Librerías Requeridas

Instala las siguientes librerías desde el Arduino Library Manager:

- **Adafruit PN532** - Para comunicación NFC
- **WiFiManager** - Para gestión de WiFi
- **ArduinoJson** - Para parsing JSON
- **SPIFFS** - Sistema de archivos (incluida en ESP32)

### 2. Configuración

1. Abre `config.h` y configura:
   - Pines del hardware (si son diferentes)
   - URLs de Google Sheets o n8n
   - Configuraciones de seguridad

2. Para Google Sheets:
   - Configura `GOOGLE_SCRIPT_URL` con la URL de tu Web App
   - Lee la sección "Configuración de Google Sheets" más abajo

3. Para n8n:
   - Configura `N8N_WEBHOOK_URL` con la URL de tu webhook
   - Configura `USE_N8N` en `true`

### 3. Compilación y Carga

1. Selecciona la placa: **ESP32-C6 Dev Module**
2. Selecciona el puerto COM correcto
3. Compila y carga el firmware

## Uso

### Primera Configuración

1. Al encender el dispositivo por primera vez, se creará un punto de acceso WiFi:
   - **SSID:** `CatWAN-SolderingStation`
   - **Contraseña:** `soldering123`

2. Conecta tu dispositivo a este WiFi y serás redirigido al captive portal
3. Selecciona tu red WiFi y proporciona la contraseña
4. El dispositivo se conectará automáticamente

### Agregar Tarjetas Autorizadas

#### Método 1: Comando Serial

1. Abre el Serial Monitor (115200 baud)
2. Lee el UID de una tarjeta NFC colocándola en el lector
3. Ejecuta el comando:
   ```
   ADD_CARD <UID>
   ```
   Ejemplo: `ADD_CARD 04A5B6C7D8`

#### Método 2: Desde Código

Edita `config.h` y agrega tarjetas en el array de inicialización (si se implementa).

### Operación Normal

1. Coloca una tarjeta NFC autorizada en el lector
2. El LED verde parpadeará y el buzzer emitirá un beep corto
3. El cautín se encenderá/apagará según su estado actual
4. El evento se registrará en Google Sheets o n8n

### Comandos Serial Disponibles

| Comando | Descripción |
|---------|-------------|
| `HELP` | Mostrar ayuda |
| `STATUS` | Estado del sistema |
| `LIST_CARDS` | Listar tarjetas autorizadas |
| `ADD_CARD <UID>` | Agregar tarjeta autorizada |
| `REMOVE_CARD <UID>` | Remover tarjeta autorizada |
| `CLEAR_CARDS` | Eliminar todas las tarjetas |
| `TEST_RELAY <i> <ON\|OFF>` | Probar relé |

Ejemplos:
```
ADD_CARD 04A5B6C7D8
REMOVE_CARD 04A5B6C7D8
TEST_RELAY 0 ON
```

## Configuración de Google Sheets

### Opción 1: Google Apps Script (Recomendado)

1. Crea una nueva hoja de Google Sheets
2. Ve a **Extensiones** → **Apps Script**
3. Crea un nuevo script con el siguiente código:

```javascript
function doPost(e) {
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
  
  var data = JSON.parse(e.postData.contents);
  
  var row = [
    new Date(),
    data.timestamp || '',
    data.cardUID || '',
    data.userName || '',
    data.action || '',
    data.status || '',
    data.relayIndex || '',
    data.duration || ''
  ];
  
  sheet.appendRow(row);
  
  return ContentService.createTextOutput(JSON.stringify({status: 'success'}))
    .setMimeType(ContentService.MimeType.JSON);
}
```

4. Guarda el script y despliégala como **Web App**:
   - Ejecutar como: Yo
   - Quién tiene acceso: Cualquiera
5. Copia la URL del Web App y configúrala en `config.h` como `GOOGLE_SCRIPT_URL`

### Opción 2: n8n

1. Crea un workflow en n8n
2. Agrega un nodo **Webhook** (HTTP Request)
3. Configura el método como POST
4. Agrega un nodo **Google Sheets** para escribir datos
5. Copia la URL del webhook y configúrala en `config.h` como `N8N_WEBHOOK_URL`

## Estructura de Datos en Google Sheets

Los eventos se registran con las siguientes columnas:

| Columna | Descripción |
|---------|-------------|
| Timestamp | Fecha y hora del evento |
| Card UID | UID de la tarjeta NFC |
| User Name | Nombre del usuario (si está disponible) |
| Action | ENCENDER o APAGAR |
| Status | EXITO o FALLO |
| Relay Index | Índice del relé (0, 1, 2) |
| Duration | Duración de uso en ms |

## Seguridad

- **Timeout Automático:** Los cautines se apagan automáticamente después de 2 horas (configurable)
- **Validación de Tarjetas:** Solo tarjetas autorizadas pueden activar los cautines
- **Registro de Eventos:** Todos los intentos (exitosos y fallidos) se registran
- **Debounce:** Protección contra lectura repetida accidental

## Troubleshooting

### NFC no detecta tarjetas
- Verifica las conexiones I2C del PN532
- Verifica que los pines estén correctamente configurados en `config.h`
- Asegúrate de usar tarjetas MIFARE S50

### WiFi no conecta
- Verifica que la red WiFi esté disponible
- Usa el captive portal para reconfigurar
- Revisa los logs seriales

### Google Sheets no recibe datos
- Verifica que la URL del Web App sea correcta
- Asegúrate de que el Web App esté desplegado
- Verifica que el WiFi esté conectado

### Relés no funcionan
- Verifica las conexiones de los relés
- Usa el comando `TEST_RELAY` para probar
- Verifica que los pines estén correctamente configurados

## Licencia

Este firmware está bajo licencia GNU AGPL v3.0.

## Autor

Electronic Cats

## Soporte

Para más información, visita: https://electroniccats.com


