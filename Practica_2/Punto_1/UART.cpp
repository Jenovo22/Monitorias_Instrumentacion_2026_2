int numeroSecreto;
int intento;
int intentos = 0;
int rangoMin = 1;
int rangoMax = 100;
bool juegoIniciado = false;
bool adivinado = false;

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));
  Serial.println("=== Juego de Adivinar el Número ===");
  Serial.println("Envía dos números separados por espacio para definir el rango (Ejemplo: 10 50)");
}

void loop() {
  if (!juegoIniciado && Serial.available() > 0) {
    rangoMin = Serial.parseInt();
    rangoMax = Serial.parseInt();
    if (rangoMin < rangoMax && rangoMax > 0) {
      numeroSecreto = random(rangoMin, rangoMax + 1);
      juegoIniciado = true;
      intentos = 0;
      Serial.print("Juego iniciado. Adivina el número entre ");
      Serial.print(rangoMin);
      Serial.print(" y ");
      Serial.println(rangoMax);
    }
  }

  if (juegoIniciado && Serial.available() > 0) {
    intento = Serial.parseInt();
    if (intento >= rangoMin && intento <= rangoMax) {
      intentos++;
      if (intento == numeroSecreto) {
        Serial.print("¡Correcto! Adivinaste el número en ");
        Serial.print(intentos);
        Serial.println(" intentos.");
        adivinado = true;
      } else if (intento < numeroSecreto) {
        Serial.println("El número es mayor.");
      } else {
        Serial.println("El número es menor.");
      }
    }
  }

  if (adivinado) {
    delay(2000);
    Serial.println("Envía nuevo rango para jugar otra vez (Ejemplo: 10 100)");
    juegoIniciado = false;
    adivinado = false;
  }
}
