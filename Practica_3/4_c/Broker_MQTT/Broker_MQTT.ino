/*
 * PRÁCTICA DE INSTRUMENTACIÓN: COMUNICACIÓN INDUSTRIAL
 * ESP32 + MQTT + HTML + Potenciómetro + LED + Servo
 *
 * Funcionamiento:
 * - El potenciómetro mueve el servo proporcionalmente de 0° a 180°.
 * - La página web puede mover el servo a 0°, 90° o 180°.
 * - El servo responde a la última orden recibida:
 *      - Si se mueve el potenciómetro, manda el potenciómetro.
 *      - Si se presiona un botón en la web, manda la web.
 * - El LED se controla desde la página web.
 * - La página web muestra el valor del potenciómetro y la posición del servo.
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

// ------------------------------------------------------
// CREDENCIALES WIFI
// ------------------------------------------------------
const char* ssid = "Novoa";
const char* password = "1234567890";

// ------------------------------------------------------
// CONFIGURACIÓN MQTT
// ------------------------------------------------------
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;

// ------------------------------------------------------
// TÓPICOS MQTT
// Deben ser iguales en el HTML
// ------------------------------------------------------
const char* topic_potenciometro = "instrumentacion/grupo1/potenciometro";
const char* topic_control_led = "instrumentacion/grupo1/control";
const char* topic_servo = "instrumentacion/grupo1/servo";
const char* topic_servo_estado = "instrumentacion/grupo1/servo/estado";

// ------------------------------------------------------
// PINES
// ------------------------------------------------------
const int pinLED = 2;              // LED integrado del ESP32
const int pinPotenciometro = 34;   // Entrada analógica
const int pinServo = 18;           // Señal PWM del servo

// ------------------------------------------------------
// SERVO
// ------------------------------------------------------
Servo servoMotor;

const int anguloMinimo = 0;
const int anguloMaximo = 180;

// ------------------------------------------------------
// CLIENTES WIFI Y MQTT
// ------------------------------------------------------
WiFiClient espClient;
PubSubClient client(espClient);

// ------------------------------------------------------
// VARIABLES DE TIEMPO
// ------------------------------------------------------
unsigned long tiempoAnteriorMQTT = 0;
const long intervaloMQTT = 1000;   // Publicar cada 1 segundo

unsigned long tiempoAnteriorServo = 0;
const long intervaloServo = 30;    // Revisar potenciómetro cada 30 ms

// ------------------------------------------------------
// VARIABLES DEL POTENCIÓMETRO Y SERVO
// ------------------------------------------------------
int valorPotActual = 0;
int valorPotReferencia = 0;

int anguloServoActual = 0;
int anguloDesdePot = 0;

// Umbral para considerar que el potenciómetro realmente se movió
const int umbralMovimientoPot = 40;

bool primeraLecturaPot = true;

String ultimoOrigenServo = "Potenciómetro";

// ------------------------------------------------------
// CONEXIÓN WIFI
// ------------------------------------------------------
void setup_wifi() {
  delay(10);

  Serial.println();
  Serial.println("--- SISTEMA MQTT INICIADO ---");
  Serial.print("Conectando a la red WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado exitosamente.");
  Serial.print("IP asignada: ");
  Serial.println(WiFi.localIP());
}

// ------------------------------------------------------
// PUBLICAR ESTADO DEL SERVO
// ------------------------------------------------------
void publicarEstadoServo() {
  String payload = String(anguloServoActual);
  client.publish(topic_servo_estado, payload.c_str());
}

// ------------------------------------------------------
// PUBLICAR VALOR DEL POTENCIÓMETRO
// ------------------------------------------------------
void publicarPotenciometro() {
  String payload = String(valorPotActual);
  client.publish(topic_potenciometro, payload.c_str());
}

// ------------------------------------------------------
// CALLBACK MQTT
// Se ejecuta cuando llega un mensaje desde la página web
// ------------------------------------------------------
void callback(char* topic, byte* payload, unsigned int length) {
  String mensaje = "";

  for (int i = 0; i < length; i++) {
    mensaje += (char)payload[i];
  }

  String topicRecibido = String(topic);

  Serial.print("Mensaje recibido en [");
  Serial.print(topicRecibido);
  Serial.print("]: ");
  Serial.println(mensaje);

  // ----------------------------------------------------
  // CONTROL DEL LED DESDE LA WEB
  // ----------------------------------------------------
  if (topicRecibido == topic_control_led) {
    if (mensaje == "1") {
      digitalWrite(pinLED, HIGH);
      Serial.println("-> Acción: LED ENCENDIDO");
    }
    else if (mensaje == "0") {
      digitalWrite(pinLED, LOW);
      Serial.println("-> Acción: LED APAGADO");
    }
  }

  // ----------------------------------------------------
  // CONTROL DEL SERVO DESDE LA WEB
  // ----------------------------------------------------
  else if (topicRecibido == topic_servo) {
    int anguloWeb = mensaje.toInt();

    anguloWeb = constrain(anguloWeb, anguloMinimo, anguloMaximo);

    servoMotor.write(anguloWeb);
    anguloServoActual = anguloWeb;
    ultimoOrigenServo = "Página web";

    /*
     * Clave:
     * Tomamos como referencia la posición actual del potenciómetro.
     * Así el potenciómetro no sobrescribe inmediatamente la orden web.
     * Solo volverá a controlar el servo cuando realmente se mueva.
     */
    valorPotReferencia = analogRead(pinPotenciometro);

    Serial.print("-> Acción: Servo movido desde la web a ");
    Serial.print(anguloServoActual);
    Serial.println(" grados");

    publicarEstadoServo();
  }
}

