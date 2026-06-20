#include <BluetoothSerial.h>
#include <FastLED.h>
#include <esp_now.h>
#include <WiFi.h>

BluetoothSerial SerialBT;

#define PIN 13
#define NUM_LEDS 64
CRGB leds[NUM_LEDS];

// ==================== COLORES ====================
CRGB verde  = CRGB(0, 255, 0);
CRGB morado = CRGB(85, 0, 85);
CRGB negro  = CRGB(0, 0, 0);

// Colores para modo noche
CRGB colorActivo_dia   = CRGB(0, 255, 0);      // Verde
CRGB colorFondo_dia    = CRGB(85, 0, 85);      // Morado
CRGB colorActivo_noche = CRGB(85, 0, 85);      // Morado
CRGB colorFondo_noche  = CRGB(0, 0, 0);        // Negro (apagado)

CRGB colorActivo  = colorActivo_dia;
CRGB colorFondo   = colorFondo_dia;

bool modoNocturna = false;
bool modoEmergencia = false;

int brillo = 50;

// ==================== DIRECCIÓN MAC DEL D1 MINI ====================
// MAC del D1 Mini: 22:9C:4A:4A:FD:0D
uint8_t broadcastAddress[] = {0x22, 0x9C, 0x4A, 0x4A, 0xFD, 0x0D};

// ==================== ESTRUCTURA DE DATOS ESP-NOW ====================
typedef struct struct_message {
    char comando[4];
    bool estado;
} struct_message;

struct_message mensajeEnvio;
struct_message datosRecibidos;

// ==================== CARAS ====================

// CARA 1 - Ojos abiertos (sonrisa)
uint8_t cara_1[64] = {
  0,0,0,0,0,0,0,0,
  1,1,1,0,0,1,1,1,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,1,0,0,0,0,1,0,
  0,0,1,1,1,1,0,0,
  0,0,0,0,0,0,0,0,
};

// CARA 1 PARPADEO - Ojos cerrados (parpadeo)
uint8_t cara_1_parpadeo[64] = {
  0,0,0,0,0,0,0,0,
  0,1,1,0,0,1,1,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,1,0,0,0,0,1,0,
  0,0,1,1,1,1,0,0,
  0,0,0,0,0,0,0,0,
};

uint8_t cara_x[64] = {
  0,1,0,0,0,0,1,0,
  1,0,1,0,0,1,0,1,
  0,1,0,0,0,0,1,0,
  0,0,0,0,0,0,0,0,
  0,0,1,1,1,1,0,0,
  0,0,1,0,0,1,0,0,
  0,0,1,1,1,1,0,0,
  0,0,0,0,0,0,0,0,
};

uint8_t cara_3[64] = {
  0,1,0,0,0,0,1,0,
  1,0,1,0,0,1,0,1,
  0,1,0,0,0,0,1,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,1,1,0,0,
  0,0,1,1,1,1,0,0,
  0,0,0,0,0,0,0,0,
};

uint8_t cara_4[64] = {
  0,1,0,0,0,0,1,0,
  1,0,1,0,0,1,0,1,
  0,1,0,0,0,0,1,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,1,1,0,0,0,
  0,0,1,1,1,1,0,0,
  0,0,0,0,0,0,0,0,
};

uint8_t cara_5[64] = {
  0,1,0,0,0,0,1,0,
  1,0,1,0,0,1,0,1,
  0,1,0,0,0,0,1,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,1,1,0,0,0,0,
  0,0,1,1,1,1,0,0,
  0,0,0,0,0,0,0,0,
};

uint8_t cara_pmo[64] = {
  0,1,1,0,0,0,0,0,
  1,0,1,1,0,0,0,0,
  1,0,0,1,0,1,1,1,
  0,1,1,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  1,1,1,0,1,1,1,0,
  1,0,1,1,1,0,1,1,
};

// ==================== PINES ====================

const int PIN_MORON    = 2;
const int PIN_POSICION = 4;
const int PIN_CORTA    = 5;
const int PIN_LARGA    = 18;
const int PIN_PINCHAZO = 21;
const int PIN_CORTESIA = 22;
const int PIN_NIEBLAS  = 23;
const int PIN_LIMPIA   = 19;

String comando = "";

