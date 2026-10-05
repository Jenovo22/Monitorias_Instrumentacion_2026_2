/*
 * PRÁCTICA DE INSTRUMENTACIÓN: INTERFAZ SÍNCRONA SPI
 * Ejercicio 3: Sensor BMP280 de Presión y Temperatura
 */

#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

// --- DEFINICIÓN DE PINES SPI (Bus VSPI estándar en ESP32) ---
// Es vital definir estos pines explícitamente para evitar errores de conexión
#define BMP_SCK  18  // Pin de Reloj (Serial Clock)
#define BMP_MISO 19  // Master Input Slave Output (Datos hacia el ESP32)
#define BMP_MOSI 23  // Master Output Slave Input (Datos hacia el sensor)
#define BMP_CS   5   // Chip Select (Selección del dispositivo)

// Crear la instancia del sensor usando la interfaz SPI por hardware
Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO, BMP_SCK);

void setup() {
  Serial.begin(115200);
  
  // Pausa para asegurar que el monitor serial inicie
  delay(1000);
  Serial.println(F("\n--- INICIANDO SENSOR BMP280 (SPI) ---"));

  // Inicializar el sensor. Si no se encuentra, el programa se detiene.
  if (!bmp.begin()) {
    Serial.println(F("Error: No se encontró un sensor BMP280 válido."));
    Serial.println(F("Verifique las conexiones de los pines SCK, MISO, MOSI y CS."));
    while (1) delay(10); // Bucle infinito de bloqueo
  }

  // Configuración de los parámetros de muestreo del sensor para mayor estabilidad
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Modo de operación */
                  Adafruit_BMP280::SAMPLING_X2,     /* Oversampling de Temp */
                  Adafruit_BMP280::SAMPLING_X16,    /* Oversampling de Presión */
                  Adafruit_BMP280::FILTER_X16,      /* Filtrado de ruido */
                  Adafruit_BMP280::STANDBY_MS_500); /* Tiempo de espera (500ms) */
                  
  Serial.println(F("Sensor configurado correctamente. Iniciando lecturas...\n"));
}

void loop() {
  // 1. Lectura de Temperatura (en grados Celsius)
  float temperatura = bmp.readTemperature();

  // 2. Lectura de Presión Barométrica (La librería devuelve Pascales, se divide por 100 para hPa)
  float presion = bmp.readPressure() / 100.0F;

  // 3. Lectura de Altitud aproximada (Requiere la presión a nivel del mar local, por defecto 1013.25)
  float altitud = bmp.readAltitude(1013.25);

  // --- PRESENTACIÓN DE DATOS (Monitor Serial) ---
  Serial.print(F("Temperatura: "));
  Serial.print(temperatura);
  Serial.println(F(" *C"));

  Serial.print(F("Presion Barometrica: "));
  Serial.print(presion);
  Serial.println(F(" hPa"));

  Serial.print(F("Altitud Aprox: "));
  Serial.print(altitud);
  Serial.println(F(" m"));

  Serial.println(F("------------------------------------"));

  // Pausa de 2 segundos entre lecturas
  delay(2000); 
}