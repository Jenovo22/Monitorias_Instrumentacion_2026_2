#include <WiFi.h>           // Incluido aunque no se use, necesario en algunos entornos
#include "BluetoothSerial.h"
#include <Adafruit_NeoPixel.h>

#define LED_PIN 8
#define NUM_LEDS 1

BluetoothSerial SerialBT;
Adafruit_NeoPixel rgb(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32C6_BT");
  rgb.begin();
  rgb.show();  // Apaga al inicio
  Serial.println("Bluetooth listo en ESP32-C6. Conéctate desde el celular.");
}

void setColor(int r, int g, int b) {
  rgb.setPixelColor(0, rgb.Color(r, g, b));
  rgb.show();
}

void loop() {
  if (SerialBT.available()) {
    String msg = SerialBT.readStringUntil('\n');
    msg.trim();

    if (msg == "ON") setColor(255, 0, 0);      // Rojo
    else if (msg == "OFF") setColor(0, 0, 0);  // Apagar
    else if (msg.startsWith("#")) {
      long color = strtol(msg.substring(1).c_str(), NULL, 16);
      int r = (color >> 16) & 0xFF;
      int g = (color >> 8) & 0xFF;
      int b = color & 0xFF;
      setColor(r, g, b);
    }
  }

  float temp = 25 + random(-2, 2);
  SerialBT.println(String("Temp: ") + temp + " °C");
  delay(1000);
}