// ==================== CALLBACK ESP-NOW ====================

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("[ESP-NOW] Estado envío: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Éxito" : "Fallo");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&datosRecibidos, incomingData, sizeof(datosRecibidos));
  Serial.print("[ESP-NOW] Recibido del D1: ");
  Serial.print(datosRecibidos.comando);
  Serial.println(datosRecibidos.estado ? " - HIGH" : " - LOW");
}

// ==================== FUNCIONES ====================

void setFrame(const uint8_t frame[64]) {
  for (int i = 0; i < 64; i++) {
    leds[i] = frame[i] ? colorActivo : colorFondo;
  }
  FastLED.show();
}

// Parpadeo simple - Abre y cierra los ojos
void parpadeo(int repeticiones = 1) {
  for (int i = 0; i < repeticiones; i++) {
    setFrame(cara_1_parpadeo);  // Ojos cerrados
    delay(100);
    setFrame(cara_1);           // Ojos abiertos
    delay(100);
  }
}

void animacionCaras() {
  const uint8_t* frames[] = {cara_1, cara_x, cara_3, cara_4, cara_5};
  const int numFrames = 5;

  unsigned long inicio = millis();

  while (millis() - inicio < 3000) {
    for (int i = 0; i < numFrames; i++) {
      setFrame(frames[i]);
      delay(200);
      
      // Agregar parpadeo cada 2 caras
      if (i == 1) {
        parpadeo(1);
      }
    }
  }
}

void animacionPMO() {
  while (true) {
    setFrame(cara_pmo);
    delay(1000);
    setFrame(cara_3);
    delay(200);
    setFrame(cara_4);
    delay(200);

    if (SerialBT.available()) return;
  }
}

// ===== MODO EMERGENCIA - Parpadeo de todos los intermitentes =====
void modoEmergenciaActivado() {
  modoEmergencia = true;
  Serial.println("[EMERGENCIA] 🚨 Modo activado - Intermitentes en parpadeo");
  
  // Parpadeo rápido con toggle cada 250ms
  while (modoEmergencia && !SerialBT.available()) {
    // ENCENDER todos
    enviarAlD1("IZQ", true);
    enviarAlD1("DER", true);
    delay(250);
    
    // APAGAR todos
    enviarAlD1("IZQ", false);
    enviarAlD1("DER", false);
    delay(250);
  }
  
  modoEmergencia = false;
  Serial.println("[EMERGENCIA] Modo desactivado");
}

void togglePin(int pin, String comando) {
  int estadoActual = digitalRead(pin);
  digitalWrite(pin, estadoActual == LOW ? HIGH : LOW);

  if (comando != "PMO") {
    animacionCaras();
  }
}

// Enviar comando al D1 Mini por ESP-NOW
void enviarAlD1(const char* comando, bool estado) {
  strcpy(mensajeEnvio.comando, comando);
  mensajeEnvio.estado = estado;
  esp_now_send(broadcastAddress, (uint8_t *) &mensajeEnvio, sizeof(mensajeEnvio));
  Serial.print("[ESP32] Enviado al D1: ");
  Serial.print(comando);
  Serial.println(estado ? " - HIGH" : " - LOW");
}

// Cambiar modo noche/día
void cambiarModo(bool nocturna) {
  modoNocturna = nocturna;
  
  if (nocturna) {
    colorActivo = colorActivo_noche;
    colorFondo = colorFondo_noche;
    Serial.println("[MODO] 🌙 NOCTURNA activado");
  } else {
    colorActivo = colorActivo_dia;
    colorFondo = colorFondo_dia;
    Serial.println("[MODO] ☀️ DIURNA activado");
  }
  
  // Mostrar cara actual con nuevo modo
  setFrame(cara_1);
}

