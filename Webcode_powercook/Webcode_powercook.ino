#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// Replace with your network credentials
const char* ssid = "wifi002";
const char* password = "12345678";

// Pin Definitions
const int lm35Pin = 32; // Analog pin for LM35 temperature sensor (GPIO 32)
const int voltageSensorPin = 35; // Analog pin for Voltage sensor (GPIO 35)

// Voltage Sensor Calibration
const float voltageReference = 3.3; // ESP32 reference voltage
const float maxSensorVoltage = 25.0; // Maximum voltage the sensor can read
const float sensorMaxOutput = 3.8; // Maximum output voltage of the sensor

// Create an instance of the server
AsyncWebServer server(80);

// Function to get temperature in Celsius
float getTemperature() {
  int lm35Reading = analogRead(lm35Pin);
  float temperature = (lm35Reading / 4095.0) * voltageReference; // Convert analog reading to voltage
  temperature = temperature * 100.0; // Convert voltage to temperature in Celsius
  return temperature;
}

// Function to get voltage
float getVoltage() {
  int voltageReading = analogRead(voltageSensorPin);
  float voltage = (voltageReading / 4095.0) * voltageReference; // Convert analog reading to voltage
  voltage = voltage * (maxSensorVoltage / sensorMaxOutput); // Convert to actual input voltage
  return fabs(voltage); // Ensure positive value
}

// Function to generate HTML content dynamically
String htmlContent() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Phoenix</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Cinzel:wght@600&display=swap');
        body {
            margin: 0;
            padding: 0;
            font-family: 'Poppins', sans-serif;
            background: linear-gradient(135deg, #cfe5ed, #ffffff);
            height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
        }
        .container {
            display: flex;
            flex-direction: column;
            align-items: center;
            width: 90%;
            max-width: 1200px;
        }
        header {
            text-align: center;
            margin-bottom: 30px;
            position: relative;
            width: 100%;
        }
        .header-icons {
            display: flex;
            justify-content: flex-end;
            align-items: center;
            position: absolute;
            top: 0;
            right: 20px;
            padding: 10px;
        }
        .settings-icon, .profile-icon {
            font-size: 1.5em;
            color: #444;
            cursor: pointer;
            transition: color 0.3s ease;
            margin-left: 20px;
        }
        .logo {
            width: 150px;
            height: auto;
            position: absolute;
            left: 50%;
            transform: translateX(-50%);
            top: 0;
        }
        h1 {
            font-family: 'Cinzel', serif;
            font-size: 4em;
            color: #333;
            margin: 0;
            text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.1);
        }
        .grid-container {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
            width: 100%;
        }
        .box {
            background: linear-gradient(145deg, #c0ebf1, #8fc7e7);
            border-radius: 15px;
            box-shadow: 0px 4px 15px rgba(0, 0, 0, 0.1);
            padding: 30px;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            transition: transform 0.3s ease, background 0.3s ease;
        }
        .box:hover {
            transform: translateY(-5px);
            background: linear-gradient(145deg, #f0f0f0, #e0e0e0);
        }
        h2 {
            color: #333;
            margin-bottom: 20px;
            font-size: 1.5em;
            text-shadow: 1px 1px 2px rgba(0, 0, 0, 0.1);
        }
        .value-box {
            font-size: 3em;
            color: #333;
            background: linear-gradient(145deg, #e0e0e0, #ffffff);
            padding: 20px;
            border-radius: 10px;
            width: 60%;
            text-align: center;
            box-shadow: inset 0px 2px 5px rgba(0, 0, 0, 0.1);
            transition: background 0.3s ease;
        }
        footer {
            margin-top: 125px;
            text-align: center;
            font-size: 0.95em;
            color: #555;
            padding: 10px;
            width: 100%;
            background: #f9f9f9;
            box-shadow: 0 -2px 10px rgba(0, 0, 0, 0.1);
            border-radius: 10px 10px 0 0;
        }
        footer p {
            margin: 0;
        }
    </style>
    <script>
        function fetchData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('volts').innerText = data.voltage + ' V';
                    document.getElementById('temperature').innerText = data.temperature + ' °C';
                });
        }
        setInterval(fetchData, 2000); // Update every 2 seconds
        window.onload = fetchData; // Fetch data when the page loads
    </script>
</head>
<body>
    <div class="container">
        <header>
            <div class="header-icons">
                <i class="fas fa-cog settings-icon"></i>
                <i class="fas fa-user profile-icon"></i>
            </div>
            <h1>PHOENIX</h1>
        </header>
        <div class="grid-container">
            <div class="box battery">
                <h2>Voltage Produced</h2>
                <div id="volts" class="value-box">-- V</div>
            </div>
            <div class="box heat">
                <h2>Temperature Emitted</h2>
                <div id="temperature" class="value-box">-- °C</div>
            </div>
        </div>
        <footer>
            <p>"Cutting carbon emissions is essential for sustainable development, ensuring that economic growth and environmental protection for future generations."</p>
        </footer>
    </div>
</body>
</html>
)rawliteral";
  return html;
}

void handleData(AsyncWebServerRequest *request) {
  String json = "{\"voltage\":" + String(getVoltage(), 2) + 
                 ",\"temperature\":" + String(getTemperature(), 2) + "}";
  request->send(200, "application/json", json);
}

void setup() {
  // Set ADC resolution
  analogReadResolution(12); // ESP32 uses a 12-bit ADC by default, 0-4095 range

  // Begin Serial communication
  Serial.begin(115200);
  delay(1000); // Allow time for Serial Monitor to open

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");

  // Print the IP address
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // Print the assigned IP address

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", htmlContent());
  });

  // Route for JSON data
  server.on("/data", HTTP_GET, handleData);

  // Start the server
  server.begin();
}

void loop() {
  // Nothing to do here, server handles requests asynchronously
}
