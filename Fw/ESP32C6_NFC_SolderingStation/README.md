# CatWAN Soldering Station - Control de Cautines con NFC

## Descripción

Sistema de control de cautines mediante autenticación NFC MIFARE S50 para ESP32-C6. Permite controlar el encendido y apagado de cautines solo con tarjetas NFC autorizadas, registrando todos los eventos en Google Sheets o n8n. Soporta múltiples dispositivos con identificadores únicos.

Incluye **feedback visual avanzado mediante pantalla OLED**, mostrando estados del sistema, conexión WiFi, eventos NFC y errores.

## Características

Este proyecto está diseñado como un sistema industrial ligero, robusto y escalable para el control de cautines mediante autenticación NFC. A continuación se describen las características de forma más detallada:

* ✅ **Autenticación NFC MIFARE S50**
  El sistema valida tarjetas NFC autorizadas usando su UID. Las tarjetas no autorizadas son rechazadas y el evento queda registrado.

* ✅ **Control de múltiples cautines (hasta 3 relés por dispositivo)**
  Cada tarjeta puede estar asociada a un relé específico o usar un relé por defecto. Esto permite que un solo dispositivo controle varias estaciones.

* ✅ **Gestión persistente de tarjetas autorizadas (SPIFFS)**
  Las tarjetas y asignaciones se almacenan en memoria flash, por lo que no se pierden al reiniciar el equipo.

* ✅ **Asignación manual de tarjetas a relés**
  Mediante comandos seriales es posible asignar una tarjeta a un cautín específico.

* ✅ **Identificador único de dispositivo (Device ID)**
  Cada ESP32 tiene un identificador único configurable que permite distinguir eventos cuando se usan múltiples estaciones.

* ✅ **Registro de eventos en Google Sheets o n8n**
  Todos los eventos (éxito, fallo, encendido, apagado, timeout) se envían vía HTTP en formato JSON.

* ✅ **WiFiManager con Captive Portal**
  Permite configurar WiFi y la URL de n8n sin recompilar el firmware.

* ✅ **Feedback visual mediante OLED y LED**
  La pantalla OLED muestra el estado del sistema (bienvenida, esperando tarjeta, tarjeta válida, errores, timeout).

* ✅ **Sistema de seguridad con timeout automático**
  Si un cautín permanece encendido más del tiempo configurado, se apaga automáticamente.

* ✅ **Comandos seriales para gestión y debug**
  Permite mantenimiento y diagnóstico sin necesidad de reprogramar el equipo.

* ✅ Autenticación NFC MIFARE S50

* ✅ Control de múltiples cautines (hasta 3 relés por dispositivo)

* ✅ Gestión de tarjetas autorizadas (almacenamiento local SPIFFS)

* ✅ Asignación manual de tarjetas a relés específicos

* ✅ Identificador único de dispositivo (Device ID)

* ✅ Registro de eventos en Google Sheets o n8n

* ✅ WiFi Manager con captive portal para configuración

* ✅ Feedback visual por **pantalla OLED (I2C)**

* ✅ Feedback visual por LED verde

* ✅ Sistema de seguridad con timeout automático (2 horas)

* ✅ Comandos seriales para gestión y debug

## Hardware Requerido

* **ESP32-C6** - Microcontrolador con WiFi
* **PN532** - Módulo NFC (I2C)
* **Pantalla OLED** - SSD1306 / SH1106 (I2C)
* **Relé/es** - Para controlar cautín/es (hasta 3 por dispositivo)
* **LED Verde** - Feedback de éxito
* **Fuente de alimentación** - 5V o 3.3V según requerimientos

## Conexiones Hardware

### PN532 (I2C)

* VCC → 3.3V
* GND → GND
* SDA → I2C Data (pines por defecto del ESP32-C6)
* SCL → I2C Clock (pines por defecto del ESP32-C6)

### Pantalla OLED (I2C)

