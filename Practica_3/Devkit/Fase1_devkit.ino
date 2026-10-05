#include <Arduino.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

int rPin = 25;
int gPin = 26;
int bPin = 27;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_RGB_ALT");  // Nombre Bluetooth visible
  Serial.println("Bluetooth listo. Conéctate con la app Serial Bluetooth Terminal.");

  pinMode(rPin, OUTPUT);
  pinMode(gPin, OUTPUT);
  pinMode(bPin, OUTPUT);
}

void setColor(int r, int g, int b) {
  analogWrite(rPin, r);
  analogWrite(gPin, g);
  analogWrite(bPin, b);
}

void loop() {
  if (SerialBT.available()) {
    String msg = SerialBT.readStringUntil('\n');
    msg.trim();

    if (msg == "ON") setColor(255, 0, 0);       // Rojo
    else if (msg == "OFF") setColor(0, 0, 0);   // Apagar
    else if (msg.startsWith("#")) {
      long color = strtol(msg.substring(1).c_str(), NULL, 16);
      int r = (color >> 16) & 0xFF;
      int g = (color >> 8) & 0xFF;
      int b = color & 0xFF;
      setColor(r, g, b);
    }
  }

  float temp = 25 + random(-3, 3);
  SerialBT.println(String("Temp: ") + temp + " °C");
  delay(1000);
}
