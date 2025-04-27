#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <WebServer.h>

// WiFi credentials
const char* ssid = "Riya";         // Replace with your WiFi SSID
const char* password = "123456789"; // Replace with your WiFi Password

WebServer server(80); // Web server on port 80

// MAX30100 setup
#define REPORTING_PERIOD_MS 1000
PulseOximeter pox;
uint32_t tsLastReport = 0;

// DS18B20 setup
#define ONE_WIRE_BUS 4 // GPIO pin where DS18B20 is connected
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
unsigned long lastTempRequest = 0;
float temperature = 0.0;
bool tempRequestPending = false;

// Store vitals globally
float heartRate = 0.0, spo2 = 0.0;
String status = "Unknown";
String doctorResponse = "No response yet";  // Default response

// Function to classify vitals
String classifyVitals(float heartRate, float spo2, float temperature) {
    if (heartRate >= 60 && heartRate <= 100 && spo2 >= 90 && temperature >= 33.0 && temperature <= 37.5) {
        return "Good";
    }
    else if ((heartRate < 60 || heartRate > 100) || (spo2 < 95 && spo2 >= 89) || (temperature < 35.0 || temperature > 38.5)) {
        return "Bad";
    }
    else if (spo2 < 89 || temperature > 39.0) {
        return "Critical";
    }
    return "Unknown";
}

// Handle AJAX request to get vitals
void handleVitalsUpdate() {
    status = classifyVitals(heartRate, spo2, temperature);  // Ensure status updates before sending response

    String json = "{";
    json += "\"heartRate\":" + String(heartRate) + ",";
    json += "\"spo2\":" + String(spo2) + ",";
    json += "\"temperature\":" + String(temperature) + ",";
    json += "\"status\":\"" + status + "\",";
    json += "\"doctorResponse\":\"" + doctorResponse + "\"";
    json += "}";
    server.send(200, "application/json", json);
}

// Handle doctor's response
void handleDoctorResponse() {
    if (server.hasArg("response")) {
        doctorResponse = server.arg("response");  // Store doctor's response
    }
    server.send(200, "text/plain", "Response received");
}

// Webpage HTML (With AJAX for Live Updates)
String generateHTML() {
    String html = "<html><head><title>Patient Vitals</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body { font-family: Arial; text-align: center; background-color: #f2f2f2; }";
    html += "h2 { color: #333; } .box { padding: 20px; border-radius: 10px; background: white; display: inline-block; margin: 10px; }";
    html += ".good { color: green; } .bad { color: orange; } .critical { color: red; }</style></head><body>";
    html += "<h2>Remote Patient Monitoring</h2>";

    // Vital Signs (Will be updated by AJAX)
    html += "<div class='box'><h3>Heart Rate: <span id='heartRate'>Loading...</span> BPM</h3></div>";
    html += "<div class='box'><h3>SpO2: <span id='spo2'>Loading...</span> %</h3></div>";
    html += "<div class='box'><h3>Temperature: <span id='temperature'>Loading...</span> °C</h3></div>";
    
    // Status
    html += "<div class='box' id='statusBox'><h3>Status: <span id='status'>Loading...</span></h3></div>";

    // Doctor's response section
    html += "<h3>Doctor's Response:</h3>";
    html += "<div class='box'><h3 id='doctorResponse'>Loading...</h3></div>";

    // Input form for doctor's response
    html += "<form id='responseForm' onsubmit='sendResponse(event)'>";
    html += "<input type='text' id='responseInput' name='response' placeholder='Enter response' required>";
    html += "<input type='submit' value='Submit'>";
    html += "</form>";

    // AJAX script for live updates
    html += "<script>";
    html += "function updateVitals() {";
    html += "fetch('/updateVitals').then(response => response.json()).then(data => {";
    html += "document.getElementById('heartRate').innerText = data.heartRate;";
    html += "document.getElementById('spo2').innerText = data.spo2;";
    html += "document.getElementById('temperature').innerText = data.temperature;";
    html += "document.getElementById('status').innerText = data.status;";
    html += "document.getElementById('statusBox').className = data.status.toLowerCase();";
    html += "document.getElementById('doctorResponse').innerText = data.doctorResponse;";
    html += "});} setInterval(updateVitals, 3000);"; // Fetch new data every 3 seconds

    // AJAX for doctor response
    html += "function sendResponse(event) {";
    html += "event.preventDefault();";
    html += "let response = document.getElementById('responseInput').value;";
    html += "fetch('/response', { method: 'POST', headers: {'Content-Type': 'application/x-www-form-urlencoded'}, body: 'response=' + encodeURIComponent(response) })";
    html += ".then(() => { document.getElementById('doctorResponse').innerText = response; document.getElementById('responseInput').value = ''; });";
    html += "}";
    html += "</script>";

    html += "</body></html>";
    return html;
}

// Web server handle function
void handleRoot() {
    server.send(200, "text/html", generateHTML());
}

void setup() {
    Serial.begin(115200);

    // WiFi connection
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected! IP address: " + WiFi.localIP().toString());

    // Initialize MAX30100
    if (!pox.begin()) {
        Serial.println("MAX30100 initialization failed!");
        while (1);
    }
    pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);

    // Initialize DS18B20
    sensors.begin();
    sensors.setWaitForConversion(false);
    sensors.requestTemperatures();
    lastTempRequest = millis();
    tempRequestPending = true;

    // Start Web Server
    server.on("/", handleRoot);
    server.on("/updateVitals", handleVitalsUpdate);
    server.on("/response", HTTP_POST, handleDoctorResponse);
    server.begin();
    Serial.println("Web server started.");
}

void loop() {
    // Update MAX30100
    pox.update();
    
    // Handle DS18B20 temperature reading
    if (tempRequestPending && (millis() - lastTempRequest >= 750)) {
        temperature = sensors.getTempCByIndex(0);
        sensors.requestTemperatures();
        lastTempRequest = millis();
    }

    // Report MAX30100 data periodically
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        heartRate = pox.getHeartRate();
        spo2 = pox.getSpO2();
        status = classifyVitals(heartRate, spo2, temperature);  // Update status here!
        tsLastReport = millis();
    }

    // Handle web server requests
    server.handleClient();
}
