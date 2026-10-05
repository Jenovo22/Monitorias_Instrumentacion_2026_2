# PUNTO 3 SPI: Obtención de presión barométrica y temperatura mediante sensor BMP280

## Objetivo
Entender y manejar la interfaz de comunicación serial síncrona **SPI (Serial Peripheral Interface)** para la adquisición de datos climáticos de alta precisión mediante un sensor barométrico.

---

## Descripción general
En este ejercicio se implementa la comunicación entre el ESP32 y el sensor **BMP280** utilizando el bus **VSPI**. El sistema permite obtener lecturas de temperatura, presión atmosférica y una estimación de la altitud, visualizando la información en tiempo real a través del **Monitor Serial**.

---

## Componentes requeridos

| Componente | Cantidad | Descripción |
|-------------|-----------|-------------|
| **ESP32 DevKit** | 1 | Microcontrolador con bus VSPI disponible |
| **Sensor BMP280** | 1 | Sensor de presión y temperatura con interfaz SPI |
| **Cable USB** | 1 | Comunicación y alimentación desde el PC |
| **PC con Arduino IDE** | 1 | Entorno de desarrollo y visualización de datos |

---

## Esquema de conexión

Para este ejercicio se utiliza el bus **VSPI** estándar del ESP32. Es fundamental asegurar que el sensor se alimente con **3.3V**, ya que voltajes superiores pueden dañarlo.

| Pin Sensor (BMP280) | Pin ESP32 (GPIO) | Función SPI |
|---------------------|------------------|-------------|
| **VCC** | 3.3V | Alimentación (Voltaje de referencia) |
| **GND** | GND | Tierra |
| **SCL** | **18** | SCK (Serial Clock) |
| **SDO** | **19** | MISO (Master Input Slave Output) |
| **SDA** | **23** | MOSI (Master Output Slave Input) |
| **CSB** | **5** | CS (Chip Select) |

---

## Lógica del programa

1. **Configuración Inicial:** Se inicializa la comunicación serial a **115200 baudios** y se verifica la conexión física del sensor mediante el bus SPI.

2. **Ajuste de Muestreo:** El sensor se configura en **Modo Normal** con técnicas de *oversampling* y filtrado interno para reducir el ruido en las lecturas de presión.

3. **Adquisición:** En cada ciclo del programa (cada 2 segundos), se extraen los valores de temperatura en °C y presión en hPa.

4. **Cálculo de Altitud:** Se utiliza la presión barométrica actual frente a la presión estándar a nivel del mar (1013.25 hPa) para estimar la altitud en metros.

5. **Visualización:** Se formatea y envía la información al monitor serial para su análisis.

---

## Ajustes y recomendaciones

- **Librerías Obligatorias:** Se debe instalar la librería `Adafruit_BMP280` y su dependencia `Adafruit Unified Sensor` desde el gestor de librerías del IDE.

- **Protocolo Síncrono:** A diferencia de I2C, la interfaz SPI utiliza cuatro hilos de datos y requiere un pin de selección de chip (**CS**) para habilitar la comunicación.

- **Pines VSPI:** Se recomienda usar los pines definidos en la tabla (18, 19, 23 y 5) por ser la configuración de hardware nativa del bus VSPI del ESP32.

---

## Código del programa

```cpp
/*
 * PRÁCTICA DE INSTRUMENTACIÓN: INTERFAZ SÍNCRONA SPI
 * Ejercicio 3: Sensor BMP280 de Presión y Temperatura
 */

#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

// --- DEFINICIÓN DE PINES SPI (Bus VSPI estándar en ESP32) ---
#define BMP_SCK  18
#define BMP_MISO 19
#define BMP_MOSI 23
#define BMP_CS   5

Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO, BMP_SCK);

void setup() {
  Serial.begin(115200);

  delay(1000);
  Serial.println(F("\\n--- INICIANDO SENSOR BMP280 (SPI) ---"));

  if (!bmp.begin()) {
    Serial.println(F("Error: No se encontró un sensor BMP280 válido."));
    Serial.println(F("Verifique las conexiones de los pines SCK, MISO, MOSI y CS."));
    while (1) delay(10);
  }

  bmp.setSampling(
      Adafruit_BMP280::MODE_NORMAL,
      Adafruit_BMP280::SAMPLING_X2,
      Adafruit_BMP280::SAMPLING_X16,
      Adafruit_BMP280::FILTER_X16,
      Adafruit_BMP280::STANDBY_MS_500
  );

  Serial.println(F("Sensor configurado correctamente. Iniciando lecturas...\\n"));
}

void loop() {
  float temperatura = bmp.readTemperature();
  float presion = bmp.readPressure() / 100.0F;
  float altitud = bmp.readAltitude(1013.25);

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

  delay(2000);
}
```

---

## Resultados esperados

```text
--- INICIANDO SENSOR BMP280 (SPI) ---
Sensor configurado correctamente. Iniciando lecturas...

Temperatura: 24.50 *C
Presion Barometrica: 1012.35 hPa
Altitud Aprox: 120.45 m
------------------------------------
```

---

## 👨‍💻 Autor

**Jerónimo Novoa Giraldo**

Proyecto de práctica con comunicación SPI e instrumentación climática usando ESP32 + BMP280, desarrollado para el curso de Instrumentación Electrónica.