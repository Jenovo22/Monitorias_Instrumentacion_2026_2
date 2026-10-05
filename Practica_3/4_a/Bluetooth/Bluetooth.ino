/*
 * PRÁCTICA DE INSTRUMENTACIÓN: COMUNICACIÓN INDUSTRIAL
 * Ejercicio 4 - Fase A: Enlace Bluetooth Serial con Potenciómetro (Doble Salida)
 */

#include "BluetoothSerial.h"

// Verificar si el Bluetooth está habilitado en la configuración del ESP32
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error "El Bluetooth no está habilitado. Verifica tu configuración o placa."
#endif

// Crear una instancia del objeto Bluetooth
BluetoothSerial SerialBT;

// --- DEFINICIÓN DE PINES ---
const int pinLED = 2;            // Usamos el LED azul integrado en la placa
const int pinPotenciometro = 34; // Pin análogo para el potenciómetro

// Variables para control de tiempo (telemetría no bloqueante)
unsigned long tiempoAnterior = 0;
const long intervalo = 3000; // Enviar valor del potenciómetro cada 3 segundos

void setup() {
  // Inicializar el puerto serial convencional (USB hacia el PC)
  Serial.begin(115200);
  
  // Configuración de pines
  pinMode(pinLED, OUTPUT);
  digitalWrite(pinLED, LOW);

  // Inicializar el Bluetooth con el nombre que aparecerá en el celular
  SerialBT.begin("ESP32_Grupo1"); // Sugiere a los alumnos cambiar este nombre
  
  // Mensajes iniciales en el Monitor Serial del PC
  Serial.println("--- SISTEMA INICIADO ---");
  Serial.println("El dispositivo Bluetooth esta listo.");
  Serial.println("Esperando emparejamiento con el celular...");
}

void loop() {
  // 1. RECEPCIÓN DE COMANDOS DESDE EL CELULAR
  if (SerialBT.available()) {
    char comando = SerialBT.read();
    
    // Ignorar saltos de línea del terminal
    if (comando != '\n' && comando != '\r') {
      Serial.print("Comando recibido vía BT: ");
      Serial.println(comando);
    }

    // Lógica de control del LED e interrogación manual
    if (comando == '1') {
      digitalWrite(pinLED, HIGH);
      SerialBT.println("Confirmacion: LED ENCENDIDO");
      Serial.println("-> Accion Local: El celular encendio el LED."); // Reflejo en PC
    } 
    else if (comando == '0') {
      digitalWrite(pinLED, LOW);
      SerialBT.println("Confirmacion: LED APAGADO");
      Serial.println("-> Accion Local: El celular apago el LED."); // Reflejo en PC
    }
    else if (comando == 'P' || comando == 'p') {
      // Si el usuario pide el valor del potenciómetro manualmente
      int valorPot = analogRead(pinPotenciometro);
      SerialBT.print("Lectura solicitada (ADC): ");
      SerialBT.println(valorPot);
      
      Serial.print("-> El celular solicito lectura. Valor enviado: ");
      Serial.println(valorPot); // Reflejo en PC
    }
  }

  // 2. ENVÍO PERIÓDICO DE DATOS (Telemetría)
  unsigned long tiempoActual = millis();
  if (tiempoActual - tiempoAnterior >= intervalo) {
    tiempoAnterior = tiempoActual;
    
    // Leer el convertidor análogo a digital (0 a 4095)
    int valorPot = analogRead(pinPotenciometro);
    
    // Enviar el dato por Bluetooth al celular
    SerialBT.print("Telemetria BT - Potenciometro: ");
    SerialBT.println(valorPot);
    
    // Enviar el mismo dato por el Monitor Serial al computador
    Serial.print("Telemetria PC - Potenciometro: ");
    Serial.println(valorPot);
  }
}