* VCC → 3.3V
* GND → GND
* SDA → I2C Data
* SCL → I2C Clock

**Nota:** El PN532 y la OLED comparten el bus I2C.

### Relés

* RELAY_1 → GPIO 8 (Cautín 1)
* RELAY_2 → GPIO 10 (Cautín 2)
* RELAY_3 → GPIO 1 (Cautín 3)

### Feedback

* LED_GREEN → GPIO 11

**Nota:** Los pines son configurables en `config.h`

## Instalación

### 1. Librerías Requeridas

Instala las siguientes librerías desde el Arduino Library Manager:

* **Adafruit PN532** – Comunicación NFC
* **WiFiManager** – Gestión de WiFi
* **ArduinoJson** – Parsing de JSON
* **Adafruit SSD1306** – Control de pantalla OLED
* **Adafruit GFX Library** – Gráficos para OLED
* **SPIFFS** – Sistema de archivos (incluido en ESP32)

### 2. Archivos Nuevos Añadidos

El proyecto incluye un gestor dedicado para la pantalla OLED:

```
/oledManager.h
/oledManager.cpp
```

**Funciones principales de `oledManager`:**

* Inicialización de la pantalla
* Mensajes de estado (boot, WiFi, NFC)
* Visualización de UID
* Mensajes de éxito / error
* Indicadores de envío HTTP

### 3. Configuración

1. Abre `config.h` y configura:

   * `DEVICE_ID`
   * Pines de relés y LED
   * URL de Google Sheets o n8n
   * Parámetros de seguridad

2. Configura el tipo de backend:

```cpp
#define USE_GOOGLE_APPS_SCRIPT false
#define USE_N8N true
```

3. Configura la URL del webhook de n8n:

```cpp
#define N8N_WEBHOOK_URL "https://tu-n8n/webhook/catwan"
```

## Documentación de Funciones Principales

### setup()

Función de inicialización del sistema. Se ejecuta una sola vez al encender el dispositivo.

Responsabilidades:

* Inicializar comunicación serial
* Montar SPIFFS
* Configurar WiFi mediante WiFiManager
* Inicializar PN532
* Inicializar pantalla OLED
* Mostrar mensaje de bienvenida

---

### loop()

Función principal de ejecución continua.

Responsabilidades:

* Verificar conexión WiFi
* Revisar si el sistema está listo
* Mostrar estado "Esperando tarjeta" en OLED
* Detectar tarjetas NFC
* Procesar lógica de autorización
* Controlar relés
* Enviar eventos HTTP
* Manejar timeouts

---

### NFCManager::begin()

Inicializa el módulo PN532 y verifica que esté correctamente conectado por I2C.

### NFCManager::isCardPresent()

Devuelve `true` si una tarjeta NFC está presente en el lector.

### NFCManager::readCardUID()

Lee el UID de la tarjeta detectada y lo devuelve como `String`.

### NFCManager::reset()

Reinicia el estado del lector para evitar múltiples lecturas de la misma tarjeta.

---

### OLEDManager::showWelcome()

Muestra un mensaje de bienvenida al encender el dispositivo.

### OLEDManager::showWaiting()

Muestra el mensaje "Esperando tarjeta NFC" cuando el sistema está en reposo.

### OLEDManager::showCardAdded(uid)

Muestra el UID de la tarjeta detectada y confirma la acción realizada.

### OLEDManager::showTimeout()

Muestra un mensaje de error cuando se excede el tiempo de espera para detectar tarjeta.

---

### sendEventToN8n(json)

Envía un payload JSON vía HTTP POST a n8n o Google Sheets.
Incluye información del dispositivo, tarjeta, acción y duración.

---

## Uso

### Primera Configuración WiFi

1. El dispositivo crea un AP:

   * **SSID:** `CatWAN-SolderingStation`
   * **Password:** `soldering123`
