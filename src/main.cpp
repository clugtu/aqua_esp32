#include <Arduino.h>
#include "Config.h"
#include "ConfigManager.h"
#include "SensorController.h"
#include "NetworkManager.h"
#include "AquaWebServer.h"
#include "CalibrationManager.h"
#include "IconPolicy.h"

// Create global objects
ConfigManager configMgr;
SensorController sensors;
NetworkManager network;
AquaWebServer webServer;
CalibrationManager calibrationMgr;

// CPU utilization monitoring variables
unsigned long lastCpuUpdate = 0;
unsigned long totalLoopTime = 0;
unsigned long activeTime = 0;
unsigned long loopCount = 0;
float cpuUtilization = 0.0;
const unsigned long CPU_UPDATE_INTERVAL = 5000; // Update CPU stats every 5 seconds

// Function to get CPU utilization (accessible from other files)
float getCpuUtilization() {
  return cpuUtilization;
}

void setup() {
  // Initialize serial communication (using default first, then config)
  Serial.begin(DEFAULT_SERIAL_BAUD_RATE);
  
  // Wait for serial port to connect (useful for debugging)
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  
  // Display NO ICONS policy
  Serial.println(SAFE_SEPARATOR);
  Serial.println("ESP32 AQUA MONITORING SYSTEM - NO ICONS POLICY");
  Serial.println("This system uses ASCII-only output for maximum compatibility");
  Serial.println("NO emoji, Unicode symbols, or special characters are used");
  Serial.println(SAFE_SEPARATOR);
  Serial.println();
  
  // Load configuration from JSON file
  Serial.println("Loading configuration...");
  if (!configMgr.begin()) {
    Serial.println("Warning: Using default configuration (config.json not found)");
  } else {
    configMgr.printConfig();
  }
  
  // Initialize the LED pin as an output  
  pinMode(configMgr.getLedPin(), OUTPUT);
  
  // Print startup message
  Serial.println();
  Serial.println("=========================================");
  Serial.println("ESP32 Aqua Monitoring System Started!");
  Serial.println("=========================================");
  Serial.println("Board: ESP32-DevKitC-32");
  Serial.println("Chip: " + String(ESP.getChipModel()));
  Serial.println("Flash Size: " + String(ESP.getFlashChipSize() / (1024 * 1024)) + " MB");
  Serial.println("Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
  Serial.println();
  
  // Initialize sensor controller
  Serial.println("Initializing Sensor Controller...");
  sensors.begin();
  Serial.println("Sensor Controller initialized successfully");
  Serial.println();
  
  // Initialize and connect to WiFi
  network.begin(&configMgr);
  Serial.println("Connecting to WiFi...");
  if (network.connect()) {
    Serial.println("WiFi connected successfully");
    network.printConnectionDetails();
  } else {
    Serial.println("WiFi connection failed");
  }
  Serial.println();
  
  // Initialize calibration manager
  Serial.println("Initializing Calibration Manager...");
  if (calibrationMgr.begin()) {
    Serial.println("Calibration Manager initialized successfully");
  } else {
    Serial.println("Warning: Calibration Manager initialization failed");
  }
  Serial.println();

  // Initialize web server
  Serial.println("Initializing Web Server...");
  webServer.begin(&sensors, &calibrationMgr);
  Serial.println("Web Server started");
  Serial.println("Access dashboard at: http://" + network.getIP() + "/");
  Serial.println("API endpoint: http://" + network.getIP() + "/api/sensors");
  Serial.println("Calibration page: http://" + network.getIP() + "/calibration");
  Serial.println();
  
  Serial.println("System Ready! Starting main loop...");
  Serial.println("=========================================");
}

void loop() {
  static unsigned long lastUpdate = 0;
  static unsigned long lastPrint = 0;
  
  // CPU utilization monitoring - start timing
  unsigned long loopStartTime = micros();
  
  int sensorInterval = configMgr.getSensorReadInterval();
  int printInterval = configMgr.getPrintInterval();
  
  // Update sensor readings every configured interval
  if (millis() - lastUpdate >= sensorInterval) {
    sensors.updateAllReadings();
    lastUpdate = millis();
  }
  
  // Print sensor values every configured interval  
  if (millis() - lastPrint >= printInterval) {
    Serial.println();
    Serial.println("Current Sensor Readings:");
    Serial.println("----------------------------");
    
    // Print temperature readings
    Serial.println("Temperature Sensors:");
    for (int i = 0; i < sensors.getTemperatureSensors().getSensorCount(); i++) {
      float temp = sensors.getTemperatureSensors().getData().readings[i];
      Serial.printf("    Temp%d: %.2fC\n", i + 1, temp);
    }
    
    Serial.println();
    
    // Print pH readings
    Serial.println("pH Sensors:");
    for (int i = 0; i < sensors.getPHSensors().getSensorCount(); i++) {
      float ph = sensors.getPHSensors().getData().readings[i];
      Serial.printf("    pH%d: %.2f\n", i + 1, ph);
    }
    
    Serial.println();
    
    // Print TDS readings
    Serial.println("TDS Sensors:");
    for (int i = 0; i < sensors.getTDSSensors().getSensorCount(); i++) {
      float tds = sensors.getTDSSensors().getData().readings[i];
      float ec = sensors.getTDSSensors().getECReading(i);
      Serial.printf("    TDS%d: %.2f ppm / %.2f uS/cm\n", i + 1, tds, ec);
    }
    
    Serial.println();
    
    // Print WiFi status and network information
    Serial.println("WiFi Status:");
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("    Connected to: %s\n", WiFi.SSID().c_str());
      Serial.printf("    IP Address: %s\n", WiFi.localIP().toString().c_str());
      Serial.printf("    Signal Strength: %d dBm\n", WiFi.RSSI());
      Serial.printf("    Web Dashboard: http://%s/\n", WiFi.localIP().toString().c_str());
      Serial.printf("    API Endpoint: http://%s/api/sensors\n", WiFi.localIP().toString().c_str());
    } else {
      Serial.println("    WiFi Disconnected");
      Serial.printf("    Status Code: %d\n", WiFi.status());
    }
    
    // Enhanced system monitoring
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t usedHeap = totalHeap - freeHeap;
    float heapUsage = ((float)usedHeap / totalHeap) * 100;
    
    Serial.println("System Performance:");
    Serial.printf("    Free Memory: %d bytes (%.1f%% used)\n", freeHeap, heapUsage);
    Serial.printf("    Min Free Heap: %d bytes\n", ESP.getMinFreeHeap());
    Serial.printf("    CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("    CPU Utilization: %.1f%%\n", cpuUtilization);
    Serial.printf("    Uptime: %.2f hours\n", millis() / 3600000.0);
    Serial.printf("    Stack High Water: %d bytes\n", uxTaskGetStackHighWaterMark(NULL));
    
    // Performance warnings
    if (heapUsage > 80.0) {
      Serial.println("    WARNING: High memory usage detected!");
    }
    if (freeHeap < 10000) {
      Serial.println("    WARNING: Low free memory!");
    }
    if (WiFi.RSSI() < -70) {
      Serial.println("    WARNING: Weak WiFi signal!");
    }
    if (cpuUtilization > 80.0) {
      Serial.println("    WARNING: High CPU utilization detected!");
    }
    
    Serial.println("=========================================");
    
    lastPrint = millis();
  }
  
  // CPU utilization monitoring - end timing and calculate
  unsigned long loopEndTime = micros();
  unsigned long loopDuration = loopEndTime - loopStartTime;
  
  // Accumulate timing data
  totalLoopTime += loopDuration;
  activeTime += loopDuration - 10000; // Subtract the delay(10) = 10ms = 10000us
  loopCount++;
  
  // Calculate CPU utilization every CPU_UPDATE_INTERVAL
  if (millis() - lastCpuUpdate >= CPU_UPDATE_INTERVAL) {
    if (loopCount > 0) {
      unsigned long avgLoopTime = totalLoopTime / loopCount;
      unsigned long avgActiveTime = activeTime / loopCount;
      
      // CPU utilization = (active time / total time) * 100
      // We estimate total available time as the measurement interval
      unsigned long intervalMicros = CPU_UPDATE_INTERVAL * 1000;
      unsigned long totalActiveTimeMicros = avgActiveTime * loopCount;
      cpuUtilization = ((float)totalActiveTimeMicros / intervalMicros) * 100.0;
      
      // Clamp between 0 and 100
      if (cpuUtilization < 0) cpuUtilization = 0;
      if (cpuUtilization > 100) cpuUtilization = 100;
    }
    
    // Reset counters
    totalLoopTime = 0;
    activeTime = 0;
    loopCount = 0;
    lastCpuUpdate = millis();
  }
  
  // Small delay to prevent watchdog issues
  delay(10);
}