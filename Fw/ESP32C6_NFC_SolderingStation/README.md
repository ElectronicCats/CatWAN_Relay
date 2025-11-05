# CatWAN Soldering Station - Control de Cautines con NFC

## Descripción

Sistema de control de cautines mediante autenticación NFC MIFARE S50 para ESP32-C6. Permite controlar el encendido y apagado de cautines solo con tarjetas NFC autorizadas, registrando todos los eventos en Google Sheets o n8n. Soporta múltiples dispositivos con identificadores únicos.

## Características

- ✅ Autenticación NFC MIFARE S50
- ✅ Control de múltiples cautines (hasta 3 relés por dispositivo)
- ✅ Gestión de tarjetas autorizadas (almacenamiento local SPIFFS)
- ✅ Asignación manual de tarjetas a relés específicos
- ✅ Identificador único de dispositivo (Device ID)
- ✅ Registro de eventos en Google Sheets o n8n
- ✅ WiFi Manager con captive portal para configuración
- ✅ Feedback visual (LED verde)
- ✅ Sistema de seguridad con timeout automático (2 horas)
- ✅ Comandos seriales para gestión y debug

## Hardware Requerido

- **ESP32-C6** - Microcontrolador con WiFi
- **PN532** - Módulo NFC (I2C)
- **Relé/es** - Para controlar cautín/es (hasta 3 por dispositivo)
- **LED Verde** - Feedback de éxito
- **Fuente de alimentación** - 5V o 3.3V según requerimientos

## Conexiones Hardware

### PN532 (I2C)
- VCC → 3.3V
- GND → GND
- SDA → I2C Data (pines por defecto del ESP32-C6)
- SCL → I2C Clock (pines por defecto del ESP32-C6)

**Nota:** El PN532 se conecta por I2C sin necesidad de especificar pines SDA/SCL explícitos.

### Relés
- RELAY_1 → GPIO 8 (Cautín 1)
- RELAY_2 → GPIO 10 (Cautín 2)
- RELAY_3 → GPIO 1 (Cautín 3)

### Feedback
- LED_GREEN → GPIO 11 (Feedback de éxito)

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
   - **Device ID:** Cambia `DEVICE_ID` a `"ESTACION1"`, `"ESTACION2"`, `"ESTACION3"`, etc. según el dispositivo
   - Pines del hardware (si son diferentes)
   - URLs de Google Sheets o n8n
   - Configuraciones de seguridad

2. **Importante:** Para cada dispositivo físico, cambia el `DEVICE_ID` en `config.h`:
   ```cpp
   #define DEVICE_ID "ESTACION1"  // Para el primer dispositivo
   #define DEVICE_ID "ESTACION2"  // Para el segundo dispositivo
   #define DEVICE_ID "ESTACION3"  // Para el tercer dispositivo
   ```

3. Para Google Sheets:
   - Configura `GOOGLE_SCRIPT_URL` con la URL de tu Web App
   - Configura `USE_GOOGLE_APPS_SCRIPT` en `true`
   - Lee la sección "Configuración de Google Sheets" más abajo

4. Para n8n:
   - Configura `N8N_WEBHOOK_URL` con la URL de tu webhook
   - Configura `USE_N8N` en `true`

### 3. Compilación y Carga

1. Selecciona la placa: **ESP32-C6 Dev Module**
2. Selecciona el puerto COM correcto
3. Compila y carga el firmware
4. **Repite para cada dispositivo** cambiando el `DEVICE_ID` antes de compilar

## Uso

### Primera Configuración

1. Al encender el dispositivo por primera vez, se creará un punto de acceso WiFi:
   - **SSID:** `CatWAN-SolderingStation`
   - **Contraseña:** `soldering123`

2. Conecta tu dispositivo a este WiFi y serás redirigido al captive portal
3. Selecciona tu red WiFi y proporciona la contraseña
4. El dispositivo se conectará automáticamente

### Agregar Tarjetas Autorizadas

