#include <ESP8266WiFi.h>
#include <espnow.h>

// ===== CONFIGURACIÓN =====
// MAC: 8A:31:C0:57:80:10
// Función: Controla el relé del INTERMITENTE TRASERO DERECHO

const int PIN_RELAY = D1;  // GPIO5

// Estructura para recibir datos
typedef struct struct_mensaje {
    char comando[4];
    bool estado;
} struct_mensaje;

struct_mensaje miDatos;

// ===== CALLBACK PARA RECIBIR DATOS =====
void onDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&miDatos, incomingData, sizeof(miDatos));
 
  String cmd = String(miDatos.comando);
  bool encender = miDatos.estado;

  Serial.print("[ESP-01 INT] Comando: ");
  Serial.print(cmd);
  Serial.println(encender ? " - ON" : " - OFF");

  if (cmd == "DER") {
    digitalWrite(PIN_RELAY, encender ? HIGH : LOW);
    Serial.print("  → Relé INT_DER: ");
    Serial.println(encender ? "ACTIVADO" : "DESACTIVADO");
  }
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n[ESP-01 INTERMITENTE] Iniciando...");
  Serial.println("MAC: 8A:31:C0:57:80:10");
  Serial.println("Función: Intermitente Trasero Derecho");

  // Configurar relé como salida
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);  // Apagado inicialmente

  // Configurar Wi-Fi y ESP-NOW
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != 0) {
    Serial.println("[ERROR] Error inicializando ESP-NOW");
    return;
  }
 
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(onDataRecv);

  Serial.println("[ESP-01 INT] Listo y esperando órdenes del D1 Mini");
}

// ===== LOOP =====
void loop() {
  delay(1000);
}