// ------------------------------------------------------
// RECONEXIÓN MQTT
// ------------------------------------------------------
void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión al broker MQTT... ");

    String clientId = "ESP32_Lab_";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado.");

      client.subscribe(topic_control_led);
      client.subscribe(topic_servo);

      Serial.println("Suscrito a:");
      Serial.println(topic_control_led);
      Serial.println(topic_servo);

      publicarPotenciometro();
      publicarEstadoServo();

    } else {
      Serial.print("Fallo, rc=");
      Serial.print(client.state());
      Serial.println(" | Reintentando en 5 segundos...");
      delay(5000);
    }
  }
}

// ------------------------------------------------------
// SETUP
// ------------------------------------------------------
void setup() {
  Serial.begin(115200);

  pinMode(pinLED, OUTPUT);
  digitalWrite(pinLED, LOW);

  analogReadResolution(12);  // ADC de 0 a 4095

  servoMotor.setPeriodHertz(50);
  servoMotor.attach(pinServo, 500, 2400);
  servoMotor.write(0);

  anguloServoActual = 0;

  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  valorPotReferencia = analogRead(pinPotenciometro);
}

// ------------------------------------------------------
// LOOP PRINCIPAL
// ------------------------------------------------------
void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  unsigned long tiempoActual = millis();

  // ----------------------------------------------------
  // LECTURA DEL POTENCIÓMETRO
  // ----------------------------------------------------
  if (tiempoActual - tiempoAnteriorServo >= intervaloServo) {
    tiempoAnteriorServo = tiempoActual;

    valorPotActual = analogRead(pinPotenciometro);

    anguloDesdePot = map(valorPotActual, 0, 4095, anguloMinimo, anguloMaximo);
    anguloDesdePot = constrain(anguloDesdePot, anguloMinimo, anguloMaximo);

    bool potSeMovio = abs(valorPotActual - valorPotReferencia) >= umbralMovimientoPot;

    if (primeraLecturaPot || potSeMovio) {
      primeraLecturaPot = false;

      valorPotReferencia = valorPotActual;

      servoMotor.write(anguloDesdePot);
      anguloServoActual = anguloDesdePot;
      ultimoOrigenServo = "Potenciómetro";

      Serial.print("Servo controlado por potenciómetro | ADC: ");
      Serial.print(valorPotActual);
      Serial.print(" | Ángulo: ");
      Serial.print(anguloServoActual);
      Serial.println(" grados");

      publicarEstadoServo();
    }
  }

  // ----------------------------------------------------
  // PUBLICACIÓN PERIÓDICA MQTT
  // ----------------------------------------------------
  if (tiempoActual - tiempoAnteriorMQTT >= intervaloMQTT) {
    tiempoAnteriorMQTT = tiempoActual;

    publicarPotenciometro();
    publicarEstadoServo();

    Serial.println("--------------------------------");
    Serial.print("Potenciómetro ADC: ");
    Serial.println(valorPotActual);

    Serial.print("Servo actual: ");
    Serial.print(anguloServoActual);
    Serial.println(" grados");

    Serial.print("Último origen servo: ");
    Serial.println(ultimoOrigenServo);
  }
}