#### Método 1: Comando Serial con UID

1. Abre el Serial Monitor (115200 baud)
2. Ejecuta el comando:
   ```
   ADD_CARD <UID>
   ```
   Ejemplo: `ADD_CARD 04A5B6C7D8`

#### Método 2: Comando Serial Automático (Recomendado)

1. Abre el Serial Monitor (115200 baud)
2. Ejecuta el comando:
   ```
   ADD_CARD
   ```
3. Coloca la tarjeta NFC en el lector dentro de 30 segundos
4. El sistema detectará automáticamente el UID y agregará la tarjeta

### Asignar Tarjeta a un Relé Específico

Para asignar una tarjeta a un relé específico (Cautín 1, 2 o 3):

```
ASSIGN_CARD <UID> <relayIndex>
```

Ejemplos:
- `ASSIGN_CARD 04A5B6C7D8 0` - Asigna tarjeta al Cautín 1 (Relé 0)
- `ASSIGN_CARD 04A5B6C7D8 1` - Asigna tarjeta al Cautín 2 (Relé 1)
- `ASSIGN_CARD 04A5B6C7D8 2` - Asigna tarjeta al Cautín 3 (Relé 2)

**Nota:** Si una tarjeta no tiene asignación, se usará el relé por defecto (Relé 0).

### Operación Normal

1. Coloca una tarjeta NFC autorizada en el lector
2. El LED verde parpadeará indicando éxito
3. El cautín asignado se encenderá/apagará según su estado actual
4. El evento se registrará en Google Sheets o n8n con el Device ID del dispositivo

### Comandos Serial Disponibles

| Comando | Descripción |
|---------|-------------|
| `HELP` | Mostrar ayuda |
| `STATUS` | Estado del sistema (incluye Device ID) |
| `LIST_CARDS` | Listar tarjetas autorizadas |
| `ADD_CARD` o `ADD_CARD <UID>` | Agregar tarjeta autorizada |
| `REMOVE_CARD <UID>` | Remover tarjeta autorizada |
| `CLEAR_CARDS` | Eliminar todas las tarjetas |
| `ASSIGN_CARD <UID> <relayIndex>` | Asignar tarjeta a un relé específico |
| `LIST_ASSIGN` | Listar todas las asignaciones tarjeta-relé |
| `TEST_RELAY <i> <ON\|OFF>` | Probar relé |

Ejemplos:
```
ADD_CARD
ADD_CARD 04A5B6C7D8
REMOVE_CARD 04A5B6C7D8
ASSIGN_CARD 04A5B6C7D8 1
LIST_ASSIGN
TEST_RELAY 0 ON
```

## Configuración de Google Sheets

### Opción 1: Google Apps Script (Recomendado)

1. Crea una nueva hoja de Google Sheets
2. Crea tres hojas:
   - **"Dispositivos"** - Tabla de mapeo Device ID → Nombre Dispositivo
   - **"Usuarios"** - Tabla de mapeo UID → Nombre Usuario
   - **"Eventos"** - Registro de todos los eventos

3. En la hoja **"Dispositivos"**, crea dos columnas:
   | Device ID | Nombre Dispositivo |
   |-----------|-------------------|
   | ESTACION1 | Estación 1        |
   | ESTACION2 | Estación 2        |
   | ESTACION3 | Estación 3        |

4. En la hoja **"Usuarios"**, crea dos columnas:
   | UID Tarjeta | Nombre Usuario |
   |-------------|----------------|
   | 04A5B6C7D8  | Juan Pérez     |
   | 1A2B3C4D5E  | María García   |

5. Ve a **Extensiones** → **Apps Script**
6. Crea un nuevo script con el siguiente código:

