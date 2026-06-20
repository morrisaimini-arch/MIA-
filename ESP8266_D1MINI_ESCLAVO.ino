#include <ESP8266WiFi.h>
#include <espnow.h>

// ===== CONFIGURACIÓN DE PINES DE SALIDA (RELÉS) =====
const int PIN_INT_IZQ      = D1;  // Intermitente Izquierdo (LOCAL)
const int PIN_INT_DER      = D2;  // Intermitente Derecho (→ ESP-01 #1)
const int PIN_FRENO        = D5;  // Luz de Freno (LOCAL + SENSOR)
const int PIN_MARCHA_ATRAS = D6;  // Luz de Marcha Atrás (→ ESP-01 #2)

// ===== CONFIGURACIÓN DE PINES DE ENTRADA (SENSORES / INTERRUPTORES) =====
const int SENSOR_FRENO = D3;      // Interruptor del pedal de freno
const int SENSOR_ATRAS = D7;      // Interruptor de la caja de cambios (marcha atrás)

// ===== DIRECCIONES MAC DE LOS ESP-01 =====
uint8_t esp01_intermitente[] = {0x8A, 0x31, 0xC0, 0x57, 0x80, 0x10};  // INT_DER
uint8_t esp01_marcha_atras[] = {0x8E, 0x77, 0x5D, 0xE0, 0x6E, 0xDA};  // MARCHA_ATRAS

// ===== ESTRUCTURA DE DATOS ESP-NOW =====
typedef struct struct_mensaje {
    char comando[4];
    bool estado;
} struct_mensaje;

struct_mensaje miDatos;
struct_mensaje datosEnvio;

// ===== CALLBACK PARA RECIBIR DATOS DEL MAESTRO =====
void onDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&miDatos, incomingData, sizeof(miDatos));
 
  String cmd = String(miDatos.comando);
  bool encender = miDatos.estado;
  int valorPin = encender ? HIGH : LOW;

  Serial.print("[ESP-NOW D1] Comando: ");
  Serial.print(cmd);
  Serial.println(encender ? " - ON" : " - OFF");

  // INTERMITENTE IZQUIERDO (LOCAL)
  if (cmd == "IZQ") {
    digitalWrite(PIN_INT_IZQ, valorPin);
    Serial.println("  → INT_IZQ (LOCAL)");
  }
  // INTERMITENTE DERECHO (enviado a ESP-01 #1)
  else if (cmd == "DER") {
    digitalWrite(PIN_INT_DER, valorPin);
    enviarAlESP01(esp01_intermitente, cmd, encender);
    Serial.println("  → INT_DER (ESP-01 #1)");
  }
  // MARCHA ATRÁS (enviado a ESP-01 #2)
  else if (cmd == "MAR") {
    digitalWrite(PIN_MARCHA_ATRAS, valorPin);
    enviarAlESP01(esp01_marcha_atras, cmd, encender);
    Serial.println("  → MARCHA_ATRAS (ESP-01 #2)");
  }
}

// ===== CALLBACK CONFIRMACIÓN DE ENVÍO =====
void onDataSent(uint8_t *mac_addr, uint8_t status) {
  Serial.print("[D1→ESP-01] Envío: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Éxito" : "Fallo");
}

// ===== FUNCIÓN PARA ENVIAR DATOS A LOS ESP-01 =====
void enviarAlESP01(uint8_t *mac, String cmd, bool estado) {
  strcpy(datosEnvio.comando, cmd.c_str());
  datosEnvio.estado = estado;
  esp_now_send(mac, (uint8_t *) &datosEnvio, sizeof(datosEnvio));
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n[D1 MINI] Iniciando...");
 
  // Configurar relés como salida
  pinMode(PIN_INT_IZQ, OUTPUT);
  pinMode(PIN_INT_DER, OUTPUT);
  pinMode(PIN_FRENO, OUTPUT);
  pinMode(PIN_MARCHA_ATRAS, OUTPUT);
 
  // Inicializar todo apagado
  digitalWrite(PIN_INT_IZQ, LOW);
  digitalWrite(PIN_INT_DER, LOW);
  digitalWrite(PIN_FRENO, LOW);
  digitalWrite(PIN_MARCHA_ATRAS, LOW);

  // Configurar sensores como entrada con Pull-up interno
  pinMode(SENSOR_FRENO, INPUT_PULLUP);
  pinMode(SENSOR_ATRAS, INPUT_PULLUP);

  // Configurar Wi-Fi y ESP-NOW
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != 0) {
    Serial.println("[ERROR] Error inicializando ESP-NOW");
    return;
  }
 
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(onDataRecv);
  esp_now_register_send_cb(onDataSent);

  // Añadir ESP-01 #1 (Intermitente Derecho)
  esp_now_add_peer(esp01_intermitente, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  // Añadir ESP-01 #2 (Marcha Atrás)
  esp_now_add_peer(esp01_marcha_atras, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  Serial.println("[D1 MINI] Esclavo iniciado correctamente");
  Serial.println("  - INT_IZQ (D1): LOCAL");
  Serial.println("  - INT_DER (D2): ESP-01 #1 (8A:31:C0:57:80:10)");
  Serial.println("  - FRENO (D5): LOCAL + SENSOR");
  Serial.println("  - MARCHA_ATRAS (D6): ESP-01 #2 (8E:77:5D:E0:6E:DA)");
}

// ===== LOOP =====
void loop() {
  // --- LEER SENSOR DE FRENO ---
  if (digitalRead(SENSOR_FRENO) == LOW) {
    digitalWrite(PIN_FRENO, HIGH);
  } else {
    digitalWrite(PIN_FRENO, LOW);
  }

  // --- LEER SENSOR DE MARCHA ATRÁS ---
  if (digitalRead(SENSOR_ATRAS) == LOW) {
    // El sensor detecta marcha atrás, pero el relé se controla por ESP32
    // Este código es de referencia si necesitas lógica local
  }

  delay(50);
}
