// ================================
//   CONTADOR DE PULSOS EN GPIO32
//   CON TIEMPO ENTRE PULSOS
// ================================

#define BOTON_PIN 32  // Pin donde está conectado el botón

unsigned long tiempoAnterior = 0; // Guarda el tiempo de la última pulsación
unsigned long tiempoActual = 0;
unsigned long diferenciaTiempo = 0;
int contadorPulsos = 0;

void setup() {
  Serial.begin(115200);
  pinMode(BOTON_PIN, INPUT_PULLUP); // Usamos resistencia interna pull-up

  Serial.println("=== Iniciando contador de pulsos ===");
  Serial.println("Presiona el botón conectado al GPIO 32...");
}

void loop() {
  // Leemos el estado del botón (activo en LOW por pull-up)
  static bool estadoAnterior = HIGH;
  bool estadoActual = digitalRead(BOTON_PIN);

  // Detectar flanco de bajada (cuando se presiona el botón)
  if (estadoAnterior == HIGH && estadoActual == LOW) {
    tiempoActual = millis();
    diferenciaTiempo = tiempoActual - tiempoAnterior;

    if (contadorPulsos > 0) {
      Serial.print("⏱ Tiempo desde el último pulso: ");
      Serial.print(diferenciaTiempo);
      Serial.println(" ms");
    }

    contadorPulsos++;
    Serial.print("🔘 Pulsos contados: ");
    Serial.println(contadorPulsos);

    tiempoAnterior = tiempoActual;

    delay(500); 
  }

  estadoAnterior = estadoActual;
}
