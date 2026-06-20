# MIA - Sistema Integrado de Control ESP32/ESP8266/ESP-01

## Arquitectura de la Red

```
MAESTRO: ESP32 (Bluetooth)
    ↓ ESP-NOW
ESCLAVO: ESP8266 D1 Mini (4 relés + 2 sensores)
    ↓ ESP-NOW
    ├─ ESP-01 #1 (MAC: 8A:31:C0:57:80:10) → Intermitente Trasero Derecho
    └─ ESP-01 #2 (MAC: 8E:77:5D:E0:6E:DA) → Marcha Atrás
```

## Distribución de Relés

### ESP32 (8 relés locales)
- PIN 2: Mor��n
- PIN 4: Posición
- PIN 5: Corta
- PIN 18: Larga
- PIN 21: Pinchazo
- PIN 22: Cortesía
- PIN 23: Nieblas
- PIN 19: Limpiaparabrisas

### ESP8266 D1 Mini (4 relés + 2 sensores)
- D1 (GPIO5): Intermitente Izquierdo (LOCAL)
- D2 (GPIO4): Intermitente Derecho → ESP-01 #1
- D5 (GPIO14): Freno (LOCAL + SENSOR D3)
- D6 (GPIO12): Marcha Atrás → ESP-01 #2

**MAC del D1 Mini: 22:9C:4A:4A:FD:0D** ✓

### ESP-01 #1 (8A:31:C0:57:80:10)
- D1 (GPIO5): Relé Intermitente Trasero Derecho

### ESP-01 #2 (8E:77:5D:E0:6E:DA)
- D1 (GPIO5): Relé Marcha Atrás

## Instalación

### 1. Instalar Librerías en Arduino IDE

```
- FastLED
- BluetoothSerial (incluida en ESP32)
- espnow (incluida en librerías Arduino)
```

### 2. Cargar códigos en orden

1. **ESP-01 #1**: `ESP01_INTERMITENTE_TRASERO.ino`
2. **ESP-01 #2**: `ESP01_MARCHA_ATRAS.ino`
3. **D1 Mini**: `ESP8266_D1MINI_ESCLAVO.ino`
4. **ESP32**: `ESP32_MAESTRO.ino` (MAC ya configurada: 22:9C:4A:4A:FD:0D)

## Comandos Bluetooth (desde App/Serial)

### Brillo LED
- `BUP` → Aumentar brillo (+10)
- `BDO` → Disminuir brillo (-10)
- `B100` → Fijar brillo a 100 (0-255)

### Intermitentes
- `IZQ` → Intermitente Izquierdo
- `DER` → Intermitente Derecho
- `INT_OFF` → Apagar ambos

### Marcha Atrás
- `MAR` → Activar marcha atrás
- `MAR_OFF` → Desactivar

### Relés Locales (ESP32)
- `POS` → Posición
- `CRT` → Corta
- `LRG` → Larga
- `MUS` → Pinchazo
- `COR` → Cortesía
- `ANT` → Nieblas
- `LIM` → Limpiaparabrisas

### Modos Especiales
- `PMO` → Modo PMO
- `SER` → Sereno
- `HAG` → Hacer
- `FIN` → Finalizar (apaga todo)

## Diagrama de Flujo de Comunicación

```
App Bluetooth
    ↓
[ESP32 - Maestro]
    ↓ ESP-NOW
[D1 Mini - Esclavo]
    ├─ Relé INT_IZQ (LOCAL)
    ├─ Relé FRENO (LOCAL + SENSOR)
    ├─ Relé INT_DER → [ESP-01 #1] (ESP-NOW)
    └─ Relé MAR → [ESP-01 #2] (ESP-NOW)
```

## Notas Importantes

⚠️ **MAC D1 Mini**: Ya está configurada en ESP32_MAESTRO.ino (22:9C:4A:4A:FD:0D)

⚠️ **Power Supply**: Los ESP-01 pueden necesitar condensador 10µF en power para estabilidad

⚠️ **Debugging**: Abre Serial Monitor a 115200 baud para ver logs de cada dispositivo

⚠️ **Canales WiFi**: Todos los ESP usan canal 1 por defecto (compatible con ESP-NOW)

## Troubleshooting

**Los ESP-01 no responden**: 
- Verifica que el D1 Mini esté online primero
- Comprueba la MAC en los códigos

**Conexión Bluetooth no funciona**: 
- Reinicia el ESP32 y la app Bluetooth
- Verifica que el Serial Monitor no está abierto

**Sensores no leen**: 
- Comprueba los pines D3 (freno) y D7 (marcha atrás) del D1
- Verifica que INPUT_PULLUP esté habilitado

**ESP-NOW falla en D1 Mini**:
- Asegúrate de usar WiFi.mode(WIFI_STA) antes de esp_now_init()
- No uses WiFi.begin() ni WiFi.connect()
