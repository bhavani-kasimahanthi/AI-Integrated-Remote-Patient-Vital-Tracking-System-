#include <Health_inferencing.h>//edge
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

#define REPORTING_PERIOD_MS 1000
#define ONE_WIRE_BUS 4
#define TELEGRAM_SEND_INTERVAL 30000 // 30 seconds between critical alerts


const char* ssid = "Riya";
const char* password = "123456789";

const char* telegramBotToken = "7915378297:AAFzGSVnvr0V6uxuAkgIP0_ug9FEUUxizxE";
const char* chatID = "6684705398";

WebServer server(80);


PulseOximeter pox;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


struct VitalSigns {
  float heartRate = 0.0;
  float spo2 = 0.0;
  float temperature = 0.0;
  String status = "Initializing";
  String mlConfidence = "";
} vitals;

String doctorResponse = "Awaiting doctor's response";
unsigned long lastTelegramAlert = 0;
unsigned long lastTempRequest = 0;
unsigned long tsLastReport = 0;
bool tempRequestPending = false;
bool criticalCondition = false;


const char* STATUS_LABELS[] = {"Good", "Warning", "Critical"};

// Feature extraction for Edge Impulse
int extract_features(size_t offset, size_t length, float *out_ptr) {
    float features[] = { vitals.heartRate, vitals.temperature, vitals.spo2 };
    memcpy(out_ptr, features + offset, length * sizeof(float));
    return 0;
}


void classifyWithML() {
    ei::signal_t signal;
    signal.total_length = 3;
    signal.get_data = [](size_t offset, size_t length, float *out_ptr) {
        return extract_features(offset, length, out_ptr);
    };

    ei_impulse_result_t result;
    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false);

    if (err != EI_IMPULSE_OK) {
        vitals.status = "ML Error: " + String(err);
        return;
    }

    size_t max_index = 0;
    float max_value = result.classification[0].value;
    for (size_t i = 1; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        if (result.classification[i].value > max_value) {
            max_index = i;
            max_value = result.classification[i].value;
        }
    }

 
    if (max_index < sizeof(STATUS_LABELS)/sizeof(STATUS_LABELS[0])) {
        vitals.status = String(STATUS_LABELS[max_index]) + " (" + String(max_value * 100, 1) + "%)";
        criticalCondition = (max_index == 2); // Critical is index 2
        vitals.mlConfidence = String(max_value * 100, 1) + "%";
    } else {
        vitals.status = "Unknown Status";
        vitals.mlConfidence = "N/A";
    }
}


void sendTelegramAlert() {
    if (millis() - lastTelegramAlert < TELEGRAM_SEND_INTERVAL) return;
    
    HTTPClient http;
    String url = "https://api.telegram.org/bot" + String(telegramBotToken) + "/sendMessage";
    String message = "🚨 *Critical Health Alert!* 🚨\n";
    message += "❤️ HR: " + String(vitals.heartRate) + " BPM\n";
    message += "🩸 SpO2: " + String(vitals.spo2) + "%\n";
    message += "🌡 Temp: " + String(vitals.temperature) + "°C\n";
    message += "📊 ML Confidence: " + vitals.mlConfidence + "\n";
    message += "📢 Immediate attention required!";

    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    
    String payload = "{\"chat_id\":\"" + String(chatID) + "\", ";
    payload += "\"text\":\"" + message + "\", ";
    payload += "\"parse_mode\":\"Markdown\"}";

    int httpCode = http.POST(payload);
    if (httpCode > 0) {
        Serial.println("Telegram alert sent");
        lastTelegramAlert = millis();
    } else {
        Serial.println("Telegram send failed");
    }
    http.end();
}

// Web server handlers
void handleVitalsUpdate() {
    String json = "{";
    json += "\"heartRate\":" + String(vitals.heartRate) + ",";
    json += "\"spo2\":" + String(vitals.spo2) + ",";
    json += "\"temperature\":" + String(vitals.temperature) + ",";
    json += "\"status\":\"" + vitals.status + "\",";
    json += "\"mlConfidence\":\"" + vitals.mlConfidence + "\",";
    json += "\"doctorResponse\":\"" + doctorResponse + "\"";
    json += "}";
    server.send(200, "application/json", json);
}

void handleDoctorResponse() {
    if (server.hasArg("response")) {
        doctorResponse = server.arg("response");
        // Add timestamp to response
        doctorResponse += " (Received: " + String(millis() / 1000) + "s)";
    }
    server.send(200, "text/plain", "Response recorded");
}

