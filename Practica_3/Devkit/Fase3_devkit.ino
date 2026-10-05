#include <WiFi.h>
#include <PubSubClient.h>

// =========================
// CONFIGURACIÓN DE PINES RGB
// =========================
// Cambia estos pines según cómo conectes tu LED RGB
#define RED_PIN   25
#define GREEN_PIN 26
#define BLUE_PIN  27

// Si tu LED es de ánodo común, cambia esta variable a true
bool anodoComun = false;

// =========================
// CONFIGURACIÓN WIFI + MQTT
// =========================
const char* ssid = "Familia Arango";
const char* password = "3107925206";
const char* mqtt_server = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

// =========================
// FUNCIONES
// =========================

// Aplica el color usando PWM
void setColor(uint8_t r, uint8_t g, uint8_t b) {
  if (anodoComun) {
    // Invertir valores si el LED es de ánodo común
    r = 255 - r;
    g = 255 - g;
    b = 255 - b;
  }

  analogWrite(RED_PIN,   r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN,  b);

  Serial.printf("LED RGB -> R:%d G:%d B:%d\n", r, g, b);
}

// Callback al recibir mensajes MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido en ");
  Serial.print(topic);
  Serial.print(": ");

  String msg;
  for (int i = 0; i < length; i++) msg += (char)payload[i];
  Serial.println(msg);

  // Espera un color en formato #RRGGBB
  if (msg.length() == 7 && msg[0] == '#') {
    long number = (long)strtol(msg.substring(1).c_str(), NULL, 16);
    uint8_t r = (number >> 16) & 0xFF;
    uint8_t g = (number >> 8) & 0xFF;
    uint8_t b = number & 0xFF;
    setColor(r, g, b);
  }
}

// Reintento de conexión MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando al broker MQTT...");
    if (client.connect("ESP32_RGB_Client")) {
      Serial.println("Conectado ✅");
      client.subscribe("esp32/led"); // Topic para cambiar color
    } else {
      Serial.print("Fallo rc=");
      Serial.println(client.state());
      Serial.println("Reintentando en 5 segundos...");
      delay(5000);
    }
  }
}

// =========================
// SETUP
// =========================
void setup() {
  Serial.begin(115200);

  // Configurar PWM en pines RGB
  ledcAttachPin(RED_PIN,   0);
  ledcAttachPin(GREEN_PIN, 1);
  ledcAttachPin(BLUE_PIN,  2);

  ledcSetup(0, 5000, 8); // Canal 0, 5kHz, resolución 8 bits
  ledcSetup(1, 5000, 8);
  ledcSetup(2, 5000, 8);

  setColor(0, 0, 0); // Apagar al inicio

  // Conexión WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi conectado");
  Serial.print("IP local: ");
  Serial.println(WiFi.localIP());

  // Configurar MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// =========================
// LOOP
// =========================
void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // Publicar temperatura simulada cada 2 s
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    float sensorValue = 25.0 + random(-300, 300) / 100.0;  // 22–28°C
    String payload = String(sensorValue, 2);
    Serial.print("📤 Publicando temperatura: ");
    Serial.println(payload);
    client.publish("esp32/sensor/temperatura", payload.c_str());
  }
}
