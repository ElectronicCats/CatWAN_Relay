# 📘 README – Corrección de errores NFC y sobresaturación por WiFiManager

## 1. Contexto del problema

Durante el desarrollo del proyecto **CATWAN_RELAY**, se integraron varios módulos críticos en un ESP32, principalmente:

* **PN532 (NFC) vía I2C**
* **WiFiManager** para configuración dinámica de red
* Comunicación con servicios externos (n8n)

Conforme el proyecto creció, comenzaron a presentarse errores constantes relacionados con NFC, identificados principalmente como **"NFC ERROR"**, pérdida total de detección de tarjetas y ausencia de mensajes por el monitor Serial.

---

## 2. Síntomas observados

1. El monitor Serial dejaba de mostrar información después de cargar nuevo código.
2. El PN532 no detectaba tarjetas NFC aunque el cableado fuera correcto.
3. Aparecían errores de compilación y linker relacionados con `NFCManager`.
4. WiFiManager se quedaba bloqueado o reiniciaba el sistema.
5. El sistema funcionaba parcialmente si se deshabilitaba WiFiManager o NFC, pero no ambos juntos.

---

## 3. Causas reales del problema

### 3.1 Sobresaturación del sistema por WiFiManager

WiFiManager crea:

* Portales cautivos
* Servidor web
* DNS interno

Esto **consume memoria, tiempo de CPU y bloquea el loop principal**, especialmente si:

* Se inicializa antes que otros periféricos
* No se limita su tiempo de ejecución

Esto provocaba que:

* El bus I2C no se inicializara correctamente
* El PN532 no respondiera a `getFirmwareVersion()`

---

### 3.2 Uso incorrecto del constructor de Adafruit_PN532

Se intentaron múltiples constructores inválidos:

* `Adafruit_PN532(Wire)` ❌
* `Adafruit_PN532(PN532_I2C_ADDRESS)` ❌

La librería **NO soporta esos constructores**.

El constructor correcto para I2C es:

```
Adafruit_PN532(uint8_t irq, uint8_t reset, TwoWire *theWire)
```

Al no usarlo correctamente:

* El objeto `nfc` se creaba mal
* Las llamadas internas fallaban silenciosamente

---

### 3.3 Inicialización incorrecta del bus I2C

El PN532 se conectó usando:

* **SDA = GPIO 6**
* **SCL = GPIO 7**

Problemas:

* GPIO 6 está reservado internamente (SPI Flash)
* GPIO 7 no es un pin I2C recomendado

Esto hacía que:

* `Wire.begin()` no funcionara correctamente
* El PN532 nunca respondiera

---

### 3.4 Inconsistencias entre `.h` y `.cpp`

Se detectaron errores graves de diseño:

* Métodos declarados en `.h` pero no definidos en `.cpp`
* Firmas distintas (`readCardUID()` vs `readCardUID(char*)`)
* Destructor declarado pero no implementado

Esto causaba:

* Errores de linker (`undefined reference`)
* Fallos en tiempo de ejecución

---

## 4. Solución aplicada

### 4.1 Reestructuración de NFCManager

Se unificaron completamente las firmas:

* `bool readCardUID(char* uidBuffer)`
* Eliminación de métodos no usados
* Estado interno claro (`initialized`)

---

### 4.2 Corrección del constructor PN532

Se adoptó el constructor correcto:

```
Adafruit_PN532 nfc(IRQ_PIN, RESET_PIN, &Wire);
```

Y se inicializó correctamente:

```
Wire.begin(SDA_PIN, SCL_PIN);
```

---

### 4.3 Corrección de pines I2C

Se cambiaron los pines a valores seguros:

* SDA = GPIO 21
* SCL = GPIO 22

Esto permitió:

* Comunicación estable
* Lectura correcta del firmware PN532

---

### 4.4 Control de ejecución de WiFiManager

Se evitó que WiFiManager:

* Bloqueara el `loop()`
* Se ejecutara permanentemente

Ahora:

* Se inicializa solo cuando es necesario
* No interfiere con NFC

---

## 5. Resultado final

✔ NFC detecta tarjetas de forma estable
✔ UID leído correctamente sin repeticiones
✔ WiFiManager funciona sin bloquear el sistema
✔ No hay errores de compilación ni linker
✔ Monitor Serial estable

---

## 6. Conclusión técnica

El problema **NO era el PN532**, sino:

* Mala inicialización de I2C
* Uso incorrecto de librerías
* Saturación del sistema por WiFiManager
* Errores estructurales en C++

Una correcta arquitectura y control de recursos permitió estabilizar todo el sistema.

---

📌 *Este README documenta los cambios realizados para futuras referencias, mantenimiento y evaluación técnica del proyecto.*