```javascript
function doPost(e) {
  var spreadsheet = SpreadsheetApp.getActiveSpreadsheet();
  var devicesSheet = spreadsheet.getSheetByName('Dispositivos');
  var usersSheet = spreadsheet.getSheetByName('Usuarios');
  var eventsSheet = spreadsheet.getSheetByName('Eventos');
  
  // Si las hojas no existen, usar la hoja activa
  if (!eventsSheet) {
    eventsSheet = spreadsheet.getActiveSheet();
  }
  
  var data = JSON.parse(e.postData.contents);
  
  // Buscar nombre del dispositivo
  var deviceName = data.dev || '';
  if (devicesSheet) {
    var deviceRange = devicesSheet.getDataRange();
    var deviceValues = deviceRange.getValues();
    for (var i = 1; i < deviceValues.length; i++) {
      if (deviceValues[i][0] == data.dev) {
        deviceName = deviceValues[i][1];
        break;
      }
    }
  }
  
  // Buscar nombre del usuario
  var userName = '';
  if (usersSheet) {
    var userRange = usersSheet.getDataRange();
    var userValues = userRange.getValues();
    for (var i = 1; i < userValues.length; i++) {
      if (userValues[i][0] == data.uid) {
        userName = userValues[i][1];
        break;
      }
    }
  }
  
  // Registrar evento
  var row = [
    new Date(),
    data.ts || '',
    data.dev || '',
    deviceName,
    data.uid || '',
    userName,
    data.act || '',
    data.st || '',
    data.rel || '',
    data.dur || ''
  ];
  
  eventsSheet.appendRow(row);
  
  return ContentService.createTextOutput(JSON.stringify({status: 'success'}))
    .setMimeType(ContentService.MimeType.JSON);
}
```

7. Guarda el script y despliégala como **Web App**:
   - Ejecutar como: Yo
   - Quién tiene acceso: Cualquiera
8. Copia la URL del Web App y configúrala en `config.h` como `GOOGLE_SCRIPT_URL`
9. Configura `USE_GOOGLE_APPS_SCRIPT` en `true`

### Opción 2: n8n

1. Crea un workflow en n8n
2. Agrega un nodo **Webhook** (HTTP Request)
3. Configura el método como POST
4. Agrega un nodo para buscar el nombre del usuario en base de datos o Google Sheets
5. Agrega un nodo **Google Sheets** para escribir datos en la hoja "Eventos"
6. Copia la URL del webhook y configúrala en `config.h` como `N8N_WEBHOOK_URL`
7. Configura `USE_N8N` en `true`

## Estructura de Datos

### JSON enviado por ESP32

El ESP32 envía el siguiente JSON a Google Sheets o n8n:

```json
{
  "ts": "02:30:45",           // Timestamp
  "dev": "ESTACION1",         // Device ID (identificador único del dispositivo)
  "uid": "04A5B6C7D8",       // UID de la tarjeta NFC
  "act": "ENCENDER",         // Acción: ENCENDER o APAGAR
  "st": "EXITO",             // Estado: EXITO o FALLO
  "rel": 0,                  // Índice del relé (0, 1, 2)
  "dur": 120000              // Duración de uso en ms (0 si enciende)
}
```

### Estructura de Datos en Google Sheets

#### Hoja "Eventos"

Los eventos se registran con las siguientes columnas:

| Columna | Descripción | Ejemplo |
|---------|-------------|---------|
| Fecha/Hora | Timestamp del servidor | 2024-01-15 14:30:45 |
| Timestamp | Timestamp del evento | 02:30:45 |
| Device ID | Identificador del dispositivo | ESTACION1 |
| Nombre Dispositivo | Nombre del dispositivo | Estación 1 |
| UID Tarjeta | UID de la tarjeta NFC | 04A5B6C7D8 |
| Nombre Usuario | Nombre del usuario | Juan Pérez |
| Acción | ENCENDER o APAGAR | ENCENDER |
| Estado | EXITO o FALLO | EXITO |
| Relé | Índice del relé (0, 1, 2) | 0 |
| Duración | Duración de uso en ms | 120000 |

**Nota:** El nombre del dispositivo y del usuario se buscan automáticamente en las hojas "Dispositivos" y "Usuarios" respectivamente.

