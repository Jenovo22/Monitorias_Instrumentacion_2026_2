#include <WiFi.h>
#include <WebServer.h>

// ====== CONFIGURACIÓN DE PINES PWM ======
#define RED_PIN   25   // Puedes usar: 25, 26, 27, 14, 32, 33
#define GREEN_PIN 26
#define BLUE_PIN  27

// ====== CONFIGURACIÓN DE WIFI ======
const char* ssid = "TuSSID";
const char* password = "TuPassword";

// ====== SERVIDOR WEB ======
WebServer server(80);

// ====== CANALES PWM ======
const int freq = 5000;   // Frecuencia PWM (Hz)
const int resolution = 8; // Resolución (0-255)
const int chR = 0;
const int chG = 1;
const int chB = 2;

// ====== FUNCIÓN PARA CONFIGURAR COLOR ======
void setColor(uint8_t r, uint8_t g, uint8_t b) {
  // Si tu LED es de ÁNODO COMÚN, usa: 255 - valor
  ledcWrite(chR, r);
  ledcWrite(chG, g);
  ledcWrite(chB, b);

  Serial.printf("Color -> R:%d  G:%d  B:%d\n", r, g, b);
}

// ====== PÁGINA WEB ======
String htmlPage() {
  return R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Control LED RGB ESP32 PWM</title>
<style>
  body {
    background: radial-gradient(circle at top, #1a237e, #000);
    color: white;
    font-family: 'Segoe UI', sans-serif;
    text-align: center;
    margin: 0;
    padding: 0;
  }
  h1 { margin-top: 30px; font-size: 2.2em; }
  .color-box {
    background: white;
    border-radius: 12px;
    display: inline-block;
    padding: 20px;
    margin-top: 40px;
  }
  input[type='color'] {
    width: 160px;
    height: 160px;
    border: none;
    border-radius: 10px;
    cursor: pointer;
  }
  button {
    margin-top: 20px;
    padding: 10px 25px;
    border: none;
    background: #00c853;
    color: white;
    font-size: 1.2em;
    border-radius: 8px;
    cursor: pointer;
  }
  button:hover { background: #00a94f; }
  #rgbValue { margin-top: 15px; font-size: 1.1em; }
  footer {
    margin-top: 50px;
    opacity: 0.8;
    font-size: 0.9em;
  }
</style>
</head>
<body>
  <h1>Control LED RGB (PWM) - ESP32</h1>
  <div class="color-box">
    <input type="color" id="colorInput" value="#00ff00">
    <div id="rgbValue">R:0 G:255 B:0</div>
    <button onclick="sendColor()">Enviar color</button>
  </div>
  <footer>Desarrollado para prácticas IoT con ESP32 ⚡</footer>

<script>
function sendColor() {
  const color = document.getElementById('colorInput').value;
  const r = parseInt(color.substr(1,2),16);
  const g = parseInt(color.substr(3,2),16);
  const b = parseInt(color.substr(5,2),16);
  document.getElementById('rgbValue').innerText = `R:${r} G:${g} B:${b}`;
  fetch(`/set?r=${r}&g=${g}&b=${b}`);
}
</script>
</body>
</html>
)rawliteral";
}

// ====== MANEJADORES DEL SERVIDOR ======
void handleRoot() {
  server.send(200, "text/html", htmlPage());
}

void handleSet() {
  if (server.hasArg("r") && server.hasArg("g") && server.hasArg("b")) {
    int r = server.arg("r").toInt();
    int g = server.arg("g").toInt();
    int b = server.arg("b").toInt();
    setColor(r, g, b);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Error: faltan parámetros");
  }
}

// ====== SETUP ======
void setup() {
  Serial.begin(115200);

  // Configurar canales PWM
  ledcSetup(chR, freq, resolution);
  ledcSetup(chG, freq, resolution);
  ledcSetup(chB, freq, resolution);

  ledcAttachPin(RED_PIN, chR);
  ledcAttachPin(GREEN_PIN, chG);
  ledcAttachPin(BLUE_PIN, chB);

  // Apagar LED al inicio
  setColor(0, 0, 0);

  // Conexión WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ Conectado a WiFi");
  Serial.print("🌐 IP del servidor: ");
  Serial.println(WiFi.localIP());

  // Configurar servidor
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.begin();
  Serial.println("Servidor web iniciado 🚀");
}

// ====== LOOP ======
void loop() {
  server.handleClient();
}
