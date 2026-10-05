/*
 * Monitoría: Instrumentación y Electrónica 
 * Versión Final: Cambio de Pin ADC a D34
 */

const int PIN_BOTON = 4;      
const int PIN_LED_BOTON = 18; 
const int PIN_LED_PWM = 19;   
const int PIN_POT = 34;       // <--- Cambiamos de 22 a 34

volatile bool estadoLedBoton = false;
volatile unsigned long tiempoUltimoPulso = 0;
volatile unsigned long diferenciaTiempo = 0;

// Configuración PWM
const int canalPWM = 0;
const int frecuencia = 5000;
const int resolucion = 12; 

void IRAM_ATTR manejadorBoton() {
  unsigned long tiempoActual = millis();
  if (tiempoActual - tiempoUltimoPulso > 200) {
    diferenciaTiempo = tiempoActual - tiempoUltimoPulso;
    tiempoUltimoPulso = tiempoActual;
    estadoLedBoton = !estadoLedBoton;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BOTON, INPUT_PULLUP);
  pinMode(PIN_LED_BOTON, OUTPUT);
  
  // El pin 34 no necesita pinMode(INPUT) ya que es solo entrada, 
  // pero lo pondremos por seguridad.
  pinMode(PIN_POT, INPUT); 

  ledcSetup(canalPWM, frecuencia, resolucion);
  ledcAttachPin(PIN_LED_PWM, canalPWM);
  
  attachInterrupt(digitalPinToInterrupt(PIN_BOTON), manejadorBoton, FALLING);
  Serial.println("Probando ADC en pin D34...");
}

void loop() {
  // Lectura directa
  int valorADC = analogRead(PIN_POT); 
  
  // Imprimir para depurar: Si sale siempre 0 o 4095, es un cable suelto
  Serial.print("Valor ADC: ");
  Serial.println(valorADC);

  ledcWrite(canalPWM, valorADC);
  digitalWrite(PIN_LED_BOTON, estadoLedBoton);

  delay(100); 
}