String generateHTML() {
    String html = R"=====(
<!DOCTYPE html>
<html>
<head>
    <title>AI Patient Monitoring</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background-color: #f5f5f5; }
        .dashboard { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }
        h1 { color: #2c3e50; text-align: center; }
        .vitals-container { display: flex; flex-wrap: wrap; justify-content: space-between; margin: 20px 0; }
        .vital-card { width: 30%; padding: 15px; margin: 5px; border-radius: 8px; text-align: center; background: #ecf0f1; }
        .status { padding: 15px; margin: 20px 0; border-radius: 8px; text-align: center; font-weight: bold; }
        .good { background: #2ecc71; color: white; }
        .warning { background: #f39c12; color: white; }
        .critical { background: #e74c3c; color: white; }
        .response-panel { margin-top: 20px; padding: 15px; background: #3498db; color: white; border-radius: 8px; }
        input[type="text"] { width: 70%; padding: 8px; }
        input[type="submit"] { padding: 8px 15px; background: #2c3e50; color: white; border: none; border-radius: 4px; }
        .ml-info { font-size: 0.9em; color: #7f8c8d; margin-top: 5px; }
    </style>
</head>
<body>
    <div class="dashboard">
        <h1>AI-Integrated Patient Monitoring</h1>
        
        <div class="vitals-container">
            <div class="vital-card">
                <h3>Heart Rate</h3>
                <p id="heartRate">--</p>
                <small>BPM</small>
            </div>
            <div class="vital-card">
                <h3>SpO2</h3>
                <p id="spo2">--</p>
                <small>%</small>
            </div>
            <div class="vital-card">
                <h3>Temperature</h3>
                <p id="temperature">--</p>
                <small>°C</small>
            </div>
        </div>
        
        <div class="status" id="statusBox">
            <h3>Health Status</h3>
            <p id="status">--</p>
            <p class="ml-info" id="mlConfidence">ML Confidence: --</p>
        </div>
        
        <div class="response-panel">
            <h3>Doctor's Response</h3>
            <p id="doctorResponse">)=====";
    html += doctorResponse;
    html += R"=====(</p>
            <form onsubmit="sendResponse(event)">
                <input type="text" id="responseInput" placeholder="Enter your medical response" required>
                <input type="submit" value="Send Response">
            </form>
        </div>
    </div>

    <script>
        function updateDashboard() {
            fetch('/updateVitals')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('heartRate').textContent = data.heartRate;
                    document.getElementById('spo2').textContent = data.spo2;
                    document.getElementById('temperature').textContent = data.temperature;
                    document.getElementById('status').textContent = data.status;
                    document.getElementById('mlConfidence').textContent = 'ML Confidence: ' + data.mlConfidence;
                    document.getElementById('doctorResponse').textContent = data.doctorResponse;
                    
                    // Update status box
                    const statusBox = document.getElementById('statusBox');
                    statusBox.className = 'status';
                    if (data.status.includes('Good')) statusBox.classList.add('good');
                    else if (data.status.includes('Warning')) statusBox.classList.add('warning');
                    else if (data.status.includes('Critical')) statusBox.classList.add('critical');
                });
        }
        
        function sendResponse(event) {
            event.preventDefault();
            const response = document.getElementById('responseInput').value;
            fetch('/response', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: 'response=' + encodeURIComponent(response)
            })
            .then(() => {
                document.getElementById('responseInput').value = '';
            });
        }
        
        setInterval(updateDashboard, 1000);
    </script>
</body>
</html>
)=====";
    return html;
}

void setup() {
    Serial.begin(115200);
    
    // Initialize I2C
    Wire.begin();
    delay(100);
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

    // Initialize sensors
    if (!pox.begin()) {
        Serial.println("MAX30100 initialization failed!");
        while(1);
    }
    pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);
    pox.setOnBeatDetectedCallback([](){
        Serial.println("Heartbeat detected!");
    });

    sensors.begin();
    sensors.setWaitForConversion(false);
    sensors.requestTemperatures();
    lastTempRequest = millis();
    tempRequestPending = true;

    // Setup web server
    server.on("/", []() {
        server.send(200, "text/html", generateHTML());
    });
    server.on("/updateVitals", handleVitalsUpdate);
    server.on("/response", HTTP_POST, handleDoctorResponse);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    
    pox.update();
    
    
    if (tempRequestPending && (millis() - lastTempRequest >= 750)) {
        vitals.temperature = sensors.getTempCByIndex(0);
        sensors.requestTemperatures();
        lastTempRequest = millis();
    }

    
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        vitals.heartRate = pox.getHeartRate();
        vitals.spo2 = pox.getSpO2();
        
        if (vitals.heartRate > 0 && vitals.spo2 > 0) {
            classifyWithML();
            
            Serial.print("HR: "); Serial.print(vitals.heartRate);
            Serial.print(" | SpO2: "); Serial.print(vitals.spo2);
            Serial.print(" | Temp: "); Serial.print(vitals.temperature);
            Serial.print(" | Status: "); Serial.println(vitals.status);
            
            // Send alert if critical
            if (criticalCondition) {
                sendTelegramAlert();
            }
        } else {
            Serial.println("Waiting for valid sensor readings...");
        }
        
        tsLastReport = millis();
    }

    // Handle web requests
    server.handleClient();
}