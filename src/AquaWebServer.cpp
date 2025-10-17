#include "AquaWebServer.h"
#include "SystemMonitor.h"

AquaWebServer::AquaWebServer() : server(WEB_SERVER_PORT), sensorController(nullptr), calibrationManager(nullptr) {}

void AquaWebServer::begin(SensorController* sensors, CalibrationManager* calibration) {
  sensorController = sensors;
  calibrationManager = calibration;
  setupRoutes();
  server.begin();
  
  Serial.println();
  Serial.println("Web Server Started!");
  Serial.println("+---------------------------------------+");
  Serial.printf("| Dashboard: http://%-19s |\n", WiFi.localIP().toString().c_str());
  Serial.printf("| API: http://%-19s/api/ |\n", WiFi.localIP().toString().c_str());
  Serial.println("+---------------------------------------+");
}

void AquaWebServer::setSensorController(SensorController* sensors) {
  sensorController = sensors;
}

void AquaWebServer::setCalibrationManager(CalibrationManager* calibration) {
  calibrationManager = calibration;
}

void AquaWebServer::setupRoutes() {
  // Serve the main dashboard page
  server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
    handleRoot(request);
  });

  // API endpoint for all sensor data
  server.on("/api/sensors", HTTP_GET, [this](AsyncWebServerRequest *request){
    handleApiSensors(request);
  });

  // API endpoint for temperature sensors only
  server.on("/api/temperature", HTTP_GET, [this](AsyncWebServerRequest *request){
    handleApiTemperature(request);
  });

  // API endpoint for pH sensors only
  server.on("/api/ph", HTTP_GET, [this](AsyncWebServerRequest *request){
    handleApiPH(request);
  });

  // API endpoint for TDS sensors only
  server.on("/api/tds", HTTP_GET, [this](AsyncWebServerRequest *request){
    handleApiTDS(request);
  });

  // API endpoint for system status
  server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request){
    handleApiStatus(request);
  });

  // Calibration routes
  server.on("/calibration", HTTP_GET, [this](AsyncWebServerRequest *request){
    handleCalibrationPage(request);
  });
  
  server.on("/api/calibration/status", HTTP_GET, [this](AsyncWebServerRequest *request){
    handleCalibrationStatus(request);
  });
  
  server.on("/api/calibration/start", HTTP_POST, [this](AsyncWebServerRequest *request){
    handleStartCalibration(request);
  });
  
  server.on("/api/calibration/point", HTTP_POST, [this](AsyncWebServerRequest *request){
    handleAddCalibrationPoint(request);
  });
  
  server.on("/api/calibration/finalize", HTTP_POST, [this](AsyncWebServerRequest *request){
    handleFinalizeCalibration(request);
  });
  
  server.on("/api/calibration/reading", HTTP_GET, [this](AsyncWebServerRequest *request){
    handleCalibrationReading(request);
  });

  server.on("/help", HTTP_GET, [this](AsyncWebServerRequest *request){
    handleHelpPage(request);
  });

  server.on("/diagnostics", HTTP_GET, [this](AsyncWebServerRequest *request){
    handleDiagnosticsPage(request);
  });

  // Enable CORS for API access from other domains
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
}

void AquaWebServer::handleRoot(AsyncWebServerRequest *request) {
  String html = generateDashboardHTML();
  request->send(200, "text/html", html);
}

void AquaWebServer::handleApiSensors(AsyncWebServerRequest *request) {
  if (!sensorController) {
    request->send(500, "application/json", "{\"error\":\"Sensor controller not initialized\"}");
    return;
  }
  
  JsonDocument doc;
  TemperatureData& tempData = sensorController->getTemperatureSensors().getData();
  PHData& phData = sensorController->getPHSensors().getData();
  TDSSensor& tdsSensors = sensorController->getTDSSensors();
  
  JsonArray tempArray = doc["temperature"].to<JsonArray>();
  for (int i = 0; i < NUM_TEMP_SENSORS; i++) {
    tempArray.add(tempData.readings[i]);
  }
  
  JsonArray phArray = doc["ph"].to<JsonArray>();
  for (int i = 0; i < NUM_PH_SENSORS; i++) {
    phArray.add(phData.readings[i]);
  }
  
  JsonArray tdsArray = doc["tds"].to<JsonArray>();
  for (int i = 0; i < NUM_TDS_SENSORS; i++) {
    JsonObject tdsObj = tdsArray.add<JsonObject>();
    tdsObj["ppm"] = tdsSensors.getTDSReading(i);
    tdsObj["ec"] = tdsSensors.getECReading(i);
  }
  
  doc["lastUpdate"] = tempData.lastUpdate;
  doc["timestamp"] = millis();
  
  String response;
  serializeJson(doc, response);
  
  request->send(200, "application/json", response);
}

void AquaWebServer::handleApiTemperature(AsyncWebServerRequest *request) {
  if (!sensorController) {
    request->send(500, "application/json", "{\"error\":\"Sensor controller not initialized\"}");
    return;
  }
  
  JsonDocument doc;
  TemperatureSensor& tempSensors = sensorController->getTemperatureSensors();
  TemperatureData& data = tempSensors.getData();
  
  JsonArray tempArray = doc["sensors"].to<JsonArray>();
  for (int i = 0; i < tempSensors.getSensorCount(); i++) {
    JsonObject sensor = tempArray.add<JsonObject>();
    sensor["id"] = i + 1;
    sensor["value"] = data.readings[i];
    sensor["unit"] = "C";
  }
  
  doc["count"] = tempSensors.getSensorCount();
  doc["timestamp"] = millis();
  
  String response;
  serializeJson(doc, response);
  
  request->send(200, "application/json", response);
}

void AquaWebServer::handleApiPH(AsyncWebServerRequest *request) {
  if (!sensorController) {
    request->send(500, "application/json", "{\"error\":\"Sensor controller not initialized\"}");
    return;
  }
  
  JsonDocument doc;
  PHSensor& phSensors = sensorController->getPHSensors();
  PHData& data = phSensors.getData();
  
  JsonArray phArray = doc["sensors"].to<JsonArray>();
  for (int i = 0; i < phSensors.getSensorCount(); i++) {
    JsonObject sensor = phArray.add<JsonObject>();
    sensor["id"] = i + 1;
    sensor["value"] = data.readings[i];
    sensor["unit"] = "pH";
  }
  
  doc["count"] = phSensors.getSensorCount();
  doc["timestamp"] = millis();
  
  String response;
  serializeJson(doc, response);
  
  request->send(200, "application/json", response);
}

void AquaWebServer::handleApiTDS(AsyncWebServerRequest *request) {
  if (!sensorController) {
    request->send(500, "application/json", "{\"error\":\"Sensor controller not initialized\"}");
    return;
  }
  
  JsonDocument doc;
  TDSSensor& tdsSensors = sensorController->getTDSSensors();
  
  JsonArray tdsArray = doc["sensors"].to<JsonArray>();
  for (int i = 0; i < tdsSensors.getSensorCount(); i++) {
    JsonObject sensor = tdsArray.add<JsonObject>();
    sensor["id"] = i + 1;
    sensor["tds"] = tdsSensors.getTDSReading(i);
    sensor["ec"] = tdsSensors.getECReading(i);
    sensor["tds_unit"] = "ppm";
    sensor["ec_unit"] = "&#181;S/cm";
  }
  
  doc["count"] = tdsSensors.getSensorCount();
  doc["timestamp"] = millis();
  
  String response;
  serializeJson(doc, response);
  
  request->send(200, "application/json", response);
}

