/*
 * PRÁCTICA DE INSTRUMENTACIÓN: INTERFAZ UART
 * Ejercicio 1: Juego "Adivinar el Número" (VERSIÓN COMPLETA)
 */

// Variables globales para la máquina de estados
int estadoJuego = 0; // 0: Esperando Mínimo, 1: Esperando Máximo, 2: Jugando
long valorMin = 0;
long valorMax = 0;
long numeroSecreto = 0;

void setup() {
  // Inicializar comunicación serial a 115200 baudios (Típico para ESP32)
  Serial.begin(115200);
  
  // Pausa para asegurar que el monitor serial inicie correctamente
  delay(1000); 
  
  Serial.println("\n--- JUEGO: ADIVINA EL NUMERO ---");
  Serial.println("Ingrese el valor MINIMO del rango:");
  
  // Inicializar la semilla para la generación de números aleatorios leyendo un pin al aire
  randomSeed(analogRead(36)); // El pin 36 (VP) suele estar libre en el ESP32 Devkit
}

void loop() {
  // Verificar si hay datos disponibles en el buffer serial
  if (Serial.available() > 0) {
    
    // Serial.parseInt() lee los caracteres y los convierte a un número entero
    long entrada = Serial.parseInt();
    
    // --- BLOQUE ANTIBASURA ---
    // Limpiar saltos de línea ('\n', '\r') o caracteres residuales en el buffer
    while (Serial.available() > 0) {
      Serial.read();
    }
    // -------------------------
    
    // Máquina de estados para gestionar el flujo de las preguntas
    switch (estadoJuego) {
      
      case 0: // ESTADO 0: Recibiendo Valor Mínimo
        valorMin = entrada;
        Serial.print("Valor minimo establecido en: ");
        Serial.println(valorMin);
        
        Serial.println("Ahora, ingrese el valor MAXIMO del rango:");
        estadoJuego = 1; // Pasar a la siguiente fase
        break;
        
      case 1: // ESTADO 1: Recibiendo Valor Máximo
        valorMax = entrada;
        
        // Validación de seguridad
        if (valorMax <= valorMin) {
          Serial.println("Error: El valor maximo debe ser mayor al minimo. Intente de nuevo:");
        } else {
          Serial.print("Valor maximo establecido en: ");
          Serial.println(valorMax);
          
          // Generar el número aleatorio. La función random es exclusiva (max-1), 
          // por eso sumamos 1 al máximo para que sea inclusivo.
          numeroSecreto = random(valorMin, valorMax + 1);
          
          Serial.println("\n¡Juego iniciado! Ingresa tu primer intento:");
          estadoJuego = 2; // Pasar a la fase de juego
        }
        break;
        
      case 2: // ESTADO 2: Fase de Adivinanza
        Serial.print("Tu intento: ");
        Serial.println(entrada);
        
        // Lógica del juego
        if (entrada > numeroSecreto) {
          Serial.println("El numero secreto es MENOR.");
        } 
        else if (entrada < numeroSecreto) {
          Serial.println("El numero secreto es MAYOR.");
        } 
        else {
          // Si no es ni mayor ni menor, es igual
          Serial.println("\n¡EXCELENTE! ¡Adivinaste el numero!");
          Serial.println("-----------------------------------");
          Serial.println("Vamos a jugar de nuevo. Ingrese un nuevo valor MINIMO:");
          
          // Reiniciar la máquina de estados para jugar otra vez
          estadoJuego = 0; 
        }
        break;
    }
  }
}