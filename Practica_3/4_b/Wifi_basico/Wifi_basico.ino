/*
 * PRÁCTICA DE INSTRUMENTACIÓN: COMUNICACIÓN INDUSTRIAL
 * Ejercicio 4 - Fase B: Servidor Web WiFi Local
 */

#include <WiFi.h>

// --- CREDENCIALES DE RED WIFI ---
// TODO: Los estudiantes deben poner el nombre y contraseña de su red (o compartir datos desde el celular)
const char* ssid     = "Novoa";
const char* password = "1234567890";

// Configurar el servidor web en el puerto 80 (estándar HTTP)
WiFiServer server(80);

// --- DEFINICIÓN DE PINES ---
const int pinLED = 2;             // LED integrado en la placa ESP32
const int pinPotenciometro = 34;  // Pin análogo de solo entrada

// Variable para almacenar el estado del LED
String estadoLED = "APAGADO";

void setup() {
  Serial.begin(115200);
  pinMode(pinLED, OUTPUT);
  digitalWrite(pinLED, LOW); // Asegurar que inicie apagado

  // 1. Conexión a la red WiFi
  Serial.print("Conectando a la red: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);

  // Bucle de espera hasta que se establezca la conexión
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n¡Conexión WiFi exitosa!");
  
  // 2. Iniciar el servidor web
  server.begin();
  
  // Mostrar la dirección IP asignada al ESP32 (clave para acceder desde el celular)
  Serial.print("Servidor Web iniciado. Para acceder, abre un navegador y escribe esta IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Verificar si algún cliente (navegador web) se ha conectado
  WiFiClient client = server.available();

  if (client) {
    Serial.println("¡Nuevo cliente conectado!");
    String peticionHTTP = "";
    
    // Leer los datos enviados por el cliente
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        peticionHTTP += c;
        
        // Si la petición HTTP termina (con un salto de línea doble)
        if (c == '\n' && peticionHTTP.endsWith("\r\n\r\n")) {
          
          // --- LÓGICA DE CONTROL (Análisis de la URL) ---
          // Si la petición contiene "GET /LED=ON"
          if (peticionHTTP.indexOf("GET /LED=ON") >= 0) {
            Serial.println("Comando Web: Encender LED");
            estadoLED = "ENCENDIDO";
            digitalWrite(pinLED, HIGH);
          } 
          // Si la petición contiene "GET /LED=OFF"
          else if (peticionHTTP.indexOf("GET /LED=OFF") >= 0) {
            Serial.println("Comando Web: Apagar LED");
            estadoLED = "APAGADO";
            digitalWrite(pinLED, LOW);
          }

          // --- LECTURA DEL SENSOR ---
          int valorPot = analogRead(pinPotenciometro);

          // --- CONSTRUCCIÓN DE LA PÁGINA WEB (HTML) ---
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println("Connection: close");
          client.println();
          
          // Código HTML enviado al navegador
          client.println("<!DOCTYPE html><html>");
          client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
          client.println("<title>Servidor ESP32</title>");
          // Refrescar la página automáticamente cada 3 segundos para actualizar el potenciómetro
          client.println("<meta http-equiv=\"refresh\" content=\"3\">"); 
          client.println("<style>body { font-family: Arial; text-align: center; margin: 50px;} ");
          client.println(".boton { background-color: #4CAF50; border: none; color: white; padding: 15px 32px; text-decoration: none; font-size: 20px; margin: 10px; cursor: pointer; border-radius: 10px;} ");
          client.println(".boton-off { background-color: #f44336; }</style></head>");
          client.println("<body><h1>Panel de Control ESP32</h1>");
          
          // Mostrar datos del Potenciómetro
          client.println("<h2>Lectura del Potenciometro: <span style=\"color:blue;\">" + String(valorPot) + "</span></h2>");
          
          // Mostrar estado del LED y Botones
          client.println("<h3>Estado del LED: " + estadoLED + "</h3>");
          client.println("<a href=\"/LED=ON\"><button class=\"boton\">ENCENDER LED</button></a>");
          client.println("<a href=\"/LED=OFF\"><button class=\"boton boton-off\">APAGAR LED</button></a>");
          
          client.println("</body></html>");
          
          // La respuesta HTTP termina con una línea en blanco
          client.println();
          break; // Salir del bucle while
        }
      }
    }
    // Cerrar la conexión para liberar al cliente
    client.stop();
    Serial.println("Cliente desconectado.\n");
  }
}