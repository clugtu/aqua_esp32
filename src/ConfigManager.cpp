#include "ConfigManager.h"
#include "Config.h"

ConfigManager::ConfigManager() : configLoaded(false) {
}

bool ConfigManager::begin() {
  Serial.println("DEBUG: ConfigManager::begin() called");
  
  if (!initSPIFFS()) {
    Serial.println("Failed to initialize SPIFFS");
    return false;
  }
  Serial.println("DEBUG: SPIFFS initialized successfully");
  
  if (!loadConfigFile()) {
    Serial.println("Failed to load configuration file");
    return false;
  }
  Serial.println("DEBUG: Config file loaded successfully");
  
  configLoaded = true;
  Serial.println("Configuration loaded successfully");
  return true;
}

bool ConfigManager::initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
    return false;
  }
  return true;
}

bool ConfigManager::loadConfigFile() {
  Serial.println("DEBUG: Attempting to open /config.json");
  
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("DEBUG: Failed to open /config.json, trying to list SPIFFS contents...");
    
    // List SPIFFS contents for debugging
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while(file) {
      Serial.printf("DEBUG: SPIFFS file found: %s (size: %d)\n", file.name(), file.size());
      file = root.openNextFile();
    }
    
    Serial.println("Failed to open config file");
    return false;
  }
  
  size_t size = configFile.size();
  Serial.printf("DEBUG: Config file size: %d bytes\n", size);
  
  if (size > 4096) {
    Serial.println("Config file size is too large");
    configFile.close();
    return false;
  }
  
  // Read file content
  String content = configFile.readString();
  configFile.close();
  
  Serial.printf("DEBUG: Config file content length: %d\n", content.length());
  Serial.println("DEBUG: Config file content:");
  Serial.println(content);
  
  // Parse JSON
  DeserializationError error = deserializeJson(config, content);
  if (error) {
    Serial.print("Failed to parse config file: ");
    Serial.println(error.c_str());
    return false;
  }
  
  Serial.println("DEBUG: JSON parsed successfully");
  return true;
}

// WiFi configuration
String ConfigManager::getWifiSSID() {
  Serial.printf("DEBUG: getWifiSSID() called, configLoaded = %s\n", configLoaded ? "true" : "false");
  if (configLoaded) {
    Serial.println("DEBUG: Checking config WiFi section...");
    if (config.containsKey("wifi")) {
      Serial.println("DEBUG: WiFi section found");
      if (config["wifi"].containsKey("ssid")) {
        String ssid = config["wifi"]["ssid"].as<String>();
        Serial.printf("DEBUG: WiFi SSID from config: '%s'\n", ssid.c_str());
        return ssid;
      } else {
        Serial.println("DEBUG: WiFi ssid key not found in config");
        return "DEFAULT_SSID";
      }
    } else {
      Serial.println("DEBUG: WiFi section not found in config");
      return "DEFAULT_SSID";
    }
  } else {
    Serial.println("DEBUG: Using default WiFi SSID (config not loaded)");
    return "DEFAULT_SSID";
  }
}

String ConfigManager::getWifiPassword() {
  Serial.printf("DEBUG: getWifiPassword() called, configLoaded = %s\n", configLoaded ? "true" : "false");
  if (configLoaded) {
    String password = config["wifi"]["password"].as<String>();
    Serial.printf("DEBUG: WiFi password from config: '%s'\n", password.c_str());
    return password;
  } else {
    Serial.println("DEBUG: Using default WiFi password");
    return "DEFAULT_PASSWORD";
  }
}

// System configuration  
String ConfigManager::getDeviceName() {
  return configLoaded ? config["system"]["device_name"].as<String>() : "ESP32 Device";
}

int ConfigManager::getSerialBaud() {
  return configLoaded ? config["system"]["serial_baud"].as<int>() : 115200;
}

int ConfigManager::getSensorReadInterval() {
  return configLoaded ? config["system"]["sensor_read_interval"].as<int>() : 5000;
}

int ConfigManager::getPrintInterval() {
  return configLoaded ? config["system"]["print_interval"].as<int>() : 5000;
}

// NO ICONS policy configuration
bool ConfigManager::getUseIcons() {
  return configLoaded ? config["system"]["use_icons"].as<bool>() : false;  // Always false
}

bool ConfigManager::getUseEmoji() {
  return configLoaded ? config["system"]["use_emoji"].as<bool>() : false;  // Always false
}

