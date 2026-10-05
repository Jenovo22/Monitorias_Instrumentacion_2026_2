#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define ANCHO 128
#define ALTO 64
#define OLED_RESET -1
Adafruit_SSD1306 display(ANCHO, ALTO, &Wire, OLED_RESET);

int potPin = 34;
int hora = 3;
int minuto = 26;
int segundo = 0;
unsigned long previo = 0;
unsigned long intervalo = 1000;

void setup() {
  Serial.begin(115200);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("No se detecta pantalla SSD1306");
    for (;;);
  }
  display.clearDisplay();
  display.display();
  Serial.println("Iniciando reloj digital con lectura de potenciómetro...");
}

void loop() {
  int valor = analogRead(potPin);
  unsigned long actual = millis();

  if (actual - previo >= intervalo) {
    previo = actual;
    segundo++;
    if (segundo >= 60) {
      segundo = 0;
      minuto++;
      if (minuto >= 60) {
        minuto = 0;
        hora++;
        if (hora >= 24) hora = 0;
      }
    }

    Serial.print("Hora actual: ");
    if (hora < 10) Serial.print("0");
    Serial.print(hora);
    Serial.print(":");
    if (minuto < 10) Serial.print("0");
    Serial.print(minuto);
    Serial.print(":");
    if (segundo < 10) Serial.print("0");
    Serial.print(segundo);
    Serial.print(" | Potenciometro: ");
    Serial.println(valor);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 10);
  if (hora < 10) display.print("0");
  display.print(hora);
  display.print(":");
  if (minuto < 10) display.print("0");
  display.print(minuto);
  display.print(":");
  if (segundo < 10) display.print("0");
  display.print(segundo);

  display.setTextSize(1);
  display.setCursor(10, 50);
  display.print("POT: ");
  display.print(valor);
  display.display();

  delay(100);
}