// ==================== SETUP ====================

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n[ESP32] Iniciando...");

  FastLED.addLeds<WS2812B, PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(brillo);

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Green;
  }
  FastLED.show();

  pinMode(PIN_MORON,    OUTPUT);
  pinMode(PIN_POSICION, OUTPUT);
  pinMode(PIN_CORTA,    OUTPUT);
  pinMode(PIN_LARGA,    OUTPUT);
  pinMode(PIN_PINCHAZO, OUTPUT);
  pinMode(PIN_CORTESIA, OUTPUT);
  pinMode(PIN_NIEBLAS,  OUTPUT);
  pinMode(PIN_LIMPIA,   OUTPUT);

  digitalWrite(PIN_MORON,    HIGH);
  digitalWrite(PIN_POSICION, HIGH);
  digitalWrite(PIN_CORTA,    HIGH);
  digitalWrite(PIN_LARGA,    HIGH);
  digitalWrite(PIN_PINCHAZO, HIGH);
  digitalWrite(PIN_CORTESIA, HIGH);
  digitalWrite(PIN_NIEBLAS,  HIGH);
  digitalWrite(PIN_LIMPIA,   HIGH);

  // ===== CONFIGURAR ESP-NOW =====
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERROR] Fallo al inicializar ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("[ERROR] Fallo al añadir peer D1 Mini");
    return;
  }

  SerialBT.begin("MIA_BRAIN");
  Serial.println("[ESP32] Maestro iniciado correctamente");
  Serial.println("[ESP32] MAC D1 Mini configurada: 22:9C:4A:4A:FD:0D");
}

// ==================== LOOP ====================

void loop() {

  if (SerialBT.available()) {
    comando = SerialBT.readString();
    comando.trim();

    Serial.print("[Bluetooth] Comando recibido: ");
    Serial.println(comando);

    // ==================== BRILLO ====================

    if (comando == "BUP") {
      brillo += 10;
      if (brillo > 255) brillo = 255;
      FastLED.setBrightness(brillo);
      FastLED.show();
    }

    else if (comando == "BDO") {
      brillo -= 10;
      if (brillo < 0) brillo = 0;
      FastLED.setBrightness(brillo);
      FastLED.show();
    }

    else if (comando.startsWith("B") && comando.length() > 1) {
      int nuevoBrillo = comando.substring(1).toInt();
      nuevoBrillo = constrain(nuevoBrillo, 0, 255);
      brillo = nuevoBrillo;
      FastLED.setBrightness(brillo);
      FastLED.show();
    }

    // ==================== MODO EMERGENCIA ====================
    // Comando: EME
    // Activa todos los intermitentes con toggle cada 250ms

    else if (comando == "EME") {
      modoEmergenciaActivado();
    }

    // ==================== MODO NOCTURNA / DIURNA ====================
    // Comandos cortos: NOC y DIU
    // Contains: "nocturna" y "diurna" (en frases completas)

    else if (comando == "NOC" || comando.indexOf("nocturna") >= 0) {
      cambiarModo(true);
    }

    else if (comando == "DIU" || comando.indexOf("diurna") >= 0) {
      cambiarModo(false);
    }

    // ==================== PARPADEO ====================
    
    else if (comando == "PAR") {
      parpadeo(3);  // 3 parpadeos
    }

    // ==================== RESTO DE COMANDOS (relés locales) ====================

    else if (comando == "POS") togglePin(PIN_POSICION, comando);
    else if (comando == "CRT") togglePin(PIN_CORTA, comando);
    else if (comando == "LRG") togglePin(PIN_LARGA, comando);
    else if (comando == "MUS") togglePin(PIN_PINCHAZO, comando);
    else if (comando == "COR") togglePin(PIN_CORTESIA, comando);
    else if (comando == "ANT") togglePin(PIN_NIEBLAS, comando);
    else if (comando == "LIM") togglePin(PIN_LIMPIA, comando);

    else if (comando == "PMO") {
      digitalWrite(PIN_MORON, LOW);
      animacionPMO();
    }

    else if (comando == "SER") {
      digitalWrite(PIN_MORON, HIGH);
      setFrame(cara_1);
    }

    else if (comando == "HAG") {
      digitalWrite(PIN_POSICION, LOW);
      digitalWrite(PIN_CORTA,    LOW);
      digitalWrite(PIN_LARGA,    LOW);
      animacionCaras();
    }

    else if (comando == "FIN") {
      digitalWrite(PIN_POSICION, HIGH);
      digitalWrite(PIN_CORTA,    HIGH);
      digitalWrite(PIN_LARGA,    HIGH);
      digitalWrite(PIN_CORTESIA, HIGH);
      digitalWrite(PIN_NIEBLAS,  HIGH);
      digitalWrite(PIN_LIMPIA,   HIGH);
      digitalWrite(PIN_PINCHAZO, HIGH);
      enviarAlD1("IZQ", false);
      enviarAlD1("DER", false);
      enviarAlD1("MAR", false);
      animacionCaras();
    }
  }
}