bool ConfigManager::getAsciiOnly() {
  return configLoaded ? config["system"]["ascii_only"].as<bool>() : true;  // Always true
}

String ConfigManager::getOutputPolicy() {
  return configLoaded ? config["system"]["output_policy"].as<String>() : "NO_ICONS_EVER";
}

// Sensor configuration
int ConfigManager::getTemperatureCount() {
  return configLoaded ? config["sensors"]["temperature_count"].as<int>() : 8;
}

int ConfigManager::getPHCount() {
  return configLoaded ? config["sensors"]["ph_count"].as<int>() : 8;
}

int ConfigManager::getTDSCount() {
  return configLoaded ? config["sensors"]["tds_count"].as<int>() : 8;
}

// Hardware configuration
int ConfigManager::getLedPin() {
  return configLoaded ? config["hardware"]["led_pin"].as<int>() : 2;
}

int ConfigManager::getTempAdcPin() {
  return configLoaded ? config["hardware"]["temp_adc_pin"].as<int>() : 32;
}

int ConfigManager::getPHAdcPin() {
  return configLoaded ? config["hardware"]["ph_adc_pin"].as<int>() : 33;
}

int ConfigManager::getTDSAdcPin() {
  return configLoaded ? config["hardware"]["tds_adc_pin"].as<int>() : 35;
}

int ConfigManager::getMuxS0() {
  return configLoaded ? config["hardware"]["mux_s0"].as<int>() : 4;
}

int ConfigManager::getMuxS1() {
  return configLoaded ? config["hardware"]["mux_s1"].as<int>() : 5;
}

int ConfigManager::getMuxS2() {
  return configLoaded ? config["hardware"]["mux_s2"].as<int>() : 18;
}

int ConfigManager::getMuxS3() {
  return configLoaded ? config["hardware"]["mux_s3"].as<int>() : 19;
}

int ConfigManager::getMuxEnable() {
  return configLoaded ? config["hardware"]["mux_enable"].as<int>() : 21;
}

// Utility methods
bool ConfigManager::isLoaded() {
  return configLoaded;
}

void ConfigManager::printConfig() {
  if (!configLoaded) {
    Serial.println("No configuration loaded");
    return;
  }
  
  Serial.println("Current Configuration:");
  Serial.println("========================");
  Serial.println("WiFi Settings:");
  Serial.printf("  SSID: %s\n", getWifiSSID().c_str());
  Serial.printf("  Password: %s\n", String("*").c_str()); // Hide password
  Serial.println();
  
  Serial.println("System Settings:");
  Serial.printf("  Device Name: %s\n", getDeviceName().c_str());
  Serial.printf("  Serial Baud: %d\n", getSerialBaud());
  Serial.printf("  Sensor Read Interval: %dms\n", getSensorReadInterval());
  Serial.printf("  Print Interval: %dms\n", getPrintInterval());
  Serial.println();
  
  Serial.println("Output Policy:");
  Serial.printf("  Use Icons: %s\n", getUseIcons() ? "true" : "false");
  Serial.printf("  Use Emoji: %s\n", getUseEmoji() ? "true" : "false");
  Serial.printf("  ASCII Only: %s\n", getAsciiOnly() ? "true" : "false");
  Serial.printf("  Policy: %s\n", getOutputPolicy().c_str());
  Serial.println();
  
  Serial.println("Sensor Configuration:");
  Serial.printf("  Temperature Sensors: %d\n", getTemperatureCount());
  Serial.printf("  pH Sensors: %d\n", getPHCount());
  Serial.printf("  TDS Sensors: %d\n", getTDSCount());
  Serial.println();
  
  Serial.println("Hardware Pins:");
  Serial.printf("  LED Pin: %d\n", getLedPin());
  Serial.printf("  Temperature ADC: %d\n", getTempAdcPin());
  Serial.printf("  pH ADC: %d\n", getPHAdcPin());
  Serial.printf("  TDS ADC: %d\n", getTDSAdcPin());
  Serial.printf("  Multiplexer Control: S0=%d, S1=%d, S2=%d, S3=%d\n", 
                getMuxS0(), getMuxS1(), getMuxS2(), getMuxS3());
  Serial.printf("  Multiplexer Enable: %d\n", getMuxEnable());
  
  Serial.println();
  Serial.println("Security Configuration:");
  Serial.printf("  Admin Username: %s\n", getAdminUsername().c_str());
  Serial.printf("  SSL Enabled: %s\n", isSSLEnabled() ? "true" : "false");
  Serial.printf("  HTTPS Port: %d\n", getHTTPSPort());
  Serial.printf("  HTTP Port: %d\n", getHTTPPort());
  
  Serial.println();
  Serial.printf("Aquariums (%d configured):\n", getAquariumCount());
  for (int i = 0; i < getAquariumCount(); i++) {
    Serial.printf("  [%d] %s (%s) - %s\n", i, 
                  getAquariumName(i).c_str(),
                  getAquariumID(i).c_str(),
                  isAquariumEnabled(i) ? "Enabled" : "Disabled");
  }
  Serial.println("========================");
}