## Flujo del Sistema

### 1. Evento en el Dispositivo

1. Usuario coloca tarjeta NFC en el lector
2. Sistema valida la tarjeta (autorizada o no)
3. Si está autorizada, activa/desactiva el relé asignado
4. Sistema envía evento a Google Sheets/n8n con:
   - Device ID del dispositivo
   - UID de la tarjeta
   - Acción realizada
   - Estado (éxito/fallo)
   - Índice del relé
   - Duración de uso

### 2. Procesamiento en Google Sheets/n8n

1. Recibe el evento con Device ID y UID
2. Busca el nombre del dispositivo en la hoja "Dispositivos" usando el Device ID
3. Busca el nombre del usuario en la hoja "Usuarios" usando el UID
4. Registra el evento completo en la hoja "Eventos" con todos los datos

### 3. Ventajas de este Flujo

- El dispositivo no almacena nombres de usuario (más simple y eficiente)
- Los nombres se pueden actualizar sin cambiar firmware
- Soporta múltiples dispositivos con identificadores únicos
- Centralización de datos en Google Sheets

## Seguridad

- **Timeout Automático:** Los cautines se apagan automáticamente después de 2 horas (configurable en `MAX_USAGE_TIME`)
- **Validación de Tarjetas:** Solo tarjetas autorizadas pueden activar los cautines
- **Registro de Eventos:** Todos los intentos (exitosos y fallidos) se registran
- **Debounce:** Protección contra lectura repetida accidental (2 segundos)

## Múltiples Dispositivos

Para usar múltiples dispositivos:

1. **Compila el firmware para cada dispositivo:**
   - Dispositivo 1: `#define DEVICE_ID "ESTACION1"`
   - Dispositivo 2: `#define DEVICE_ID "ESTACION2"`
   - Dispositivo 3: `#define DEVICE_ID "ESTACION3"`

2. **Carga el firmware en cada dispositivo**

3. **Configura Google Sheets:**
   - Agrega cada Device ID en la hoja "Dispositivos"
   - Asocia cada Device ID con un nombre descriptivo

4. **Todos los dispositivos pueden usar la misma configuración:**
   - Misma URL de Google Sheets/n8n
   - Mismas tarjetas autorizadas (cada dispositivo puede tener sus propias tarjetas)
   - Los eventos se diferencian por Device ID

## Troubleshooting

### NFC no detecta tarjetas
- Verifica las conexiones I2C del PN532
- Verifica que los pines estén correctamente configurados en `config.h`
- Asegúrate de usar tarjetas MIFARE S50
- Revisa los mensajes de debug en el Serial Monitor

### WiFi no conecta
- Verifica que la red WiFi esté disponible
- Usa el captive portal para reconfigurar (apaga y enciende el dispositivo)
- Revisa los logs seriales

### Google Sheets no recibe datos
- Verifica que la URL del Web App sea correcta
- Asegúrate de que el Web App esté desplegado
- Verifica que el WiFi esté conectado
- Revisa que `USE_GOOGLE_APPS_SCRIPT` esté en `true`
- Verifica los logs seriales para errores HTTP

### Relés no funcionan
- Verifica las conexiones de los relés
- Usa el comando `TEST_RELAY` para probar
- Verifica que los pines estén correctamente configurados en `config.h`
- Revisa el comando `STATUS` para ver el estado de los relés

### Device ID no aparece en STATUS
- Verifica que `DEVICE_ID` esté definido en `config.h`
- Recompila y carga el firmware

### Errores I2C esporádicos
- Los errores I2C ocasionales pueden aparecer después de un tiempo de ejecución
- No afectan el funcionamiento general del sistema
- Si persisten, verifica las conexiones I2C y la alimentación

## Licencia

Este firmware está bajo licencia GNU AGPL v3.0.

## Autor

Electronic Cats

## Soporte

Para más información, visita: https://electroniccats.com
