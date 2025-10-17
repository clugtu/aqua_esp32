#include "ConfigManager.h"

ConfigManager::ConfigManager() : configLoaded(false) {
}

bool ConfigManager::begin() {
  if (!initSPIFFS()) {
    Serial.println("Failed to initialize SPIFFS");
    return false;
  }
  
  if (!loadConfigFile()) {
    Serial.println("Failed to load configuration file");
    return false;
  }
  
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
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }
  
  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    configFile.close();
    return false;
  }
  
  // Read file content
  String content = configFile.readString();
  configFile.close();
  
  // Parse JSON
  DeserializationError error = deserializeJson(config, content);
  if (error) {
    Serial.print("Failed to parse config file: ");
    Serial.println(error.c_str());
    return false;
  }
  
  return true;
}

// WiFi configuration
String ConfigManager::getWifiSSID() {
  return configLoaded ? config["wifi"]["ssid"].as<String>() : "DEFAULT_SSID";
}

String ConfigManager::getWifiPassword() {
  return configLoaded ? config["wifi"]["password"].as<String>() : "DEFAULT_PASSWORD";
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
  Serial.println("========================");
}