// Security configuration methods
String ConfigManager::getAdminUsername() {
  Serial.printf("Debug: configLoaded = %s\n", configLoaded ? "true" : "false");
  if (configLoaded) {
    String username = config["security"]["admin_username"].as<String>();
    Serial.printf("Debug: Admin username from config: '%s'\n", username.c_str());
    return username;
  } else {
    Serial.printf("Debug: Using default admin username: '%s'\n", DEFAULT_ADMIN_USERNAME);
    return DEFAULT_ADMIN_USERNAME;
  }
}

String ConfigManager::getAdminPassword() {
  if (configLoaded) {
    String password = config["security"]["admin_password"].as<String>();
    Serial.printf("Debug: Admin password from config: '%s'\n", password.c_str());
    return password;
  } else {
    Serial.printf("Debug: Using default admin password: '%s'\n", DEFAULT_ADMIN_PASSWORD);
    return DEFAULT_ADMIN_PASSWORD;
  }
}

bool ConfigManager::isSSLEnabled() {
  return configLoaded ? config["security"]["ssl_enabled"].as<bool>() : DEFAULT_SSL_ENABLED;
}

int ConfigManager::getHTTPSPort() {
  return configLoaded ? config["security"]["ssl_port"].as<int>() : DEFAULT_HTTPS_PORT;
}

int ConfigManager::getHTTPPort() {
  return configLoaded ? config["security"]["http_port"].as<int>() : DEFAULT_HTTP_PORT;
}

// Aquarium management methods
int ConfigManager::getAquariumCount() {
  if (!configLoaded || !config["aquariums"].is<JsonArray>()) {
    return 0;
  }
  return config["aquariums"].size();
}

String ConfigManager::getAquariumName(int index) {
  if (!configLoaded || index < 0 || index >= getAquariumCount()) {
    return "Unknown";
  }
  return config["aquariums"][index]["name"].as<String>();
}

String ConfigManager::getAquariumID(int index) {
  if (!configLoaded || index < 0 || index >= getAquariumCount()) {
    return "unknown";
  }
  return config["aquariums"][index]["id"].as<String>();
}

String ConfigManager::getAquariumDescription(int index) {
  if (!configLoaded || index < 0 || index >= getAquariumCount()) {
    return "";
  }
  return config["aquariums"][index]["description"].as<String>();
}

bool ConfigManager::isAquariumEnabled(int index) {
  if (!configLoaded || index < 0 || index >= getAquariumCount()) {
    return false;
  }
  return config["aquariums"][index]["enabled"].as<bool>();
}

// Sensor range methods
float ConfigManager::getTemperatureMin(int aquariumIndex) {
  if (!configLoaded || aquariumIndex < 0 || aquariumIndex >= getAquariumCount()) {
    return DEFAULT_TEMP_MIN;
  }
  return config["aquariums"][aquariumIndex]["sensors"]["temperature"]["normal_range"]["min"].as<float>();
}

float ConfigManager::getTemperatureMax(int aquariumIndex) {
  if (!configLoaded || aquariumIndex < 0 || aquariumIndex >= getAquariumCount()) {
    return DEFAULT_TEMP_MAX;
  }
  return config["aquariums"][aquariumIndex]["sensors"]["temperature"]["normal_range"]["max"].as<float>();
}

float ConfigManager::getPHMin(int aquariumIndex) {
  if (!configLoaded || aquariumIndex < 0 || aquariumIndex >= getAquariumCount()) {
    return DEFAULT_PH_MIN;
  }
  return config["aquariums"][aquariumIndex]["sensors"]["ph"]["normal_range"]["min"].as<float>();
}

