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

  delay(150);
}
