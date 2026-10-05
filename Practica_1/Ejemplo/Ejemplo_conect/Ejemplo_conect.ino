/*
 * Monitoría: Instrumentación y Electrónica 
 * Proyecto: Control de Posición, Interrupciones e IoT (MQTT)
 * Correcciones: Visualización en vivo (0-4095) separada para Serial y Web
 */

#include <WiFi.h>
#include <PubSubClient.h>

// --- Credenciales Wi-Fi y MQTT ---
const char* ssid = "Novoa";
const char* password = "1234567890";
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// --- Configuración de Pines ---
const int PIN_BOTON = 4;      
const int PIN_LED_BOTON = 18; 
const int PIN_LED_PWM = 19;   
const int PIN_POT = 34;       

// --- Variables para la Lógica y Tiempos ---
bool estadoLedBoton = false;
unsigned long tiempoUltimoPulsoValido = 0; 
unsigned long diferenciaTiempo = 0;
int contadorInterrupciones = 0;

// Variables para el ISR (Debounce Diferido)
volatile bool posiblePresion = false;
volatile unsigned long tiempoInterrupcion = 0;

// Banderas de comunicación
bool eventoFisicoPendiente = false;
bool eventoWebPendiente = false;

// Configuración PWM (Estructura v2.x.x)
const int canalPWM = 0;
const int frecuencia = 5000;
const int resolucion = 12; // 0-4095

// --- Interrupción por Hardware (ISR) ---
void IRAM_ATTR manejadorBoton() {
  if (!posiblePresion) {
    tiempoInterrupcion = millis();
    posiblePresion = true; 
  }
}

// --- Callback MQTT (Botón de la Página Web) ---
void callbackMQTT(char* topic, byte* payload, unsigned int length) {
  String mensaje = "";
  for (int i = 0; i < length; i++) {
    mensaje += (char)payload[i];
  }
  
  if (String(topic) == "novoa/instru/btn_web" && mensaje == "PRESIONADO") {
    unsigned long tiempoActual = millis();
    diferenciaTiempo = tiempoActual - tiempoUltimoPulsoValido;
    tiempoUltimoPulsoValido = tiempoActual;
    estadoLedBoton = !estadoLedBoton;
    contadorInterrupciones++;
    eventoWebPendiente = true;
  }
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado.");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT... ");
    String clientId = "ESP32Client-Novoa-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado al broker");
      client.subscribe("novoa/instru/btn_web");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BOTON, INPUT_PULLUP);
  pinMode(PIN_LED_BOTON, OUTPUT);
  digitalWrite(PIN_LED_BOTON, LOW);

  ledcSetup(canalPWM, frecuencia, resolucion);
  ledcAttachPin(PIN_LED_PWM, canalPWM);
  
  attachInterrupt(digitalPinToInterrupt(PIN_BOTON), manejadorBoton, FALLING);
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callbackMQTT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); 

  // --- LÓGICA DE DEBOUNCE DIFERIDO ---
  if (posiblePresion) {
    if (millis() - tiempoInterrupcion > 50) {
      if (digitalRead(PIN_BOTON) == LOW) {
        unsigned long tiempoActual = millis();
        diferenciaTiempo = tiempoActual - tiempoUltimoPulsoValido;
        tiempoUltimoPulsoValido = tiempoActual;
        
        estadoLedBoton = !estadoLedBoton;
        contadorInterrupciones++;
        eventoFisicoPendiente = true;
      }
      posiblePresion = false;
    }
  }

  // --- LECTURA ADC Y ESCRITURA PWM ---
  int valorADC = analogRead(PIN_POT); 
  ledcWrite(canalPWM, valorADC); 

  // ========================================================
  // SOLUCIÓN: VISUALIZACIÓN EN VIVO DESACOPLADA
  // ========================================================

  // 1. Mostrar en Vivo en el Monitor/Plotter Serial (Cada 50 ms)
  static unsigned long ultimoPrintSerial = 0;
  if (millis() - ultimoPrintSerial > 50) {
    Serial.print("ADC:");
    Serial.println(valorADC); // Imprime el valor de 0 a 4095 constantemente
    ultimoPrintSerial = millis();
  }

  // 2. Publicar en la Página Web por MQTT (Cada 250 ms)
  static unsigned long ultimoPublishADC = 0;
  if (millis() - ultimoPublishADC > 250) { 
    client.publish("novoa/instru/adc", String(valorADC).c_str());
    ultimoPublishADC = millis();
  }

  // ========================================================

  digitalWrite(PIN_LED_BOTON, estadoLedBoton);

  // --- Impresión de Eventos ---
  if (eventoFisicoPendiente || eventoWebPendiente) {
    // Al ocurrir un evento, imprimimos una barrera visual en el Serial
    Serial.println();
    Serial.print(">>> EVENTO: ");
    Serial.print(eventoFisicoPendiente ? "Hardware (Boton)" : "Software (Web)");
    Serial.print(" | Conteo: ");
    Serial.print(contadorInterrupciones);
    Serial.print(" | Delta T: ");
    Serial.print(diferenciaTiempo);
    Serial.println(" ms <<<");
    Serial.println();

    client.publish("novoa/instru/count", String(contadorInterrupciones).c_str());
    client.publish("novoa/instru/time", String(diferenciaTiempo).c_str());
    
    eventoFisicoPendiente = false;
    eventoWebPendiente = false;
  }
}