float ConfigManager::getPHMax(int aquariumIndex) {
  if (!configLoaded || aquariumIndex < 0 || aquariumIndex >= getAquariumCount()) {
    return DEFAULT_PH_MAX;
  }
  return config["aquariums"][aquariumIndex]["sensors"]["ph"]["normal_range"]["max"].as<float>();
}

float ConfigManager::getTDSMin(int aquariumIndex) {
  if (!configLoaded || aquariumIndex < 0 || aquariumIndex >= getAquariumCount()) {
    return DEFAULT_TDS_MIN;
  }
  return config["aquariums"][aquariumIndex]["sensors"]["tds"]["normal_range"]["min"].as<float>();
}

float ConfigManager::getTDSMax(int aquariumIndex) {
  if (!configLoaded || aquariumIndex < 0 || aquariumIndex >= getAquariumCount()) {
    return DEFAULT_TDS_MAX;
  }
  return config["aquariums"][aquariumIndex]["sensors"]["tds"]["normal_range"]["max"].as<float>();
}

// Sensor assignment methods
int ConfigManager::getTemperatureSensorCount(int aquariumIndex) {
  if (!configLoaded || aquariumIndex < 0 || aquariumIndex >= getAquariumCount()) {
    return 0;
  }
  JsonArray sensors = config["aquariums"][aquariumIndex]["sensors"]["temperature"]["sensor_ids"];
  return sensors.size();
}

int ConfigManager::getTemperatureSensorID(int aquariumIndex, int sensorIndex) {
  if (!configLoaded || aquariumIndex < 0 || aquariumIndex >= getAquariumCount()) {
    return -1;
  }
  JsonArray sensors = config["aquariums"][aquariumIndex]["sensors"]["temperature"]["sensor_ids"];
  if (sensorIndex < 0 || sensorIndex >= (int)sensors.size()) {
    return -1;
  }
  return sensors[sensorIndex].as<int>();
}

int ConfigManager::getPHSensorCount(int aquariumIndex) {
  if (!configLoaded || aquariumIndex < 0 || aquariumIndex >= getAquariumCount()) {
    return 0;
  }
  JsonArray sensors = config["aquariums"][aquariumIndex]["sensors"]["ph"]["sensor_ids"];
  return sensors.size();
}

int ConfigManager::getPHSensorID(int aquariumIndex, int sensorIndex) {
  if (!configLoaded || aquariumIndex < 0 || aquariumIndex >= getAquariumCount()) {
    return -1;
  }
  JsonArray sensors = config["aquariums"][aquariumIndex]["sensors"]["ph"]["sensor_ids"];
  if (sensorIndex < 0 || sensorIndex >= (int)sensors.size()) {
    return -1;
  }
  return sensors[sensorIndex].as<int>();
}

int ConfigManager::getTDSSensorCount(int aquariumIndex) {
  if (!configLoaded || aquariumIndex < 0 || aquariumIndex >= getAquariumCount()) {
    return 0;
  }
  JsonArray sensors = config["aquariums"][aquariumIndex]["sensors"]["tds"]["sensor_ids"];
  return sensors.size();
}

int ConfigManager::getTDSSensorID(int aquariumIndex, int sensorIndex) {
  if (!configLoaded || aquariumIndex < 0 || aquariumIndex >= getAquariumCount()) {
    return -1;
  }
  JsonArray sensors = config["aquariums"][aquariumIndex]["sensors"]["tds"]["sensor_ids"];
  if (sensorIndex < 0 || sensorIndex >= (int)sensors.size()) {
    return -1;
  }
  return sensors[sensorIndex].as<int>();
}

// Range checking utilities
bool ConfigManager::isTemperatureInRange(int aquariumIndex, float value) {
  float min = getTemperatureMin(aquariumIndex);
  float max = getTemperatureMax(aquariumIndex);
  return (value >= min && value <= max);
}

bool ConfigManager::isPHInRange(int aquariumIndex, float value) {
  float min = getPHMin(aquariumIndex);
  float max = getPHMax(aquariumIndex);
  return (value >= min && value <= max);
}

bool ConfigManager::isTDSInRange(int aquariumIndex, float value) {
  float min = getTDSMin(aquariumIndex);
  float max = getTDSMax(aquariumIndex);
  return (value >= min && value <= max);
}