2. Configura tu red WiFi desde el portal cautivo
3. El estado se muestra en la pantalla OLED

### Operación Normal

1. Presenta tarjeta NFC
2. OLED muestra UID y acción
3. Relé cambia de estado
4. Evento enviado a Google Sheets o n8n
5. OLED confirma envío exitoso

## Arquitectura del Software

El firmware está organizado de forma modular para facilitar mantenimiento, escalabilidad y depuración. Cada módulo tiene una responsabilidad clara.

### Módulos Principales

* **CATWAN_RELAY.ino**
  Archivo principal. Contiene `setup()` y `loop()`, coordina todos los managers y define el flujo general del sistema.

* **NFCManager**
  Encapsula toda la lógica relacionada con el PN532:

  * Inicialización del lector NFC
  * Detección de tarjetas
  * Lectura del UID
  * Reset del lector para evitar lecturas duplicadas

* **CardManager**
  Gestiona las tarjetas autorizadas:

  * Agregar / eliminar tarjetas
  * Asignar tarjetas a relés
  * Guardar y leer datos desde SPIFFS

* **RelayManager**
  Controla los relés físicos:

  * Encender / apagar relés
  * Llevar control del estado actual
  * Medir tiempo de encendido

* **WiFiManagerHelper**
  Extiende WiFiManager para:

  * Configurar WiFi
  * Configurar la URL de n8n desde el captive portal

* **OLEDManager (nuevo)**
  Maneja la pantalla OLED por I2C:

  * Mensaje de bienvenida
  * Estado "Esperando tarjeta"
  * Mostrar UID leído
  * Mostrar mensajes de error y timeout

* **FeedbackManager**
  Controla el LED verde para indicar éxito visual inmediato.

---

## Flujo General del Sistema

```text
[ Encendido ]
      |
      v
[ Inicialización ]
- Serial
- WiFi
- SPIFFS
- PN532
- OLED
      |
      v
[ Mostrar Bienvenida OLED ]
      |
      v
[ Esperando Tarjeta NFC ] <-------------------+
      |                                      |
      v                                      |
[ Tarjeta Detectada ]                         |
      |                                      |
      v                                      |
[ ¿Tarjeta Autorizada? ] -- NO --> [ Error ] -+
      |
     SI
      |
      v
[ Activar / Desactivar Relé ]
      |
      v
[ Enviar JSON a n8n / Sheets ]
      |
      v
[ Mostrar Resultado en OLED ]
      |
      v
[ Volver a Espera ]
```

---

## Estructura de Datos

### JSON enviado por ESP32

### JSON enviado a n8n

```json
{
  "ts": "02:30:45",
  "dev": "ESTACION1",
  "uid": "04A5B6C7D8",
  "act": "ENCENDER",
  "st": "EXITO",
  "rel": 0,
  "dur": 120000
}
```

### Uso del JSON en n8n

En n8n el JSON puede usarse para:

* Buscar usuario por UID
* Buscar estación por Device ID
* Registrar eventos
* Calcular tiempo de uso
* Activar alertas o dashboards

## Flujo del Sistema

1. Lectura NFC
2. Validación local
3. Acción sobre relé
4. Feedback OLED + LED
5. Envío HTTP POST
6. Registro centralizado

## Seguridad

* Timeout automático configurable
* Debounce de lecturas NFC
* Registro de intentos fallidos
* Sin almacenamiento de nombres en el ESP32

## Troubleshooting

### HTTP Error: -1

* No hay conexión WiFi
* URL inválida
* Error DNS
* Webhook n8n inactivo

### OLED no muestra información

* Verifica dirección I2C (0x3C / 0x3D)
* Verifica librerías Adafruit
* Confirma alimentación estable

---
## Licencia

Este firmware está bajo licencia GNU AGPL v3.0.

---

## 👨‍💻 Autor

ElectronicCats.

---
## Soporte

Para más información, visita: https://electroniccats.com