void AquaWebServer::handleApiStatus(AsyncWebServerRequest *request) {
  JsonDocument doc;
  
  // Basic system info
  doc["system"] = "ESP32 Aqua Monitor";
  doc["version"] = "1.0.0";
  doc["uptime"] = millis();
  doc["uptimeHours"] = millis() / 3600000.0;
  
  // Memory monitoring
  doc["memory"]["freeHeap"] = ESP.getFreeHeap();
  doc["memory"]["totalHeap"] = ESP.getHeapSize();
  doc["memory"]["usedHeap"] = ESP.getHeapSize() - ESP.getFreeHeap();
  doc["memory"]["heapUsagePercent"] = ((float)(ESP.getHeapSize() - ESP.getFreeHeap()) / ESP.getHeapSize()) * 100;
  doc["memory"]["minFreeHeap"] = ESP.getMinFreeHeap();
  doc["memory"]["maxAllocHeap"] = ESP.getMaxAllocHeap();
  
  // CPU and task monitoring
  doc["cpu"]["frequency"] = ESP.getCpuFreqMHz();
  doc["cpu"]["cores"] = 2; // ESP32 has 2 cores
  doc["cpu"]["utilization"] = getCpuUtilization();
  
  // Flash memory info
  doc["flash"]["size"] = ESP.getFlashChipSize();
  doc["flash"]["speed"] = ESP.getFlashChipSpeed();
  doc["flash"]["mode"] = ESP.getFlashChipMode();
  
  // WiFi performance
  doc["wifi"]["ssid"] = WiFi.SSID();
  doc["wifi"]["rssi"] = WiFi.RSSI();
  doc["wifi"]["ip"] = WiFi.localIP().toString();
  doc["wifi"]["mac"] = WiFi.macAddress();
  doc["wifi"]["channel"] = WiFi.channel();
  
  // Performance indicators
  doc["performance"]["taskStackHighWaterMark"] = uxTaskGetStackHighWaterMark(NULL);
  doc["performance"]["resetReason"] = esp_reset_reason();
  
  // Sensor status
  if (sensorController) {
    doc["sensors"]["temperature"] = sensorController->getTemperatureSensors().getSensorCount();
    doc["sensors"]["ph"] = sensorController->getPHSensors().getSensorCount();
    doc["sensors"]["tds"] = sensorController->getTDSSensors().getSensorCount();
    doc["sensors"]["status"] = "active";
  } else {
    doc["sensors"]["temperature"] = 0;
    doc["sensors"]["ph"] = 0;
    doc["sensors"]["tds"] = 0;
    doc["sensors"]["status"] = "inactive";
  }
  
  // System health indicators
  bool memoryOk = (doc["memory"]["heapUsagePercent"].as<float>() < 80.0);
  bool wifiOk = (WiFi.RSSI() > -70);
  bool cpuOk = (getCpuUtilization() < 80.0);
  bool uptimeOk = (millis() > 60000); // System stable for at least 1 minute
  
  doc["health"]["overall"] = (memoryOk && wifiOk && cpuOk && uptimeOk) ? "good" : "warning";
  doc["health"]["memory"] = memoryOk ? "good" : "high";
  doc["health"]["wifi"] = wifiOk ? "good" : "weak";
  doc["health"]["cpu"] = cpuOk ? "good" : "high";
  doc["health"]["uptime"] = uptimeOk ? "stable" : "starting";
  
  String response;
  serializeJson(doc, response);
  
  request->send(200, "application/json", response);
}

void AquaWebServer::handleCalibrationPage(AsyncWebServerRequest *request) {
  String html = generateCalibrationHTML();
  request->send(200, "text/html", html);
}

void AquaWebServer::handleCalibrationStatus(AsyncWebServerRequest *request) {
  if (!calibrationManager) {
    request->send(500, "application/json", "{\"error\":\"Calibration manager not initialized\"}");
    return;
  }
  
  String response = calibrationManager->getFullCalibrationStatus();
  request->send(200, "application/json", response);
}

void AquaWebServer::handleStartCalibration(AsyncWebServerRequest *request) {
  if (!calibrationManager) {
    request->send(500, "application/json", "{\"error\":\"Calibration manager not initialized\"}");
    return;
  }
  
  if (!request->hasParam("sensor_type", true) || !request->hasParam("sensor_id", true)) {
    request->send(400, "application/json", "{\"error\":\"Missing sensor_type or sensor_id parameter\"}");
    return;
  }
  
  String sensorType = request->getParam("sensor_type", true)->value();
  int sensorId = request->getParam("sensor_id", true)->value().toInt() - 1; // Convert to 0-based
  String notes = request->hasParam("notes", true) ? request->getParam("notes", true)->value() : "";
  
  bool success = false;
  if (sensorType == "temperature") {
    success = calibrationManager->startTemperatureCalibration(sensorId, notes);
  } else if (sensorType == "ph") {
    success = calibrationManager->startPHCalibration(sensorId, notes);
  } else if (sensorType == "tds") {
    success = calibrationManager->startTDSCalibration(sensorId, notes);
  }
  
  JsonDocument doc;
  doc["success"] = success;
  doc["message"] = success ? "Calibration started" : "Failed to start calibration";
  doc["instructions"] = calibrationManager->getCalibrationInstructions(sensorType, 1);
  
  String response;
  serializeJson(doc, response);
  request->send(success ? 200 : 400, "application/json", response);
}

void AquaWebServer::handleAddCalibrationPoint(AsyncWebServerRequest *request) {
  if (!calibrationManager || !sensorController) {
    request->send(500, "application/json", "{\"error\":\"Calibration manager or sensor controller not initialized\"}");
    return;
  }
  
  if (!request->hasParam("sensor_type", true) || !request->hasParam("sensor_id", true) || 
      !request->hasParam("actual_value", true)) {
    request->send(400, "application/json", "{\"error\":\"Missing required parameters\"}");
    return;
  }
  
  String sensorType = request->getParam("sensor_type", true)->value();
  int sensorId = request->getParam("sensor_id", true)->value().toInt() - 1;
  float actualValue = request->getParam("actual_value", true)->value().toFloat();
  float temperature = request->hasParam("temperature", true) ? 
                     request->getParam("temperature", true)->value().toFloat() : 25.0;
  
  // Get current raw reading from sensor
  float rawValue = 0;
  if (sensorType == "temperature") {
    rawValue = sensorController->getTemperatureSensors().getData().readings[sensorId];
  } else if (sensorType == "ph") {
    rawValue = sensorController->getPHSensors().getData().readings[sensorId];
  } else if (sensorType == "tds") {
    rawValue = sensorController->getTDSSensors().getData().readings[sensorId];
  }
  
  bool success = false;
  if (sensorType == "temperature") {
    success = calibrationManager->addTemperatureCalibrationPoint(sensorId, rawValue, actualValue, temperature);
  } else if (sensorType == "ph") {
    success = calibrationManager->addPHCalibrationPoint(sensorId, rawValue, actualValue, temperature);
  } else if (sensorType == "tds") {
    success = calibrationManager->addTDSCalibrationPoint(sensorId, rawValue, actualValue, temperature);
  }
  
  JsonDocument doc;
  doc["success"] = success;
  doc["message"] = success ? "Calibration point added" : "Failed to add calibration point";
  doc["raw_value"] = rawValue;
  doc["actual_value"] = actualValue;
  
  String response;
  serializeJson(doc, response);
  request->send(success ? 200 : 400, "application/json", response);
}

void AquaWebServer::handleFinalizeCalibration(AsyncWebServerRequest *request) {
  if (!calibrationManager) {
    request->send(500, "application/json", "{\"error\":\"Calibration manager not initialized\"}");
    return;
  }
  
  if (!request->hasParam("sensor_type", true) || !request->hasParam("sensor_id", true)) {
    request->send(400, "application/json", "{\"error\":\"Missing sensor_type or sensor_id parameter\"}");
    return;
  }
  
  String sensorType = request->getParam("sensor_type", true)->value();
  int sensorId = request->getParam("sensor_id", true)->value().toInt() - 1;
  
  bool success = false;
  if (sensorType == "temperature") {
    success = calibrationManager->finalizeTemperatureCalibration(sensorId);
  } else if (sensorType == "ph") {
    success = calibrationManager->finalizePHCalibration(sensorId);
  } else if (sensorType == "tds") {
    success = calibrationManager->finalizeTDSCalibration(sensorId);
  }
  
  JsonDocument doc;
  doc["success"] = success;
  doc["message"] = success ? "Calibration completed successfully" : "Failed to finalize calibration";
  
  String response;
  serializeJson(doc, response);
  request->send(success ? 200 : 400, "application/json", response);
}

