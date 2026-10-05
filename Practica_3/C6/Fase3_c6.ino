#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 8
#define NUM_LEDS 1

const char* ssid = "Familia Arango";
const char* password = "3107925206";

const char* mqtt_server = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_NeoPixel rgb( NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800 );

unsigned long lastMsg = 0;

// Función para aplicar color al Neopixel
void setColor(uint8_t r, uint8_t g, uint8_t b) {
  rgb.setPixelColor(0, rgb.Color(r,g,b));
  rgb.show();
  Serial.printf("LED RGB -> R:%d G:%d B:%d\n", r,g,b);
}

// Callback al recibir mensajes
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido en ");
  Serial.print(topic);
  Serial.print(": ");

  String msg;
  for (int i = 0; i < length; i++) msg += (char)payload[i];
  Serial.println(msg);

  // Espera un color en formato #RRGGBB
  if (msg.length() == 7 && msg[0] == '#') {
    long number = (long) strtol(msg.substring(1).c_str(), NULL, 16);
    uint8_t r = (number >> 16) & 0xFF;
    uint8_t g = (number >> 8) & 0xFF;
    uint8_t b = number & 0xFF;
    setColor(r,g,b);
  }
}

// Reconexion MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando al broker MQTT...");
    if (client.connect("ESP32C6_NeoPixel")) {
      Serial.println("Conectado ✅");
      client.subscribe("esp32c6/led"); // Topic de control de color
    } else {
      Serial.print("Fallo rc=");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Configuración LED
  rgb.begin();
  rgb.show(); // Apaga al inicio

  // Configuración WiFi
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

  // Configuración MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // Publicar sensor cada 2s
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    float sensorValue = 25.0 + random(-300,300)/100.0; // 22–28°C
    String payload = String(sensorValue,2);
    Serial.print("📤 Publicando temperatura: ");
    Serial.println(payload);
    client.publish("esp32c6/sensor/temperatura", payload.c_str());
  }
}
