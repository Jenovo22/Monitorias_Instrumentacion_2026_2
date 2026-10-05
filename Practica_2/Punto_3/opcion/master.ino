#include <SPI.h>
//master
#define PIN_SS 5

void setup() {
  Serial.begin(115200);
  SPI.begin(18, 19, 23, PIN_SS);
  pinMode(PIN_SS, OUTPUT);
  digitalWrite(PIN_SS, HIGH);
}

void loop() {
  uint8_t value = random(0, 255); // numero aleatorio
  digitalWrite(PIN_SS, LOW);      // iniciar comunicación

  uint8_t response = SPI.transfer(value);

  digitalWrite(PIN_SS, HIGH);     // terminar comunicación

  Serial.print("Master envió: ");
  Serial.print(value);
  Serial.print(" | Slave respondió: ");
  Serial.println(response);

  delay(1000);
}
