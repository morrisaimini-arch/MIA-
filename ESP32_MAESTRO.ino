#include <BluetoothSerial.h>
#include <FastLED.h>
#include <esp_now.h>
#include <WiFi.h>

BluetoothSerial SerialBT;

#define PIN 13
#define NUM_LEDS 64
CRGB leds[NUM_LEDS];

CRGB verde  = CRGB(0, 255, 0);
CRGB morado = CRGB(85, 0, 85);

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

uint8_t cara_1[64] = {
  0,1,0,0,0,0,1,0,
  1,0,1,0,0,1,0,1,
  0,1,0,0,0,0,1,0,
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
    leds[i] = frame[i] ? verde : morado;
  }
  FastLED.show();
}

void animacionCaras() {
  const uint8_t* frames[] = {cara_1, cara_x, cara_3, cara_4, cara_5};
  const int numFrames = 5;

  unsigned long inicio = millis();

  while (millis() - inicio < 3000) {
    for (int i = 0; i < numFrames; i++) {
      setFrame(frames[i]);
      delay(200);
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

    // ==================== INTERMITENTES ====================
    // INT_IZQ: local en D1
    // INT_DER: controlado por ESP-01 #1 (8A:31:C0:57:80:10)

    else if (comando == "IZQ") {
      enviarAlD1("IZQ", true);
      animacionCaras();
    }
    else if (comando == "DER") {
      enviarAlD1("DER", true);
      animacionCaras();
    }
    else if (comando == "INT_OFF") {
      enviarAlD1("IZQ", false);
      enviarAlD1("DER", false);
    }

    // ==================== MARCHA ATRÁS ====================
    // Controlado por ESP-01 #2 (8E:77:5D:E0:6E:DA)

    else if (comando == "MAR") {
      enviarAlD1("MAR", true);
      animacionCaras();
    }
    else if (comando == "MAR_OFF") {
      enviarAlD1("MAR", false);
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
