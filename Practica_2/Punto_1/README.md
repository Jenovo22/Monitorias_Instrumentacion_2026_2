# Práctica 3: Interfaces de Comunicación Serial - Punto 1

## 1. Objetivo
[cite_start]Comprender el funcionamiento de la interfaz de comunicación serial asíncrona **UART** y el manejo de flujos de datos bidireccionales entre el sistema embebido (ESP32) y un computador[cite: 1, 2, 332].

## 2. Descripción del Ejercicio
[cite_start]Se implementa un juego interactivo de **"Adivinar el número"** utilizando la comunicación UART para la interacción con el usuario a través del Monitor Serial[cite: 4, 333]. El flujo del programa es el siguiente:
* [cite_start]**Configuración del Rango:** El sistema solicita al usuario el ingreso de un valor mínimo y un valor máximo[cite: 5, 334].
* [cite_start]**Generación de Número:** Se genera un número aleatorio dentro del rango establecido[cite: 335].
* **Fase de Adivinanza:** El jugador envía sus intentos. [cite_start]El sistema responde indicando si el número secreto es **mayor** o **menor** al enviado, hasta que se logra descubrir el número[cite: 6, 337].

## 3. Funcionamiento del Código
[cite_start]El programa está estructurado mediante una **máquina de estados** para gestionar la secuencia lógica de la comunicación[cite: 49, 50, 338]:
* [cite_start]**Estado 0:** Espera y procesa la entrada del valor mínimo[cite: 338].
* [cite_start]**Estado 1:** Espera y procesa el valor máximo, validando que sea superior al mínimo[cite: 339].
* [cite_start]**Estado 2:** Gestiona los intentos de adivinanza y realiza las comparaciones lógicas[cite: 340].

[cite_start]Para garantizar la integridad de los datos, el código incluye un **bloque antibasura** encargado de limpiar el buffer serial de caracteres residuales, como saltos de línea (`\n` o `\r`), tras procesar cada número[cite: 21, 49, 341].

## 4. Librerías Necesarias
[cite_start]Para este ejercicio **no se requieren librerías externas**[cite: 342]. [cite_start]Solo es necesario contar con el soporte para placas ESP32 instalado en el entorno de Arduino IDE[cite: 343].

## 5. Esquema de Conexión
[cite_start]La comunicación se realiza directamente a través del cable de programación del dispositivo[cite: 88, 89, 344].

| Componente | Conexión al ESP32 |
| :--- | :--- |
| **Cable USB** | [cite_start]Puerto de datos (Micro-USB / USB-C) de la placa [cite: 346] |
| **Computador** | [cite_start]Puerto USB para visualización y envío de datos [cite: 347] |

## 6. Instrucciones para el Estudiante
1.  [cite_start]**Carga:** Copia el código base en un nuevo proyecto de Arduino IDE y súbelo a la placa ESP32[cite: 348].
2.  [cite_start]**Configuración del Monitor Serial:** * Abra el Monitor Serial (`Ctrl + Shift + M`)[cite: 349].
    * [cite_start]Configure la velocidad a **115200 baudios**[cite: 47, 350].
    * [cite_start]Establezca la terminación de línea en **"Ambos NL & CR"** o **"Nueva línea"**[cite: 351].
3.  [cite_start]**Interacción:** Siga las instrucciones impresas en el monitor e ingrese sus valores en la barra de texto superior[cite: 352, 353].

## Código
```cpp 
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
```