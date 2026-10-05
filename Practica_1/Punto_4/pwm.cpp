#include <ESP32Servo.h>

const int ldrIzq = 35;   // ADC1_CH0
const int ldrDer = 34;   // ADC1_CH3
const int pinServo = 32; // PWM
const int ledAlarma = 26; // LED opcional

Servo servo;

// Variables de calibración
float Rfija = 10000.0; // 10kΩ
float Ksensibilidad = 0.8; // Ajusta la respuesta al contraste de luz
int angulo = 90; // posición inicial centrada

void setup() {
  Serial.begin(115200);
  servo.attach(pinServo);
  pinMode(ledAlarma, OUTPUT);
  digitalWrite(ledAlarma, LOW);
  servo.write(angulo);
  delay(1000);
  Serial.println("Sistema de seguimiento de luz iniciado");
}

void loop() {
  // Lecturas analógicas
  int vIzq = analogRead(ldrIzq);
  int vDer = analogRead(ldrDer);

  // Conversión a voltaje
  float VadcIzq = (vIzq / 4095.0) * 3.3;
  float VadcDer = (vDer / 4095.0) * 3.3;

  // Calcular resistencia de cada LDR
  float RldrIzq = (3.3 * Rfija / VadcIzq) - Rfija;
  float RldrDer = (3.3 * Rfija / VadcDer) - Rfija;

  // Diferencia relativa (negativo: más luz en derecha, positivo: más en izquierda)
  float diff = (RldrDer - RldrIzq) / Rfija;

  // Control proporcional simple
  angulo = angulo + diff * 20 * Ksensibilidad;

  // Limitar ángulo entre 0 y 90 grados
  angulo = constrain(angulo, 0, 90);
  servo.write(angulo);

  // Alarma si hay mucha diferencia
  if (fabs(diff) > 0.5)
    digitalWrite(ledAlarma, HIGH);
  else
    digitalWrite(ledAlarma, LOW);

  // Impresiones
  Serial.print("LDR Izq (RΩ): "); Serial.print(RldrIzq, 0);
  Serial.print(" | LDR Der (RΩ): "); Serial.print(RldrDer, 0);
  Serial.print(" | Diff: "); Serial.print(diff, 2);
  Serial.print(" | Servo: "); Serial.println(angulo);

  delay(300);
}
