# 🛠️ Resumen de Trabajo y Corrección de Errores (WiFi / I2C)

## 1. 🚨 Diagnóstico de Errores (WiFi y I2C)

El sistema presentaba inestabilidad general y fallos en la detección del lector NFC PN532 cuando se activaban las funciones de red.

### A. Error del Bus I2C / NFC
**Síntoma:** El lector NFC no era detectado o dejaba de funcionar aleatoriamente.
**Causa Raíz:**
1.  **Constructor Incorrecto:** La instanciación de la librería `Adafruit_PN532` no usaba el constructor adecuado para el bus I2C por hardware, lo que forzaba comportamientos inestables o por software (bit-banging) no deseados.
2.  **Conflictos de Inicialización:** El orden de inicialización en `setup()` permitía que otros periféricos interfirieran antes de que el bus I2C estuviera listo.

**Solución:**
*   Se corrigió el constructor para pasar el puntero `&Wire` explícitamente.
*   Se aseguró una inicialización limpia con `Wire.begin()` antes de iniciar el subsistema NFC.

### B. Inestabilidad WiFi y Bloqueos
**Síntoma:** El ESP32 se reiniciaba o quedaba en bucles infinitos de "Desconectado" que bloqueaban el resto del código (incluyendo NFC).
**Causa Raíz:**
*   El manejo de la conexión WiFi y el portal cautivo (`WiFiManager`) era **bloqueante**. Si la conexión fallaba, el código quedaba atrapado intentando reconectar indefinidamente, impidiendo que el ciclo `loop()` principal se ejecutara.

**Solución:**
*   Se implementó un manejo **no bloqueante** en el `loop()`.
*   Se añadieron timeouts para evitar quedarse "colgado" esperando respuesta del router.
*   Se optimizó la lógica de reconexión para que ocurra en segundo plano sin detener la lectura de tarjetas NFC.

---

## 2. ✨ Nuevas Funcionalidades Implementadas

Además de las correcciones, se integraron mejoras operativas:

### 🕒 Sincronización de Hora (n8n)
*   Se añadió la capacidad de obtener la hora real desde un webhook de **n8n**.
*   Esto permite que los registros (logs) en Google Sheets tengan marcas de tiempo precisas incluso si el ESP32 se reinicia.
*   **Implementación:** Función `sheetsManager.syncTime()` que consulta el endpoint configurado.

### 🔌 Seguridad en Relés (Estado Inicial)
*   Se garantizó que todos los relés (cautines) inicien en estado **OFF (Apagado)** al arrancar el sistema.
*   Esto previene accidentes en caso de cortes de energía y reinicios automáticos; los cautines no se calentarán hasta que una tarjeta autorizada lo solicite.

### ⚙️ Configuración Dinámica de URLs
*   Se agregaron campos personalizados en el portal WiFi (`WiFiManager`) para ingresar las URLs de los Webhooks de n8n sin necesidad de reprogramar el código.
    *   `n8n URL logs`: Para registro de eventos.
    *   `n8n URL hora`: Para sincronización de fecha/hora.