void AquaWebServer::handleCalibrationReading(AsyncWebServerRequest *request) {
  if (!sensorController) {
    request->send(500, "application/json", "{\"error\":\"Sensor controller not initialized\"}");
    return;
  }
  
  if (!request->hasParam("sensor_type") || !request->hasParam("sensor_id")) {
    request->send(400, "application/json", "{\"error\":\"Missing sensor_type or sensor_id parameter\"}");
    return;
  }
  
  String sensorType = request->getParam("sensor_type")->value();
  int sensorId = request->getParam("sensor_id")->value().toInt() - 1;
  
  if (sensorId < 0 || sensorId >= 8) {
    request->send(400, "application/json", "{\"error\":\"Invalid sensor_id (1-8)\"}");
    return;
  }
  
  JsonDocument doc;
  doc["timestamp"] = millis();
  doc["sensor_type"] = sensorType;
  doc["sensor_id"] = sensorId + 1;
  
  if (sensorType == "temperature") {
    TemperatureData& tempData = sensorController->getTemperatureSensors().getData();
    doc["value"] = tempData.readings[sensorId];
    doc["unit"] = "C";
  } else if (sensorType == "ph") {
    PHData& phData = sensorController->getPHSensors().getData();
    doc["value"] = phData.readings[sensorId];
    doc["unit"] = "pH";
  } else if (sensorType == "tds") {
    TDSSensor& tdsSensors = sensorController->getTDSSensors();
    doc["value"] = tdsSensors.getTDSReading(sensorId);
    doc["ec"] = tdsSensors.getECReading(sensorId);
    doc["unit"] = "ppm";
  } else {
    request->send(400, "application/json", "{\"error\":\"Invalid sensor_type (temperature, ph, tds)\"}");
    return;
  }
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

void AquaWebServer::handleHelpPage(AsyncWebServerRequest *request) {
  String html = generateHelpHTML();
  request->send(200, "text/html", html);
}

String AquaWebServer::generateDashboardHTML() {
  return R"(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Aqua Monitor</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f8ff; }
        .container { max-width: 1400px; margin: 0 auto; }
        h1 { color: #2c5f91; text-align: center; margin-bottom: 10px; }
        .status { text-align: center; padding: 10px; border-radius: 5px; margin: 10px 0; }
        .online { background: #d4edda; color: #155724; }
        
        /* Aquarium Grid Layout */
        .aquarium-grid { 
            display: grid; 
            grid-template-columns: repeat(4, 1fr); 
            gap: 15px; 
            margin: 20px 0; 
        }
        
        /* Individual Aquarium Box */
        .aquarium-box { 
            background: white; 
            border-radius: 12px; 
            box-shadow: 0 4px 15px rgba(0,0,0,0.1); 
            padding: 15px; 
            transition: transform 0.2s ease, box-shadow 0.2s ease;
            border: 3px solid #e0e0e0;
        }
        
        .aquarium-box:hover {
            transform: translateY(-2px);
            box-shadow: 0 6px 20px rgba(0,0,0,0.15);
        }
        
        /* Aquarium Title */
        .aquarium-title { 
            font-size: 1.1em; 
            font-weight: bold; 
            color: #2c5f91; 
            text-align: center; 
            margin-bottom: 12px; 
            padding: 8px;
            background: linear-gradient(135deg, #e6f3ff, #f0f8ff);
            border-radius: 8px;
            border: 1px solid #b3d9ff;
        }
        
        /* Sensor Reading Row */
        .sensor-row { 
            display: flex; 
            justify-content: space-between; 
            align-items: center;
            margin: 8px 0; 
            padding: 8px 12px; 
            border-radius: 6px; 
            font-size: 0.95em;
        }
        
        /* Sensor Type Colors */
        .temp-row { background: linear-gradient(135deg, #ffe6e6, #fff0f0); border-left: 4px solid #ff6b6b; }
        .ph-row { background: linear-gradient(135deg, #e6f3ff, #f0f8ff); border-left: 4px solid #4dabf7; }
        .tds-row { background: linear-gradient(135deg, #e6ffe6, #f0fff0); border-left: 4px solid #51cf66; }
        
        .sensor-label { 
            font-weight: 600; 
            color: #333; 
            min-width: 45px;
        }
        
        .sensor-value { 
            font-weight: bold; 
            color: #2c5f91; 
            font-family: 'Courier New', monospace;
            font-size: 0.9em;
        }
        
        /* Controls */
        .controls { 
            text-align: center; 
            margin: 25px 0; 
        }
        
        .control-button { 
            background: linear-gradient(135deg, #2c5f91, #3d6fa7); 
            color: white; 
            padding: 12px 25px; 
            border: none; 
            border-radius: 8px; 
            font-size: 16px; 
            cursor: pointer; 
            margin: 0 10px;
            transition: all 0.3s ease;
            box-shadow: 0 3px 10px rgba(44, 95, 145, 0.3);
        }
        
        .control-button:hover { 
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(44, 95, 145, 0.4);
            background: linear-gradient(135deg, #3d6fa7, #2c5f91);
        }
        
        .help-button {
            background: linear-gradient(135deg, #6c757d, #5a6268);
            box-shadow: 0 3px 10px rgba(108, 117, 125, 0.3);
        }
        
        .help-button:hover {
            background: linear-gradient(135deg, #5a6268, #6c757d);
            box-shadow: 0 5px 15px rgba(108, 117, 125, 0.4);
        }
        
        .timestamp { 
            text-align: center; 
            margin: 20px 0; 
            color: #666; 
            font-size: 0.9em;
        }
        
        /* Responsive Design */
        @media (max-width: 1200px) {
            .aquarium-grid { 
                grid-template-columns: repeat(3, 1fr); 
            }
        }
        
        @media (max-width: 900px) {
            .aquarium-grid { 
                grid-template-columns: repeat(2, 1fr); 
                gap: 12px; 
            }
            .aquarium-box { padding: 12px; }
        }
        
        @media (max-width: 600px) {
            .aquarium-grid { 
                grid-template-columns: repeat(2, 1fr); 
                gap: 10px; 
            }
            .control-button { padding: 10px 20px; font-size: 14px; margin: 5px; }
        }
        
        @media (max-width: 400px) {
            .aquarium-grid { 
                grid-template-columns: 1fr; 
                gap: 10px; 
            }
            .sensor-row { font-size: 0.85em; padding: 6px 10px; }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32 Aqua Monitoring System</h1>
        <div class="status online">System Online - Real-time Monitoring</div>
        
        <div class="aquarium-grid" id="aquarium-grid">
            <!-- Aquarium boxes will be dynamically generated here -->
        </div>
        
        <div class="controls">
            <button onclick="location.href='/calibration'" class="control-button">
                &#9881; Sensor Calibration
            </button>
            <button onclick="location.href='/diagnostics'" class="control-button help-button">
                &#128295; System Diagnostics
            </button>
            <button onclick="location.href='/help'" class="control-button help-button">
                &#10068; Help & Shopping Guide
            </button>
        </div>
        
        <div class="timestamp" id="last-update">Loading...</div>
    </div>

    <script>
        function updateReadings() {
            fetch('/api/sensors')
                .then(response => response.json())
                .then(data => {
                    const gridContainer = document.getElementById('aquarium-grid');
                    gridContainer.innerHTML = '';
                    
                    // Assuming 8 aquariums (sensors 1-8)
                    for (let i = 0; i < 8; i++) {
                        const aquariumDiv = document.createElement('div');
                        aquariumDiv.className = 'aquarium-box';
                        
                        const tempValue = data.temperature[i] ? data.temperature[i].toFixed(2) : '--';
                        const phValue = data.ph[i] ? data.ph[i].toFixed(2) : '--';
                        const tdsValue = data.tds[i] ? `${data.tds[i].ppm.toFixed(0)} ppm` : '-- ppm';
                        const ecValue = data.tds[i] ? `${data.tds[i].ec.toFixed(0)}&#181;S/cm` : '--&#181;S/cm';
                        
                        aquariumDiv.innerHTML = `
                            <div class="aquarium-title">Aquarium ${i + 1}</div>
                            
                            <div class="sensor-row temp-row">
                                <span class="sensor-label">Temp:</span>
                                <span class="sensor-value">${tempValue}&#176;C</span>
                            </div>
                            
                            <div class="sensor-row ph-row">
                                <span class="sensor-label">pH:</span>
                                <span class="sensor-value">${phValue}</span>
                            </div>
                            
                            <div class="sensor-row tds-row">
                                <span class="sensor-label">TDS:</span>
                                <span class="sensor-value">${tdsValue}</span>
                            </div>
                            
                            <div class="sensor-row tds-row" style="margin-top: 2px;">
                                <span class="sensor-label">EC:</span>
                                <span class="sensor-value">${ecValue}</span>
                            </div>
                        `;
                        
                        gridContainer.appendChild(aquariumDiv);
                    }
                    
                    // Update timestamp
                    document.getElementById('last-update').textContent = 
                        `Last updated: ${new Date().toLocaleTimeString()}`;
                })
                .catch(error => {
                    console.error('Error fetching data:', error);
                    document.getElementById('last-update').textContent = 'Connection error';
                });
        }
        
        // Update every 5 seconds
        updateReadings();
        setInterval(updateReadings, 5000);
    </script>
</body>
</html>
  )";
}

String AquaWebServer::generateCalibrationHTML() {
  return R"HTML(<!DOCTYPE html>
<html>
<head>
    <title>Sensor Calibration - ESP32 Aqua Monitor</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f8ff; }
        .container { max-width: 1200px; margin: 0 auto; }
        h1 { color: #2c5f91; text-align: center; }
        .nav-button { background: #6c757d; color: white; padding: 10px 20px; border: none; border-radius: 5px; margin: 5px; cursor: pointer; text-decoration: none; display: inline-block; }
        .nav-button:hover { background: #5a6268; }
        .sensor-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(350px, 1fr)); gap: 20px; margin: 20px 0; }
        .sensor-card { background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        .sensor-title { font-size: 1.2em; color: #2c5f91; margin-bottom: 15px; border-bottom: 2px solid #e0e0e0; padding-bottom: 10px; }
        .sensor-item { display: flex; justify-content: space-between; align-items: center; margin: 10px 0; padding: 10px; border-radius: 5px; background: #f8f9fa; }
        .sensor-status { font-weight: bold; }
        .current-value { background: #e9ecef; padding: 4px 8px; border-radius: 3px; font-family: monospace; min-width: 80px; text-align: center; margin: 0 10px; }
        .calibrated { color: #28a745; }
        .not-calibrated { color: #dc3545; }
        .cal-button { background: #007bff; color: white; padding: 8px 15px; border: none; border-radius: 3px; cursor: pointer; font-size: 14px; }
        .cal-button:hover { background: #0056b3; }
        .cal-button:disabled { background: #6c757d; cursor: not-allowed; }
        .instructions { background: #d1ecf1; border: 1px solid #bee5eb; color: #0c5460; padding: 15px; border-radius: 5px; margin: 20px 0; }
        .form-group { margin: 15px 0; }
        .form-group label { display: block; margin-bottom: 5px; font-weight: bold; }
        .form-group input, .form-group textarea { width: 100%; padding: 8px; border: 1px solid #ccc; border-radius: 3px; }
        .button-group { text-align: center; margin: 20px 0; }
        .modal { display: none; position: fixed; z-index: 1; left: 0; top: 0; width: 100%; height: 100%; background-color: rgba(0,0,0,0.4); }
        .modal-content { background-color: #fefefe; margin: 15% auto; padding: 20px; border-radius: 10px; width: 90%; max-width: 500px; }
        .close { color: #aaa; float: right; font-size: 28px; font-weight: bold; cursor: pointer; }
        .close:hover { color: black; }
        .progress { background: #e9ecef; border-radius: 5px; margin: 10px 0; }
        .progress-bar { background: #007bff; height: 20px; border-radius: 5px; text-align: center; color: white; line-height: 20px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Sensor Calibration System</h1>
        
        <div style="text-align: center; margin-bottom: 20px;">
            <a href="/" class="nav-button">&#8962; Dashboard</a>
            <button onclick="refreshCalibrationStatus()" class="nav-button">&#8635; Refresh</button>
        </div>
        
        <div class="instructions">
            <h3>&#9432; Quick Calibration Guide</h3>
            <p><strong>1.</strong> Click "Calibrate" button for any sensor</p>
            <p><strong>2.</strong> Enter reference value for calibration solution</p>
            <p><strong>3.</strong> Place sensor in solution and wait for stable reading</p>
            <p><strong>4.</strong> Add up to 3 calibration points for accuracy</p>
            <p><strong>5.</strong> Complete calibration when finished</p>
            <a href="/help" class="nav-button" style="margin-top: 10px; display: inline-block;">&#10068; Detailed Instructions & Shopping Guide</a>
        </div>
        
        <div style="text-align: center; margin: 10px 0; color: #666; font-size: 0.9em;">
            <span id="cal-last-update">Loading sensor values...</span>
        </div>
        
        <div class="sensor-grid">
            <div class="sensor-card">
                <div class="sensor-title">Temperature Sensors</div>
                <div id="temperature-calibration">Loading...</div>
            </div>
            
            <div class="sensor-card">
                <div class="sensor-title">pH Sensors</div>
                <div id="ph-calibration">Loading...</div>
            </div>
            
            <div class="sensor-card">
                <div class="sensor-title">TDS Sensors</div>
                <div id="tds-calibration">Loading...</div>
            </div>
        </div>
    </div>
    
    <script>
        var currentCalibration = {
            sensorType: '',
            sensorId: 0,
            step: 0,
            totalSteps: 0
        };
        
        function refreshCalibrationStatus() {
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function() {
                if (xhr.readyState == 4 && xhr.status == 200) {
                    var data = JSON.parse(xhr.responseText);
                    updateCalibrationDisplay(data);
                }
            };
            xhr.open('GET', '/api/calibration/status', true);
            xhr.send();
            
            // Also fetch current sensor readings
            fetchCurrentReadings();
        }
        
        var currentReadings = {};
        
        function fetchCurrentReadings() {
            // Fetch all sensor readings from the unified endpoint
            console.log('Fetching current sensor readings...');
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function() {
                if (xhr.readyState == 4 && xhr.status == 200) {
                    var data = JSON.parse(xhr.responseText);
                    currentReadings = data;
                    updateSensorReadings();
                    // Update timestamp
                    document.getElementById('cal-last-update').textContent = 
                        'Sensor values updated: ' + new Date().toLocaleTimeString();
                    console.log('Sensor readings updated successfully');
                } else if (xhr.readyState == 4) {
                    console.error('Failed to fetch sensor readings, status:', xhr.status);
                }
            };
            xhr.open('GET', '/api/sensors', true);
            xhr.send();
        }
        
        function updateCalibrationDisplay(data) {
            // Update temperature sensors
            var tempContainer = document.getElementById('temperature-calibration');
            if (!tempContainer.querySelector('.sensor-item')) {
                // Only rebuild if sensor items don't exist (first load or after loading state)
                tempContainer.innerHTML = '';
                for (var i = 0; i < 8; i++) {
                    var div = document.createElement('div');
                    div.className = 'sensor-item';
                    div.innerHTML = '<span class="sensor-status" id="temp-status-' + i + '">Temp' + (i + 1) + ':</span>' +
                        '<span class="current-value" id="temp-value-' + i + '">--</span>' +
                        '<button class="cal-button" onclick="openCalibrationModal(\'temperature\', ' + (i + 1) + ')">Calibrate</button>';
                    tempContainer.appendChild(div);
                }
            }
            
            // Update calibration status for temperature sensors
            for (var i = 0; i < 8; i++) {
                var isCalibrated = data.temperature && data.temperature[i] && data.temperature[i].isCalibrated;
                var statusElem = document.getElementById('temp-status-' + i);
                if (statusElem) {
                    statusElem.className = 'sensor-status ' + (isCalibrated ? 'calibrated' : 'not-calibrated');
                }
            }
            
            // Update pH sensors
            var phContainer = document.getElementById('ph-calibration');
            if (!phContainer.querySelector('.sensor-item')) {
                // Only rebuild if sensor items don't exist (first load or after loading state)
                phContainer.innerHTML = '';
                for (var i = 0; i < 8; i++) {
                    var div = document.createElement('div');
                    div.className = 'sensor-item';
                    div.innerHTML = '<span class="sensor-status" id="ph-status-' + i + '">pH' + (i + 1) + ':</span>' +
                        '<span class="current-value" id="ph-value-' + i + '">--</span>' +
                        '<button class="cal-button" onclick="openCalibrationModal(\'ph\', ' + (i + 1) + ')">Calibrate</button>';
                    phContainer.appendChild(div);
                }
            }
            
            // Update calibration status for pH sensors
            for (var i = 0; i < 8; i++) {
                var isCalibrated = data.ph && data.ph[i] && data.ph[i].isCalibrated;
                var statusElem = document.getElementById('ph-status-' + i);
                if (statusElem) {
                    statusElem.className = 'sensor-status ' + (isCalibrated ? 'calibrated' : 'not-calibrated');
                }
            }
            
            // Update TDS sensors
            var tdsContainer = document.getElementById('tds-calibration');
            if (!tdsContainer.querySelector('.sensor-item')) {
                // Only rebuild if sensor items don't exist (first load or after loading state)
                tdsContainer.innerHTML = '';
                for (var i = 0; i < 8; i++) {
                    var div = document.createElement('div');
                    div.className = 'sensor-item';
                    div.innerHTML = '<span class="sensor-status" id="tds-status-' + i + '">TDS' + (i + 1) + ':</span>' +
                        '<span class="current-value" id="tds-value-' + i + '">--</span>' +
                        '<button class="cal-button" onclick="openCalibrationModal(\'tds\', ' + (i + 1) + ')">Calibrate</button>';
                    tdsContainer.appendChild(div);
                }
            }
            
            // Update calibration status for TDS sensors
            for (var i = 0; i < 8; i++) {
                var isCalibrated = data.tds && data.tds[i] && data.tds[i].isCalibrated;
                var statusElem = document.getElementById('tds-status-' + i);
                if (statusElem) {
                    statusElem.className = 'sensor-status ' + (isCalibrated ? 'calibrated' : 'not-calibrated');
                }
            }
        }
        
        function updateSensorReadings() {
            // Update temperature readings
            if (currentReadings.temperature && Array.isArray(currentReadings.temperature)) {
                currentReadings.temperature.forEach(function(temp, i) {
                    var elem = document.getElementById('temp-value-' + i);
                    if (elem && typeof temp === 'number' && !isNaN(temp)) {
                        elem.innerHTML = temp.toFixed(2) + '&#176;C';
                    }
                });
            }
            
            // Update pH readings
            if (currentReadings.ph && Array.isArray(currentReadings.ph)) {
                currentReadings.ph.forEach(function(ph, i) {
                    var elem = document.getElementById('ph-value-' + i);
                    if (elem && typeof ph === 'number' && !isNaN(ph)) {
                        elem.textContent = ph.toFixed(2);
                    }
                });
            }
            
            // Update TDS readings
            if (currentReadings.tds && Array.isArray(currentReadings.tds)) {
                currentReadings.tds.forEach(function(tds, i) {
                    var elem = document.getElementById('tds-value-' + i);
                    if (elem && tds && typeof tds.ppm === 'number' && typeof tds.ec === 'number' && !isNaN(tds.ppm) && !isNaN(tds.ec)) {
                        elem.innerHTML = tds.ppm.toFixed(0) + ' ppm / ' + tds.ec.toFixed(0) + '&#181;S/cm';
                    }
                });
            }
        }
        
        var currentCalibration = {
            sensorType: '',
            sensorId: 0,
            step: 0,
            totalSteps: 0,
            isActive: false,
            readings: [],
            stabilityTimer: null,
            updateTimer: null
        };
        
        // Statistical analysis for stability detection
        function calculateStats(readings) {
            if (readings.length < 10) return null;
            
            var sum = readings.reduce(function(a, b) { return a + b; }, 0);
            var avg = sum / readings.length;
            
            var variance = readings.reduce(function(sum, val) {
                return sum + Math.pow(val - avg, 2);
            }, 0) / readings.length;
            
            var stdev = Math.sqrt(variance);
            
            return {
                average: avg,
                stdev: stdev,
                count: readings.length,
                isStable: readings.length >= 20 && stdev < getStabilityThreshold(currentCalibration.sensorType)
            };
        }
        
        function getStabilityThreshold(sensorType) {
            // Stability thresholds for different sensor types
            switch(sensorType) {
                case 'temperature': return 0.1; // ±0.1°C
                case 'ph': return 0.05; // +/-0.05 pH
                case 'tds': return 2.0; // +/-2 ppm
                default: return 0.1;
            }
        }
        
        function startLiveReadings() {
            if (currentCalibration.updateTimer) {
                clearInterval(currentCalibration.updateTimer);
            }
            
            currentCalibration.readings = [];
            currentCalibration.isActive = true;
            
            currentCalibration.updateTimer = setInterval(function() {
                if (!currentCalibration.isActive) return;
                
                var xhr = new XMLHttpRequest();
                xhr.onreadystatechange = function() {
                    if (xhr.readyState == 4 && xhr.status == 200) {
                        var data = JSON.parse(xhr.responseText);
                        updateLiveReading(data);
                    }
                };
                xhr.open('GET', '/api/calibration/reading?sensor_type=' + currentCalibration.sensorType + '&sensor_id=' + currentCalibration.sensorId, true);
                xhr.send();
            }, 500); // Update every 500ms
        }
        
        function stopLiveReadings() {
            currentCalibration.isActive = false;
            if (currentCalibration.updateTimer) {
                clearInterval(currentCalibration.updateTimer);
                currentCalibration.updateTimer = null;
            }
            if (currentCalibration.stabilityTimer) {
                clearTimeout(currentCalibration.stabilityTimer);
                currentCalibration.stabilityTimer = null;
            }
        }
        
        function updateLiveReading(data) {
            // Add reading to rolling buffer (keep last 30 readings = 15 seconds)
            currentCalibration.readings.push(data.value);
            if (currentCalibration.readings.length > 30) {
                currentCalibration.readings.shift();
            }
            
            // Update display
            var readingDiv = document.getElementById('currentReading');
            if (readingDiv) {
                var stats = calculateStats(currentCalibration.readings);
                var stabilityText = '';
                
                if (stats) {
                    var stabilityIndicator = stats.isStable ? '[STABLE]' : '[STABILIZING]';
                    stabilityText = ' - ' + stabilityIndicator + ' (stdev=' + stats.stdev.toFixed(3) + ')';
                    
                    // Auto-advance when stable
                    if (stats.isStable && !currentCalibration.stabilityTimer) {
                        currentCalibration.stabilityTimer = setTimeout(function() {
                            autoAddCalibrationPoint(stats.average);
                        }, 3000); // Wait 3 seconds after stability detected
                    }
                }
                
                readingDiv.innerHTML = '<strong>Current Reading:</strong> ' + data.value.toFixed(2) + ' ' + (data.unit || '') + stabilityText +
                    '<br><small>Readings: ' + currentCalibration.readings.length + '/30</small>';
            }
        }
        
        function autoAddCalibrationPoint(stableValue) {
            // Use the reference value already entered in the modal
            var referenceValue = currentCalibration.currentReferenceValue;
            if (!referenceValue) {
                alert('[ERROR] No reference value set. Please restart calibration.');
                return;
            }
            
            // Add calibration point with pre-entered reference value
            var formData = new FormData();
            formData.append('sensor_type', currentCalibration.sensorType);
            formData.append('sensor_id', currentCalibration.sensorId);
            formData.append('actual_value', referenceValue);
            
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function() {
                if (xhr.readyState == 4) {
                    if (xhr.status == 200) {
                        var data = JSON.parse(xhr.responseText);
                        if (data.success) {
                            currentCalibration.readings = []; // Reset for next point
                            if (currentCalibration.stabilityTimer) {
                                clearTimeout(currentCalibration.stabilityTimer);
                                currentCalibration.stabilityTimer = null;
                            }
                            
                            // Ask if user wants to add another calibration point
                            askForNextCalibrationPoint();
                        } else {
                            alert('[ERROR] Failed to add calibration point: ' + (data.message || 'Unknown error'));
                        }
                    } else {
                        alert('[ERROR] Server error while adding calibration point');
                    }
                }
            };
            xhr.open('POST', '/api/calibration/point', true);
            xhr.send(formData);
        }
        
        function askForNextCalibrationPoint() {
            // Maximum 3 calibration points, minimum 1
            if (currentCalibration.step >= 3) {
                finishCalibration();
                return;
            }
            
            var addAnother = confirm('&#9989; CALIBRATION POINT ADDED: Point ' + currentCalibration.step + ' of 3 added successfully.\\n\\nWould you like to add another calibration point?\\n\\n&#9989; OK = Add another point\\n&#10060; Cancel = Finish calibration (minimum 1 point required)');
            
            if (addAnother) {
                // Ask for next reference value
                var nextReferenceValue = promptForReferenceValue();
                if (nextReferenceValue !== null) {
                    currentCalibration.currentReferenceValue = nextReferenceValue;
                    updateProgress();
                    showNextCalibrationStep();
                } else {
                    finishCalibration();
                }
            } else {
                finishCalibration();
            }
        }
        
        function finishCalibration() {
            stopLiveReadings();
            document.getElementById('addPointButton').style.display = 'none';
            document.getElementById('finalizeButton').style.display = 'inline-block';
            
            var pointsText = currentCalibration.step === 1 ? '1 calibration point' : currentCalibration.step + ' calibration points';
            showCalibrationInstructions('[CALIBRATION READY] ' + pointsText + ' collected. Click Finish to complete calibration.');
            
            // Set progress to 100%
            updateProgressToComplete();
        }
        
        function getCurrentReferenceValue() {
            // Get reference value from user input (stored when calibration point dialog is accepted)
            return currentCalibration.currentReferenceValue || '0';
        }
        
        function promptForReferenceValue() {
            var sensorType = currentCalibration.sensorType;
            var step = currentCalibration.step;
            var unit = getSensorUnit(sensorType);
            
            var message = '[CALIBRATION POINT ' + step + '] Please enter the reference ' + sensorType + ' value (' + unit + '):';
            var suggested = getSuggestedReferenceValue(sensorType, step);
            
            var referenceValue = prompt(message, suggested);
            if (referenceValue === null || referenceValue === '') {
                return null; // User cancelled
            }
            
            // Validate the input
            var numValue = parseFloat(referenceValue);
            if (isNaN(numValue)) {
                alert('[ERROR] Invalid reference value. Please enter a valid number.');
                return promptForReferenceValue(); // Try again
            }
            
            return referenceValue;
        }
        
        function getSensorUnit(sensorType) {
            switch(sensorType) {
                case 'temperature': return '&#176;C';
                case 'ph': return 'pH';
                case 'tds': return 'ppm';
                default: return '';
            }
        }
        
        function getSuggestedReferenceValue(sensorType, step) {
            // Provide common reference values as suggestions
            if (sensorType === 'temperature') {
                return step === 1 ? '0.0' : '25.0';
            } else if (sensorType === 'ph') {
                var values = ['4.01', '7.00', '10.01'];
                return values[step - 1] || '7.00';
            } else if (sensorType === 'tds') {
                return step === 1 ? '1413' : '12880';
            }
            return '0';
        }
        
        function showNextCalibrationStep() {
            currentCalibration.step++; // Increment to next step (1, 2, 3...)
            var pointText = currentCalibration.step === 1 ? 'first' : (currentCalibration.step === 2 ? 'second' : 'third');
            var sensorTypeText = currentCalibration.sensorType.toUpperCase();
            
            var instruction = '[CALIBRATION POINT ' + currentCalibration.step + '] Place your sensor in the ' + sensorTypeText + ' reference standard (' + currentCalibration.currentReferenceValue + ' ' + getSensorUnit(currentCalibration.sensorType) + ').<br><br>' +
                'Wait for the reading to stabilize. The calibration point will be added automatically when the reading is stable.';
            
            showCalibrationInstructions(instruction + '<br><strong>The system will automatically detect when the reading is stable.</strong>');
            startLiveReadings();
        }
        
        function openCalibrationModal(sensorType, sensorId) {
            currentCalibration.sensorType = sensorType;
            currentCalibration.sensorId = sensorId;
            currentCalibration.step = 0;
            currentCalibration.totalSteps = 3; // Maximum 3 points, but user can stop early
            
            // Create modal HTML dynamically
            var modalHTML = '<div id="calibrationModal" style="display:block; position:fixed; z-index:1000; left:0; top:0; width:100%; height:100%; background-color:rgba(0,0,0,0.4);">' +
                '<div style="background-color:#fefefe; margin:15% auto; padding:20px; border-radius:10px; width:90%; max-width:500px;">' +
                '<span style="color:#aaa; float:right; font-size:28px; font-weight:bold; cursor:pointer;" onclick="closeCalibrationModal()">&times;</span>' +
                '<h2 id="modalTitle">' + sensorType.toUpperCase() + ' Sensor ' + sensorId + ' Calibration</h2>' +
                '<div style="margin:15px 0;"><label style="display:block; margin-bottom:5px; font-weight:bold;">Calibration Notes:</label>' +
                '<textarea id="calibrationNotes" rows="2" style="width:100%; padding:8px; border:1px solid #ccc; border-radius:3px;" placeholder="Optional notes about this calibration..."></textarea></div>' +
                '<div style="margin:15px 0;"><label style="display:block; margin-bottom:5px; font-weight:bold;">Reference Value (' + getSensorUnit(sensorType) + '):</label>' +
                '<input type="number" step="any" id="referenceValue" style="width:100%; padding:8px; border:1px solid #ccc; border-radius:3px;" placeholder="Enter the actual ' + sensorType + ' value of your reference standard" value="' + getSuggestedReferenceValue(sensorType, 1) + '"></div>' +
                '<div id="calibrationSteps"></div>' +
                '<div id="currentReading" style="margin:15px 0; padding:12px; background:#e8f4fd; border-radius:5px; border-left:4px solid #0066cc; font-family:monospace; display:none;"></div>' +
                '<div style="background:#e9ecef; border-radius:5px; margin:10px 0;"><div id="progressBar" style="background:#007bff; height:20px; border-radius:5px; text-align:center; color:white; line-height:20px; width:0%;">0%</div></div>' +
                '<div style="text-align:center; margin:20px 0;">' +
                '<button id="startCalButton" onclick="startCalibration()" style="background:#007bff; color:white; padding:8px 15px; border:none; border-radius:3px; cursor:pointer; margin:5px;">[START] Begin Calibration</button>' +
                '<button id="addPointButton" onclick="addCalibrationPoint()" style="background:#007bff; color:white; padding:8px 15px; border:none; border-radius:3px; cursor:pointer; margin:5px; display:none;">[ADD] Calibration Point</button>' +
                '<button id="finalizeButton" onclick="finalizeCalibration()" style="background:#007bff; color:white; padding:8px 15px; border:none; border-radius:3px; cursor:pointer; margin:5px; display:none;">[FINISH] Complete Calibration</button>' +
                '<button onclick="closeCalibrationModal()" style="background:#6c757d; color:white; padding:8px 15px; border:none; border-radius:3px; cursor:pointer; margin:5px;">[CANCEL] Close</button>' +
                '</div></div></div>';
            
            document.body.insertAdjacentHTML('beforeend', modalHTML);
        }
        
        function closeCalibrationModal() {
            stopLiveReadings();
            var modal = document.getElementById('calibrationModal');
            if (modal) {
                modal.remove();
            }
        }
        
        function startCalibration() {
            var notes = document.getElementById('calibrationNotes').value;
            var referenceValue = document.getElementById('referenceValue').value;
            
            // Validate reference value
            if (!referenceValue || isNaN(parseFloat(referenceValue))) {
                alert('[ERROR] Please enter a valid reference value before starting calibration.');
                return;
            }
            
            // Store reference value for this calibration
            currentCalibration.currentReferenceValue = referenceValue;
            
            var formData = new FormData();
            formData.append('sensor_type', currentCalibration.sensorType);
            formData.append('sensor_id', currentCalibration.sensorId);
            formData.append('notes', notes);
            
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function() {
                if (xhr.readyState == 4) {
                    if (xhr.status == 200) {
                        var data = JSON.parse(xhr.responseText);
                        if (data.success) {
                            currentCalibration.step = 0;  // Start at 0 - no points added yet
                            updateProgress();  // Show 0% progress
                            document.getElementById('startCalButton').style.display = 'none';
                            document.getElementById('addPointButton').style.display = 'none'; // Hide manual button
                            document.getElementById('currentReading').style.display = 'block';
                            showNextCalibrationStep();
                        } else {
                            alert('[ERROR] ' + (data.message || 'Failed to start calibration'));
                        }
                    } else {
                        alert('[ERROR] Failed to start calibration - Server error');
                    }
                }
            };
            xhr.open('POST', '/api/calibration/start', true);
            xhr.send(formData);
        }
        
        function addCalibrationPoint() {
            var instruction = getCalibrationInstruction(currentCalibration.sensorType, currentCalibration.step);
            var actualValue = prompt('[STEP ' + currentCalibration.step + '] ' + instruction + '\\n\\nEnter the actual ' + currentCalibration.sensorType + ' value:');
            if (!actualValue) return;
            
            var formData = new FormData();
            formData.append('sensor_type', currentCalibration.sensorType);
            formData.append('sensor_id', currentCalibration.sensorId);
            formData.append('actual_value', actualValue);
            
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function() {
                if (xhr.readyState == 4) {
                    if (xhr.status == 200) {
                        var data = JSON.parse(xhr.responseText);
                        if (data.success) {
                            currentCalibration.step++;
                            
                            if (currentCalibration.step >= currentCalibration.totalSteps) {
                                // All points collected - show 100% progress and finalize button
                                updateProgressToComplete();
                                document.getElementById('addPointButton').style.display = 'none';
                                document.getElementById('finalizeButton').style.display = 'inline-block';
                                showCalibrationInstructions('[READY] All calibration points added. Click Finish to complete.');
                            } else {
                                // More points needed - update progress and continue
                                updateProgress();
                                showCalibrationInstructions(getCalibrationInstruction(currentCalibration.sensorType, currentCalibration.step));
                            }
                        } else {
                            alert('[ERROR] ' + (data.message || 'Failed to add calibration point'));
                        }
                    } else {
                        alert('[ERROR] Failed to add calibration point - Server error');
                    }
                }
            };
            xhr.open('POST', '/api/calibration/point', true);
            xhr.send(formData);
        }
        
        function finalizeCalibration() {
            var formData = new FormData();
            formData.append('sensor_type', currentCalibration.sensorType);
            formData.append('sensor_id', currentCalibration.sensorId);
            
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function() {
                if (xhr.readyState == 4) {
                    if (xhr.status == 200) {
                        var data = JSON.parse(xhr.responseText);
                        if (data.success) {
                            alert('[SUCCESS] Calibration completed successfully!');
                            closeCalibrationModal();
                            refreshCalibrationStatus();
                        } else {
                            alert('[ERROR] ' + (data.message || 'Failed to finalize calibration'));
                        }
                    } else {
                        alert('[ERROR] Failed to finalize calibration - Server error');
                    }
                }
            };
            xhr.open('POST', '/api/calibration/finalize', true);
            xhr.send(formData);
        }
        
        function updateProgress() {
            // Calculate progress based on completed steps (minimum 1, maximum 3)
            var progress = Math.min(100, (currentCalibration.step / 3) * 100);
            
            var progressBar = document.getElementById('progressBar');
            if (progressBar) {
                progressBar.style.width = progress + '%';
                progressBar.textContent = Math.round(progress) + '%';
            }
        }
        
        function updateProgressToComplete() {
            // Force progress to 100% when calibration is complete
            var progressBar = document.getElementById('progressBar');
            if (progressBar) {
                progressBar.style.width = '100%';
                progressBar.textContent = '100%';
            }
        }
        
        function showCalibrationInstructions(text) {
            var stepsDiv = document.getElementById('calibrationSteps');
            if (stepsDiv) {
                stepsDiv.innerHTML = '<div style="background:#d1ecf1; border:1px solid #bee5eb; color:#0c5460; padding:15px; border-radius:5px; margin:20px 0;"><strong>[INSTRUCTION]</strong> ' + text + '</div>';
            }
        }
        
        function getCalibrationInstruction(sensorType, step) {
            // Generic calibration instructions - specific values entered by user
            var sensorTypeText = sensorType.toUpperCase();
            var stepText = step === 1 ? 'first' : (step === 2 ? 'second' : 'third');
            
            return 'Place sensor in your ' + stepText + ' ' + sensorTypeText + ' reference standard and wait for stable reading';
        }
        
        // Load calibration status on page load
        document.addEventListener('DOMContentLoaded', function() {
            console.log('Calibration page loaded, starting refresh cycle');
            refreshCalibrationStatus();
        });
        
        // Auto-refresh every 5 seconds to match dashboard
        setInterval(function() {
            console.log('Auto-refreshing calibration page at', new Date().toLocaleTimeString());
            refreshCalibrationStatus();
        }, 5000);
    </script>
</body>
</html>)HTML";
}

String AquaWebServer::generateHelpHTML() {
  return R"HTML(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Aqua Monitor - Calibration Help</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f8ff; }
        .container { max-width: 1000px; margin: 0 auto; }
        h1 { color: #2c5f91; text-align: center; }
        h2 { color: #2c5f91; border-bottom: 2px solid #e0e0e0; padding-bottom: 10px; }
        h3 { color: #0066cc; margin-top: 25px; }
        .nav-bar { text-align: center; margin: 20px 0; padding: 15px; background: white; border-radius: 10px; }
        .nav-button { background: #6c757d; color: white; padding: 10px 20px; border: none; border-radius: 5px; margin: 5px; cursor: pointer; text-decoration: none; display: inline-block; }
        .nav-button:hover { background: #5a6268; }
        .section { background: white; padding: 20px; margin: 20px 0; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        .procedure-step { background: #f8f9fa; padding: 15px; margin: 10px 0; border-left: 4px solid #007bff; border-radius: 5px; }
        .warning { background: #fff3cd; border: 1px solid #ffeaa7; color: #856404; padding: 15px; border-radius: 5px; margin: 10px 0; }
        .shopping-list { background: #d1ecf1; border: 1px solid #bee5eb; padding: 15px; border-radius: 5px; margin: 10px 0; }
        .amazon-link { color: #007bff; text-decoration: none; font-weight: bold; }
        .amazon-link:hover { text-decoration: underline; }
        table { width: 100%; border-collapse: collapse; margin: 15px 0; }
        th, td { border: 1px solid #ddd; padding: 12px; text-align: left; }
        th { background-color: #f2f2f2; }
        .highlight { background: #ffffcc; padding: 2px 4px; border-radius: 3px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>&#10068; ESP32 Aqua Monitor - Calibration Guide</h1>
        
        <div class="nav-bar">
            <a href="/" class="nav-button">&#8962; Dashboard</a>
            <a href="/calibration" class="nav-button">&#9881; Calibration</a>
        </div>

        <div class="section">
            <h2>&#9432; General Calibration Process</h2>
            <div class="procedure-step">
                <strong>Step 1:</strong> Click "Calibrate" button for the sensor you want to calibrate
            </div>
            <div class="procedure-step">
                <strong>Step 2:</strong> Enter the reference value of your calibration solution
            </div>
            <div class="procedure-step">
                <strong>Step 3:</strong> Place sensor in the reference solution and wait for reading to stabilize
            </div>
            <div class="procedure-step">
                <strong>Step 4:</strong> System will automatically detect stable readings and add calibration point
            </div>
            <div class="procedure-step">
                <strong>Step 5:</strong> Repeat for up to 3 different reference solutions (recommended for accuracy)
            </div>
            <div class="procedure-step">
                <strong>Step 6:</strong> Complete calibration when finished
            </div>
            
            <div class="warning">
                <strong>&#9888; WARNING:</strong> Always rinse sensors with distilled water between different solutions to prevent contamination!
            </div>
        </div>

        <div class="section">
            <h2>&#127777; Temperature Calibration</h2>
            <h3>Reference Solutions:</h3>
            <table>
                <tr><th>Reference Point</th><th>Temperature</th><th>Method</th><th>Accuracy</th></tr>
                <tr><td>Ice Point</td><td>0.0&#176;C</td><td>Ice + distilled water</td><td>&#177;0.1&#176;C</td></tr>
                <tr><td>Room Temperature</td><td>20-25&#176;C</td><td>Digital thermometer verification</td><td>&#177;0.5&#176;C</td></tr>
                <tr><td>Body Temperature</td><td>37.0&#176;C</td><td>Calibrated thermometer</td><td>&#177;0.2&#176;C</td></tr>
                <tr><td>Boiling Point</td><td>100.0&#176;C</td><td>Boiling distilled water (sea level)</td><td>&#177;1.0&#176;C</td></tr>
            </table>
            
            <h3>Procedure:</h3>
            <ol>
                <li>Use ice water (0&#176;C) for primary calibration point</li>
                <li>Optional: Add room temperature or body temperature point</li>
                <li>Optional: Use boiling water for high-range calibration</li>
                <li>Allow 30 seconds for temperature equilibration</li>
            </ol>
        </div>

        <div class="section">
            <h2>&#128167; pH Calibration</h2>
            <h3>Reference Buffer Solutions:</h3>
            <table>
                <tr><th>pH Value</th><th>Color Code</th><th>Use Case</th><th>Temperature Coefficient</th></tr>
                <tr><td>4.00</td><td>Red</td><td>Acidic calibration point</td><td>-0.0003 pH/&#176;C</td></tr>
                <tr><td>6.86 / 7.00</td><td>Yellow/Green</td><td>Neutral calibration point</td><td>0.0000 pH/&#176;C</td></tr>
                <tr><td>9.18 / 10.00</td><td>Blue</td><td>Alkaline calibration point</td><td>+0.0012 pH/&#176;C</td></tr>
            </table>
            
            <h3>Procedure:</h3>
            <ol>
                <li>Start with pH 7.00 buffer (neutral point)</li>
                <li>Add pH 4.00 for acidic range coverage</li>
                <li>Add pH 10.00 for alkaline range coverage</li>
                <li>Use fresh buffers (replace monthly)</li>
                <li>Allow 60 seconds for pH equilibration</li>
            </ol>
            
            <div class="warning">
                <strong>&#128161; TIP:</strong> Calibrate from low to high pH values to minimize cross-contamination.
            </div>
        </div>

        <div class="section">
            <h2>&#128168; TDS/EC Calibration</h2>
            <h3>Conductivity Standards:</h3>
            <table>
                <tr><th>Conductivity</th><th>TDS Equivalent</th><th>Use Case</th><th>Color Code</th></tr>
                <tr><td>0 &#181;S/cm</td><td>0 ppm</td><td>Zero point (distilled water)</td><td>Clear</td></tr>
                <tr><td>1,413 &#181;S/cm</td><td>~700 ppm</td><td>Low-range calibration</td><td>Usually clear</td></tr>
                <tr><td>12,880 &#181;S/cm</td><td>~6,400 ppm</td><td>High-range calibration</td><td>Usually clear</td></tr>
            </table>
            
            <h3>Procedure:</h3>
            <ol>
                <li>Start with distilled water (0 &#181;S/cm) for zero calibration</li>
                <li>Use 1,413 &#181;S/cm standard for low-range calibration</li>
                <li>Use 12,880 &#181;S/cm standard for high-range calibration</li>
                <li>Temperature affects conductivity: 2% per &#176;C</li>
                <li>Allow 30 seconds for conductivity equilibration</li>
            </ol>
        </div>

        <div class="section">
            <h2>&#128722; Reference Solutions - Amazon Links</h2>
            
            <div class="shopping-list">
                <h3>pH Buffer Solutions:</h3>
                <p><a href="https://www.amazon.com/s?k=pH+buffer+solution+4.00+7.00+10.00" class="amazon-link" target="_blank">pH Buffer Set (4.00, 7.00, 10.00) - Search on Amazon</a></p>
                <p><a href="https://www.amazon.com/s?k=pH+calibration+solution+packets" class="amazon-link" target="_blank">pH Calibration Solution Packets - Search on Amazon</a></p>
                
                <h3>TDS/EC Calibration Solutions:</h3>
                <p><a href="https://www.amazon.com/s?k=1413+microsiemens+conductivity+standard" class="amazon-link" target="_blank">1413 &#181;S/cm Conductivity Standard - Search on Amazon</a></p>
                <p><a href="https://www.amazon.com/s?k=12880+microsiemens+conductivity+standard" class="amazon-link" target="_blank">12880 &#181;S/cm Conductivity Standard - Search on Amazon</a></p>
                <p><a href="https://www.amazon.com/s?k=TDS+calibration+solution+1500+ppm" class="amazon-link" target="_blank">TDS Calibration Solutions - Search on Amazon</a></p>
                
                <h3>General Supplies:</h3>
                <p><a href="https://www.amazon.com/s?k=distilled+water+gallon" class="amazon-link" target="_blank">Distilled Water for Rinsing - Search on Amazon</a></p>
                <p><a href="https://www.amazon.com/s?k=digital+thermometer+aquarium" class="amazon-link" target="_blank">Reference Thermometer - Search on Amazon</a></p>
                <p><a href="https://www.amazon.com/s?k=small+beakers+glass+100ml" class="amazon-link" target="_blank">Small Glass Beakers (100ml) - Search on Amazon</a></p>
            </div>
        </div>

        <div class="section">
            <h2>&#128161; Best Practices</h2>
            
            <h3>Storage and Handling:</h3>
            <ul>
                <li><span class="highlight">Store pH buffers</span> in original containers, sealed tightly</li>
                <li><span class="highlight">Replace monthly</span> or when solutions become cloudy</li>
                <li><span class="highlight">Use small amounts</span> - pour into separate containers, don't dip sensors into main bottles</li>
                <li><span class="highlight">Temperature matters</span> - calibrate at the same temperature you'll be measuring</li>
            </ul>
            
            <h3>Calibration Frequency:</h3>
            <ul>
                <li><span class="highlight">pH sensors:</span> Weekly for critical applications, monthly for general use</li>
                <li><span class="highlight">TDS sensors:</span> Monthly or when readings seem inconsistent</li>
                <li><span class="highlight">Temperature sensors:</span> Quarterly or after system updates</li>
            </ul>
            
            <h3>Troubleshooting:</h3>
            <ul>
                <li><span class="highlight">Unstable readings:</span> Clean sensor, check for air bubbles, ensure proper immersion</li>
                <li><span class="highlight">Wrong values:</span> Check reference solution expiration, verify temperature compensation</li>
                <li><span class="highlight">Drift over time:</span> Sensor may need replacement or more frequent calibration</li>
            </ul>
        </div>

        <div class="section">
            <h2>&#9889; ESP32 ADC Information</h2>
            
            <h3>ADC Pin Compatibility:</h3>
            <table>
                <tr><th>ADC Channel</th><th>GPIO Pins</th><th>WiFi Compatibility</th><th>Recommended Use</th></tr>
                <tr><td>ADC1</td><td>32, 33, 34, 35, 36, 39</td><td>&#9989; WiFi Safe</td><td>Primary sensor connections</td></tr>
                <tr><td>ADC2</td><td>0, 2, 4, 12, 13, 14, 15, 25, 26, 27</td><td>&#9888; WiFi Conflicts</td><td>Secondary/backup sensors only</td></tr>
            </table>
            
            <p><strong>Note:</strong> This system uses ADC1 pins for reliable operation with WiFi enabled.</p>
        </div>

    </div>
</body>
</html>)HTML";
}

void AquaWebServer::handleDiagnosticsPage(AsyncWebServerRequest *request) {
  String html = generateDiagnosticsHTML();
  request->send(200, "text/html", html);
}

String AquaWebServer::generateDiagnosticsHTML() {
  return R"HTML(
<!DOCTYPE html>
<html>
<head>
    <title>System Diagnostics - ESP32 Aqua Monitor</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f8ff; }
        .container { max-width: 1200px; margin: 0 auto; }
        h1 { color: #2c5f91; text-align: center; }
        .nav-button { background: #6c757d; color: white; padding: 10px 20px; border: none; border-radius: 5px; margin: 5px; cursor: pointer; text-decoration: none; display: inline-block; }
        .nav-button:hover { background: #5a6268; }
        .diagnostic-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; margin: 20px 0; }
        .diagnostic-card { background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        .card-title { font-size: 1.2em; color: #2c5f91; margin-bottom: 15px; border-bottom: 2px solid #e0e0e0; padding-bottom: 10px; }
        .metric-row { display: flex; justify-content: space-between; margin: 8px 0; padding: 8px; border-radius: 5px; background: #f8f9fa; }
        .metric-label { font-weight: bold; }
        .metric-value { font-family: monospace; }
        .status-good { color: #28a745; font-weight: bold; }
        .status-warning { color: #ffc107; font-weight: bold; }
        .status-danger { color: #dc3545; font-weight: bold; }
        .refresh-button { background: #007bff; color: white; padding: 15px 30px; border: none; border-radius: 5px; font-size: 16px; cursor: pointer; margin: 20px 0; }
        .refresh-button:hover { background: #0056b3; }
        .timestamp { text-align: center; margin: 20px 0; color: #666; font-size: 0.9em; }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32 System Diagnostics</h1>
        
        <div style="text-align: center; margin-bottom: 20px;">
            <a href="/" class="nav-button">&#8962; Dashboard</a>
            <a href="/calibration" class="nav-button">&#9881; Calibration</a>
            <a href="/help" class="nav-button">&#10068; Help</a>
            <button onclick="refreshDiagnostics()" class="refresh-button">&#8635; Refresh Diagnostics</button>
        </div>
        
        <div class="diagnostic-grid" id="diagnostic-grid">
            <!-- Diagnostic cards will be populated here -->
        </div>
        
        <div class="timestamp" id="last-update">Loading diagnostics...</div>
    </div>

    <script>
        function refreshDiagnostics() {
            fetch('/api/status')
                .then(response => response.json())
                .then(data => {
                    updateDiagnosticDisplay(data);
                    document.getElementById('last-update').textContent = 
                        'Last updated: ' + new Date().toLocaleTimeString();
                })
                .catch(error => {
                    console.error('Error fetching diagnostics:', error);
                    document.getElementById('last-update').textContent = 'Error loading diagnostics';
                });
        }
        
        function updateDiagnosticDisplay(data) {
            const container = document.getElementById('diagnostic-grid');
            container.innerHTML = '';
            
            // System Information Card
            const systemCard = createDiagnosticCard('System Information', [
                { label: 'System', value: data.system },
                { label: 'Version', value: data.version },
                { label: 'Uptime', value: data.uptimeHours.toFixed(2) + ' hours' },
                { label: 'CPU Frequency', value: data.cpu.frequency + ' MHz' },
                { label: 'CPU Utilization', value: data.cpu.utilization.toFixed(1) + '%', status: getCpuUtilizationStatus(data.cpu.utilization) },
                { label: 'CPU Cores', value: data.cpu.cores },
                { label: 'Reset Reason', value: data.performance.resetReason }
            ]);
            container.appendChild(systemCard);
            
            // Memory Performance Card
            const memoryCard = createDiagnosticCard('Memory Performance', [
                { label: 'Free Heap', value: formatBytes(data.memory.freeHeap), status: getMemoryStatus(data.memory.heapUsagePercent) },
                { label: 'Used Heap', value: formatBytes(data.memory.usedHeap) },
                { label: 'Total Heap', value: formatBytes(data.memory.totalHeap) },
                { label: 'Usage', value: data.memory.heapUsagePercent.toFixed(1) + '%', status: getMemoryStatus(data.memory.heapUsagePercent) },
                { label: 'Min Free Heap', value: formatBytes(data.memory.minFreeHeap) },
                { label: 'Max Alloc', value: formatBytes(data.memory.maxAllocHeap) }
            ]);
            container.appendChild(memoryCard);
            
            // WiFi Performance Card
            const wifiCard = createDiagnosticCard('WiFi Performance', [
                { label: 'SSID', value: data.wifi.ssid },
                { label: 'IP Address', value: data.wifi.ip },
                { label: 'Signal Strength', value: data.wifi.rssi + ' dBm', status: getWiFiStatus(data.wifi.rssi) },
                { label: 'MAC Address', value: data.wifi.mac },
                { label: 'Channel', value: data.wifi.channel }
            ]);
            container.appendChild(wifiCard);
            
            // Flash Storage Card
            const flashCard = createDiagnosticCard('Flash Storage', [
                { label: 'Size', value: formatBytes(data.flash.size) },
                { label: 'Speed', value: data.flash.speed + ' Hz' },
                { label: 'Mode', value: data.flash.mode }
            ]);
            container.appendChild(flashCard);
            
            // Sensor Status Card
            const sensorCard = createDiagnosticCard('Sensor Status', [
                { label: 'Temperature Sensors', value: data.sensors.temperature },
                { label: 'pH Sensors', value: data.sensors.ph },
                { label: 'TDS Sensors', value: data.sensors.tds },
                { label: 'Overall Status', value: data.sensors.status, status: data.sensors.status === 'active' ? 'good' : 'danger' }
            ]);
            container.appendChild(sensorCard);
            
            // System Health Card
            const healthCard = createDiagnosticCard('System Health', [
                { label: 'Overall Health', value: data.health.overall, status: data.health.overall },
                { label: 'Memory Health', value: data.health.memory, status: data.health.memory },
                { label: 'CPU Health', value: data.health.cpu, status: data.health.cpu },
                { label: 'WiFi Health', value: data.health.wifi, status: data.health.wifi },
                { label: 'Uptime Health', value: data.health.uptime, status: data.health.uptime },
                { label: 'Stack Water Mark', value: data.performance.taskStackHighWaterMark + ' bytes' }
            ]);
            container.appendChild(healthCard);
        }
        
        function createDiagnosticCard(title, metrics) {
            const card = document.createElement('div');
            card.className = 'diagnostic-card';
            
            let html = '<div class="card-title">' + title + '</div>';
            metrics.forEach(metric => {
                const statusClass = metric.status ? 'status-' + metric.status : '';
                html += '<div class="metric-row">';
                html += '<span class="metric-label">' + metric.label + ':</span>';
                html += '<span class="metric-value ' + statusClass + '">' + metric.value + '</span>';
                html += '</div>';
            });
            
            card.innerHTML = html;
            return card;
        }
        
        function formatBytes(bytes) {
            if (bytes === 0) return '0 Bytes';
            const k = 1024;
            const sizes = ['Bytes', 'KB', 'MB', 'GB'];
            const i = Math.floor(Math.log(bytes) / Math.log(k));
            return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
        }
        
        function getMemoryStatus(usagePercent) {
            if (usagePercent < 60) return 'good';
            if (usagePercent < 80) return 'warning';
            return 'danger';
        }
        
        function getCpuUtilizationStatus(utilization) {
            if (utilization < 50) return 'good';
            if (utilization < 80) return 'warning';
            return 'danger';
        }
        
        function getWiFiStatus(rssi) {
            if (rssi > -50) return 'good';
            if (rssi > -70) return 'warning';
            return 'danger';
        }
        
        // Load diagnostics on page load
        document.addEventListener('DOMContentLoaded', function() {
            refreshDiagnostics();
        });
        
        // Auto-refresh every 10 seconds
        setInterval(refreshDiagnostics, 10000);
    </script>
</body>
</html>)HTML";
}