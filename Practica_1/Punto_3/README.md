
# PUNTO 3 ADC: Detección de dirección de una fuente de luz con dos LDRs en ESP32

## Descripción general

Este proyecto utiliza **dos fotorresistencias (LDRs)** conectadas al **ESP32 DevKit** para detectar la **dirección de un foco de luz**.  
La idea es comparar los niveles de iluminación recibidos por cada LDR y determinar **hacia qué lado hay más luz** (izquierda o derecha).  
Adicionalmente, se activa un **LED indicador** led del LDR izquierda o led del LDR derecho para indicar donde es la concentracion mayor de luz.

---

## Componentes requeridos

| Componente | Cantidad | Descripción |
|-------------|-----------|-------------|
| ESP32 DevKit | 1 | Microcontrolador principal |
| LDR (fotorresistencia) | 2 | Sensores de luz analógicos |
| Resistencia fija 10 kΩ | 2 | Para formar divisores de tensión con los LDR |
| LED | 2 | Indicador visual de luz fuerte |
| Resistencia 220 Ω | 1 | Limitadora de corriente para el LED |
| Protoboard y cables | - | Conexiones |

---

##  Conexión del circuito

Cada LDR forma un **divisor de tensión** junto a una resistencia de **10 kΩ**.  
El voltaje medido en el punto medio del divisor se conecta a un **pin analógico** del ESP32.

```
(Vcc = 3.3V)
         LDR1           LDR2
          │               │
         ┌┴┐             ┌┴┐
         │ │             │ │
         │ │             │ │
         └┬┘             └┬┘
          │               │
          ├── A0 (GPIO 32)│── A1 (GPIO 33)
          [10kΩ]           [10kΩ]
          │               │
         GND             GND

LED → GPIO 25 (a través de 220Ω)
```

---

## 🧮 Principio de funcionamiento

El valor de tensión en el punto de medición depende de la **resistencia variable del LDR**:

$V_{out} = V_{cc} \times \frac{R_{fija}}{R_{LDR} + R_{fija}}$

Cuando hay **más luz**, la resistencia del LDR **disminuye**, haciendo que el voltaje de salida **aumente**.

---

## Lógica del programa

1. Se leen los valores analógicos de los dos LDR (`ldrLeft` y `ldrRight`).
2. Se escalan para obtener una estimación de resistencia del LDR:
   \[
   R_{LDR} = R_{fija} \times \left( \frac{V_{cc} - V_{LDR}}{V_{LDR}} \right)
   \]
3. Se calcula la **diferencia entre ambos LDRs**:
   - Si `LDR1 > LDR2` → la luz viene del **lado izquierdo**.  
   - Si `LDR2 > LDR1` → la luz viene del **lado derecho**.  
4. Se enciende un **LED de alarma** si cualquiera de los dos LDR supera el umbral definido (`THRESHOLD`).

---

## Código del proyecto

```cpp
// Detección de dirección de luz con 2 LDRs (ESP32)
// Lectura, suavizado por media móvil y estimación angular aproximada.

#define LDR_LEFT_PIN 35
#define LDR_RIGHT_PIN 34

#define LED_LEFT_PIN 25
#define LED_RIGHT_PIN 26

// Parámetros físicos / calibración
#define R_FIJA 10000.0        // valor de la resistencia fija en ohmios (uso informativo)
#define MAX_ANGLE 60.0        // grados: mitad del ángulo que esperas que cubra el arreglo (ajusta en calibración)

// Suavizado
#define N_SMOOTH 8            // número de muestras para promedio móvil (potencia de suavidad)

// Umbral mínimo de luz para considerar "señal válida"
#define MIN_ADC_SUM 50        // si suma de ADCs < esto, se considera muy poca luz

// ---- variables ----
float bufferLeft[N_SMOOTH];
float bufferRight[N_SMOOTH];
int idxSmooth = 0;
bool filled = false;

void setup() {
  Serial.begin(115200);
  pinMode(LDR_LEFT_PIN, INPUT);
  pinMode(LDR_RIGHT_PIN, INPUT);

  pinMode(LED_LEFT_PIN, OUTPUT);
  pinMode(LED_RIGHT_PIN, OUTPUT);

  // init buffers
  for (int i = 0; i < N_SMOOTH; ++i) { bufferLeft[i] = 0.0; bufferRight[i] = 0.0; }

  Serial.println("=== Direccion de luz con 2 LDRs ===");
  Serial.println("Ajusta MAX_ANGLE en codigo tras calibracion.");
  delay(500);
}

float readSmooth(int pin, float *buf) {
  int adc = analogRead(pin);           // 0..4095
  buf[idxSmooth] = (float)adc;
  // promedio
  float sum = 0;
  int count = filled ? N_SMOOTH : (idxSmooth + 1);
  for (int i = 0; i < count; ++i) sum += buf[i];
  return sum / (float)count;
}

void loop() {
  float avgLeft = readSmooth(LDR_LEFT_PIN, bufferLeft);
  float avgRight = readSmooth(LDR_RIGHT_PIN, bufferRight);

  idxSmooth++;
  if (idxSmooth >= N_SMOOTH) { idxSmooth = 0; filled = true; }

  float sum = avgLeft + avgRight;

  // Si hay muy poca luz, no estimamos
  if (sum < MIN_ADC_SUM) {
    Serial.println("Señal muy baja -> poca luz. No se estima direccion.");
    digitalWrite(LED_LEFT_PIN, LOW);
    digitalWrite(LED_RIGHT_PIN, LOW);
    delay(200);
    return;
  }

  // proporción del lado derecho (0..1)
  float r = avgRight / sum;

  // convertir a ángulo aproximado: (-MAX_ANGLE .. +MAX_ANGLE)
  float angle = (r - 0.5f) * 2.0f * MAX_ANGLE;

  // indicar lado con LED (umbral simple)
  float SIDE_THRESHOLD = 0.55f;  // >0.55 -> derecha, <0.45 -> izquierda, entre -> centro
  if (r > SIDE_THRESHOLD) {
    digitalWrite(LED_RIGHT_PIN, HIGH);
    digitalWrite(LED_LEFT_PIN, LOW);
  } else if (r < (1.0f - SIDE_THRESHOLD)) {
    digitalWrite(LED_LEFT_PIN, HIGH);
    digitalWrite(LED_RIGHT_PIN, LOW);
  } else {
    digitalWrite(LED_LEFT_PIN, LOW);
    digitalWrite(LED_RIGHT_PIN, LOW);
  }

  // salida por serial
  Serial.print("ADC_L: "); Serial.print(avgLeft, 1);
  Serial.print("\tADC_R: "); Serial.print(avgRight, 1);
  Serial.print("\t r: "); Serial.print(r, 3);
  Serial.print("\t Angle_est (deg): "); Serial.print(angle, 1);
  Serial.println();

  delay(300);
}

```

---

##  Ajustes recomendados

- Puedes modificar la **resistencia fija (R_FIXED)** para ajustar la sensibilidad.  
  - Mayor resistencia → más sensibilidad a bajas luces.  
  - Menor resistencia → más sensibilidad a luces intensas.
- El umbral `THRESHOLD` debe calibrarse según la iluminación del ambiente.
- Si las lecturas son muy bajas, puedes aumentar el valor del divisor a **10 kΩ**.

---

##  Resultados esperados

Por el monitor serial se imprimen los valores de resistencia de cada LDR y la dirección estimada de la fuente de luz.  
El LED se encenderá cuando la intensidad lumínica general sea alta.

Ejemplo de salida:
```
LDR Izq: 1250 Ω | LDR Der: 3400 Ω | → Luz desde la IZQUIERDA
```


---

## 👨‍💻 Autor
**Juan Esteban**  
Proyecto educativo de medición de luz y control con ESP32.  
Diseñado para el curso de instrumentacion electronica uso en laboratorios de sistemas embebidos.

