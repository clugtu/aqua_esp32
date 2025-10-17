#include "CalibrationManager.h"
#include "SPIFFS.h"
#include <time.h>

CalibrationManager::CalibrationManager() : dataLoaded(false) {
  // Initialize all calibration data as invalid
  memset(&calibrationData, 0, sizeof(calibrationData));
}

bool CalibrationManager::begin() {
  if (!SPIFFS.begin()) {
    Serial.println("[CAL] Failed to mount SPIFFS");
    return false;
  }
  
  if (!loadCalibrationData()) {
    Serial.println("[CAL] No existing calibration data found, starting fresh");
  }
  
  return true;
}

bool CalibrationManager::loadCalibrationData() {
  File file = SPIFFS.open("/calibration.json", "r");
  if (!file) {
    Serial.println("[CAL] Calibration file not found");
    return false;
  }
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    Serial.printf("[CAL] Failed to parse calibration file: %s\n", error.c_str());
    return false;
  }
  
  // Load temperature calibrations
  if (doc.containsKey("temperature")) {
    JsonArray tempArray = doc["temperature"];
    for (int i = 0; i < 8 && i < tempArray.size(); i++) {
      JsonObject tempCal = tempArray[i];
      TemperatureCalibration& cal = calibrationData.temperature[i];
      
      if (tempCal.containsKey("isCalibrated")) {
        cal.isCalibrated = tempCal["isCalibrated"];
        cal.slope = tempCal["slope"] | 1.0;
        cal.offset = tempCal["offset"] | 0.0;
        cal.calibrationDate = tempCal["date"] | "";
        cal.notes = tempCal["notes"] | "";
        
        // Load calibration points
        if (tempCal.containsKey("point1")) {
          JsonObject p1 = tempCal["point1"];
          cal.point1.rawValue = p1["raw"];
          cal.point1.actualValue = p1["actual"];
          cal.point1.temperature = p1["temp"];
          cal.point1.valid = p1["valid"];
        }
        
        if (tempCal.containsKey("point2")) {
          JsonObject p2 = tempCal["point2"];
          cal.point2.rawValue = p2["raw"];
          cal.point2.actualValue = p2["actual"];
          cal.point2.temperature = p2["temp"];
          cal.point2.valid = p2["valid"];
        }
      }
    }
  }
  
  // Load pH calibrations
  if (doc.containsKey("ph")) {
    JsonArray phArray = doc["ph"];
    for (int i = 0; i < 8 && i < phArray.size(); i++) {
      JsonObject phCal = phArray[i];
      PHCalibration& cal = calibrationData.ph[i];
      
      if (phCal.containsKey("isCalibrated")) {
        cal.isCalibrated = phCal["isCalibrated"];
        cal.slope = phCal["slope"] | 1.0;
        cal.offset = phCal["offset"] | 0.0;
        cal.tempCoeff = phCal["tempCoeff"] | CalibrationStandards::TEMP_COEFF_PH;
        cal.calibrationDate = phCal["date"] | "";
        cal.notes = phCal["notes"] | "";
        
        // Load calibration points
        if (phCal.containsKey("point1")) {
          JsonObject p1 = phCal["point1"];
          cal.point1.rawValue = p1["raw"];
          cal.point1.actualValue = p1["actual"];
          cal.point1.temperature = p1["temp"];
          cal.point1.valid = p1["valid"];
        }
        
        if (phCal.containsKey("point2")) {
          JsonObject p2 = phCal["point2"];
          cal.point2.rawValue = p2["raw"];
          cal.point2.actualValue = p2["actual"];
          cal.point2.temperature = p2["temp"];
          cal.point2.valid = p2["valid"];
        }
        
        if (phCal.containsKey("point3")) {
          JsonObject p3 = phCal["point3"];
          cal.point3.rawValue = p3["raw"];
          cal.point3.actualValue = p3["actual"];
          cal.point3.temperature = p3["temp"];
          cal.point3.valid = p3["valid"];
        }
      }
    }
  }
  
  // Load TDS calibrations
  if (doc.containsKey("tds")) {
    JsonArray tdsArray = doc["tds"];
    for (int i = 0; i < 8 && i < tdsArray.size(); i++) {
      JsonObject tdsCal = tdsArray[i];
      TDSCalibration& cal = calibrationData.tds[i];
      
      if (tdsCal.containsKey("isCalibrated")) {
        cal.isCalibrated = tdsCal["isCalibrated"];
        cal.kFactor = tdsCal["kFactor"] | 1.0;
        cal.slope = tdsCal["slope"] | 1.0;
        cal.offset = tdsCal["offset"] | 0.0;
        cal.tempCoeff = tdsCal["tempCoeff"] | CalibrationStandards::TEMP_COEFF_EC;
        cal.calibrationDate = tdsCal["date"] | "";
        cal.notes = tdsCal["notes"] | "";
        
        // Load calibration points
        if (tdsCal.containsKey("point1")) {
          JsonObject p1 = tdsCal["point1"];
          cal.point1.rawValue = p1["raw"];
          cal.point1.actualValue = p1["actual"];
          cal.point1.temperature = p1["temp"];
          cal.point1.valid = p1["valid"];
        }
        
        if (tdsCal.containsKey("point2")) {
          JsonObject p2 = tdsCal["point2"];
          cal.point2.rawValue = p2["raw"];
          cal.point2.actualValue = p2["actual"];
          cal.point2.temperature = p2["temp"];
          cal.point2.valid = p2["valid"];
        }
      }
    }
  }
  
  dataLoaded = true;
  Serial.println("[CAL] Calibration data loaded successfully");
  return true;
}

bool CalibrationManager::saveCalibrationData() {
  JsonDocument doc;
  
  // Save temperature calibrations
  JsonArray tempArray = doc["temperature"].to<JsonArray>();
  for (int i = 0; i < 8; i++) {
    JsonObject tempCal = tempArray.add<JsonObject>();
    const TemperatureCalibration& cal = calibrationData.temperature[i];
    
    tempCal["isCalibrated"] = cal.isCalibrated;
    tempCal["slope"] = cal.slope;
    tempCal["offset"] = cal.offset;
    tempCal["date"] = cal.calibrationDate;
    tempCal["notes"] = cal.notes;
    
    JsonObject p1 = tempCal["point1"].to<JsonObject>();
    p1["raw"] = cal.point1.rawValue;
    p1["actual"] = cal.point1.actualValue;
    p1["temp"] = cal.point1.temperature;
    p1["valid"] = cal.point1.valid;
    
    JsonObject p2 = tempCal["point2"].to<JsonObject>();
    p2["raw"] = cal.point2.rawValue;
    p2["actual"] = cal.point2.actualValue;
    p2["temp"] = cal.point2.temperature;
    p2["valid"] = cal.point2.valid;
  }
  
  // Save pH calibrations
  JsonArray phArray = doc["ph"].to<JsonArray>();
  for (int i = 0; i < 8; i++) {
    JsonObject phCal = phArray.add<JsonObject>();
    const PHCalibration& cal = calibrationData.ph[i];
    
    phCal["isCalibrated"] = cal.isCalibrated;
    phCal["slope"] = cal.slope;
    phCal["offset"] = cal.offset;
    phCal["tempCoeff"] = cal.tempCoeff;
    phCal["date"] = cal.calibrationDate;
    phCal["notes"] = cal.notes;
    
    JsonObject p1 = phCal["point1"].to<JsonObject>();
    p1["raw"] = cal.point1.rawValue;
    p1["actual"] = cal.point1.actualValue;
    p1["temp"] = cal.point1.temperature;
    p1["valid"] = cal.point1.valid;
    
    JsonObject p2 = phCal["point2"].to<JsonObject>();
    p2["raw"] = cal.point2.rawValue;
    p2["actual"] = cal.point2.actualValue;
    p2["temp"] = cal.point2.temperature;
    p2["valid"] = cal.point2.valid;
    
    JsonObject p3 = phCal["point3"].to<JsonObject>();
    p3["raw"] = cal.point3.rawValue;
    p3["actual"] = cal.point3.actualValue;
    p3["temp"] = cal.point3.temperature;
    p3["valid"] = cal.point3.valid;
  }
  
  // Save TDS calibrations
  JsonArray tdsArray = doc["tds"].to<JsonArray>();
  for (int i = 0; i < 8; i++) {
    JsonObject tdsCal = tdsArray.add<JsonObject>();
    const TDSCalibration& cal = calibrationData.tds[i];
    
    tdsCal["isCalibrated"] = cal.isCalibrated;
    tdsCal["kFactor"] = cal.kFactor;
    tdsCal["slope"] = cal.slope;
    tdsCal["offset"] = cal.offset;
    tdsCal["tempCoeff"] = cal.tempCoeff;
    tdsCal["date"] = cal.calibrationDate;
    tdsCal["notes"] = cal.notes;
    
    JsonObject p1 = tdsCal["point1"].to<JsonObject>();
    p1["raw"] = cal.point1.rawValue;
    p1["actual"] = cal.point1.actualValue;
    p1["temp"] = cal.point1.temperature;
    p1["valid"] = cal.point1.valid;
    
    JsonObject p2 = tdsCal["point2"].to<JsonObject>();
    p2["raw"] = cal.point2.rawValue;
    p2["actual"] = cal.point2.actualValue;
    p2["temp"] = cal.point2.temperature;
    p2["valid"] = cal.point2.valid;
  }
  
  // Save to file
  File file = SPIFFS.open("/calibration.json", "w");
  if (!file) {
    Serial.println("[CAL] Failed to open calibration file for writing");
    return false;
  }
  
  if (serializeJson(doc, file) == 0) {
    Serial.println("[CAL] Failed to write calibration data");
    file.close();
    return false;
  }
  
  file.close();
  Serial.println("[CAL] Calibration data saved successfully");
  return true;
}

float CalibrationManager::calculateSlope(const CalibrationPoint& p1, const CalibrationPoint& p2) {
  if (p1.rawValue == p2.rawValue) {
    return 1.0; // Avoid division by zero
  }
  return (p2.actualValue - p1.actualValue) / (p2.rawValue - p1.rawValue);
}

float CalibrationManager::calculateOffset(const CalibrationPoint& p1, float slope) {
  return p1.actualValue - (slope * p1.rawValue);
}

String CalibrationManager::getCurrentDateTime() {
  time_t now = time(0);
  struct tm* timeinfo = localtime(&now);
  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
  return String(buffer);
}

// Temperature sensor calibration methods
bool CalibrationManager::startTemperatureCalibration(int sensorIndex, const String& notes) {
  if (sensorIndex < 0 || sensorIndex >= 8) return false;
  
  TemperatureCalibration& cal = calibrationData.temperature[sensorIndex];
  memset(&cal, 0, sizeof(cal));
  cal.notes = notes;
  
  Serial.printf("[CAL] Started temperature calibration for sensor %d\n", sensorIndex + 1);
  return true;
}

bool CalibrationManager::addTemperatureCalibrationPoint(int sensorIndex, float rawValue, float actualTemp, float ambientTemp) {
  if (sensorIndex < 0 || sensorIndex >= 8) return false;
  
  TemperatureCalibration& cal = calibrationData.temperature[sensorIndex];
  
  if (!cal.point1.valid) {
    cal.point1.rawValue = rawValue;
    cal.point1.actualValue = actualTemp;
    cal.point1.temperature = ambientTemp;
    cal.point1.valid = true;
    Serial.printf("[CAL] Temp%d Point 1: Raw=%.3f, Actual=%.2fC\n", sensorIndex + 1, rawValue, actualTemp);
    return true;
  } else if (!cal.point2.valid) {
    cal.point2.rawValue = rawValue;
    cal.point2.actualValue = actualTemp;
    cal.point2.temperature = ambientTemp;
    cal.point2.valid = true;
    Serial.printf("[CAL] Temp%d Point 2: Raw=%.3f, Actual=%.2fC\n", sensorIndex + 1, rawValue, actualTemp);
    return true;
  }
  
  Serial.printf("[CAL] Temperature sensor %d already has 2 calibration points\n", sensorIndex + 1);
  return false;
}

bool CalibrationManager::finalizeTemperatureCalibration(int sensorIndex) {
  if (sensorIndex < 0 || sensorIndex >= 8) return false;
  
  TemperatureCalibration& cal = calibrationData.temperature[sensorIndex];
  
  if (!cal.point1.valid || !cal.point2.valid) {
    Serial.printf("[CAL] Temperature sensor %d needs 2 calibration points\n", sensorIndex + 1);
    return false;
  }
  
  cal.slope = calculateSlope(cal.point1, cal.point2);
  cal.offset = calculateOffset(cal.point1, cal.slope);
  cal.isCalibrated = true;
  cal.calibrationDate = getCurrentDateTime();
  
  Serial.printf("[CAL] Temperature sensor %d calibrated: slope=%.6f, offset=%.6f\n", 
                sensorIndex + 1, cal.slope, cal.offset);
  
  saveCalibrationData();
  return true;
}

float CalibrationManager::getCalibratedTemperature(int sensorIndex, float rawValue) {
  if (sensorIndex < 0 || sensorIndex >= 8) return rawValue;
  
  const TemperatureCalibration& cal = calibrationData.temperature[sensorIndex];
  
  if (!cal.isCalibrated) {
    // Return uncalibrated value
    return rawValue;
  }
  
  return (cal.slope * rawValue) + cal.offset;
}

bool CalibrationManager::isTemperatureCalibrated(int sensorIndex) {
  if (sensorIndex < 0 || sensorIndex >= 8) return false;
  return calibrationData.temperature[sensorIndex].isCalibrated;
}

// pH sensor calibration methods
bool CalibrationManager::startPHCalibration(int sensorIndex, const String& notes) {
  if (sensorIndex < 0 || sensorIndex >= 8) return false;
  
  PHCalibration& cal = calibrationData.ph[sensorIndex];
  memset(&cal, 0, sizeof(cal));
  cal.notes = notes;
  cal.tempCoeff = CalibrationStandards::TEMP_COEFF_PH;
  
  Serial.printf("[CAL] Started pH calibration for sensor %d\n", sensorIndex + 1);
  return true;
}

bool CalibrationManager::addPHCalibrationPoint(int sensorIndex, float rawValue, float actualPH, float temperature) {
  if (sensorIndex < 0 || sensorIndex >= 8) return false;
  
  PHCalibration& cal = calibrationData.ph[sensorIndex];
  
  if (!cal.point1.valid) {
    cal.point1.rawValue = rawValue;
    cal.point1.actualValue = actualPH;
    cal.point1.temperature = temperature;
    cal.point1.valid = true;
    Serial.printf("[CAL] pH%d Point 1: Raw=%.3f, Actual=%.2f pH\n", sensorIndex + 1, rawValue, actualPH);
    return true;
  } else if (!cal.point2.valid) {
    cal.point2.rawValue = rawValue;
    cal.point2.actualValue = actualPH;
    cal.point2.temperature = temperature;
    cal.point2.valid = true;
    Serial.printf("[CAL] pH%d Point 2: Raw=%.3f, Actual=%.2f pH\n", sensorIndex + 1, rawValue, actualPH);
    return true;
  } else if (!cal.point3.valid) {
    cal.point3.rawValue = rawValue;
    cal.point3.actualValue = actualPH;
    cal.point3.temperature = temperature;
    cal.point3.valid = true;
    Serial.printf("[CAL] pH%d Point 3: Raw=%.3f, Actual=%.2f pH\n", sensorIndex + 1, rawValue, actualPH);
    return true;
  }
  
  Serial.printf("[CAL] pH sensor %d already has 3 calibration points\n", sensorIndex + 1);
  return false;
}

bool CalibrationManager::finalizePHCalibration(int sensorIndex) {
  if (sensorIndex < 0 || sensorIndex >= 8) return false;
  
  PHCalibration& cal = calibrationData.ph[sensorIndex];
  
  if (!cal.point1.valid || !cal.point2.valid) {
    Serial.printf("[CAL] pH sensor %d needs at least 2 calibration points\n", sensorIndex + 1);
    return false;
  }
  
  // Use 2-point calibration (can be extended to 3-point later)
  cal.slope = calculateSlope(cal.point1, cal.point2);
  cal.offset = calculateOffset(cal.point1, cal.slope);
  cal.isCalibrated = true;
  cal.calibrationDate = getCurrentDateTime();
  
  Serial.printf("[CAL] pH sensor %d calibrated: slope=%.6f, offset=%.6f\n", 
                sensorIndex + 1, cal.slope, cal.offset);
  
  saveCalibrationData();
  return true;
}

float CalibrationManager::getCalibratedPH(int sensorIndex, float rawValue, float temperature) {
  if (sensorIndex < 0 || sensorIndex >= 8) return rawValue;
  
  const PHCalibration& cal = calibrationData.ph[sensorIndex];
  
  if (!cal.isCalibrated) {
    return rawValue;
  }
  
  float phValue = (cal.slope * rawValue) + cal.offset;
  
  // Apply temperature compensation
  float tempDelta = temperature - 25.0; // Reference temperature
  phValue = phValue - (cal.tempCoeff * tempDelta);
  
  return phValue;
}

bool CalibrationManager::isPHCalibrated(int sensorIndex) {
  if (sensorIndex < 0 || sensorIndex >= 8) return false;
  return calibrationData.ph[sensorIndex].isCalibrated;
}

// TDS sensor calibration methods
bool CalibrationManager::startTDSCalibration(int sensorIndex, const String& notes) {
  if (sensorIndex < 0 || sensorIndex >= 8) return false;
  
  TDSCalibration& cal = calibrationData.tds[sensorIndex];
  memset(&cal, 0, sizeof(cal));
  cal.notes = notes;
  cal.kFactor = 1.0;
  cal.tempCoeff = CalibrationStandards::TEMP_COEFF_EC;
  
  Serial.printf("[CAL] Started TDS calibration for sensor %d\n", sensorIndex + 1);
  return true;
}

bool CalibrationManager::addTDSCalibrationPoint(int sensorIndex, float rawValue, float actualEC, float temperature) {
  if (sensorIndex < 0 || sensorIndex >= 8) return false;
  
  TDSCalibration& cal = calibrationData.tds[sensorIndex];
  
  if (!cal.point1.valid) {
    cal.point1.rawValue = rawValue;
    cal.point1.actualValue = actualEC;
    cal.point1.temperature = temperature;
    cal.point1.valid = true;
    Serial.printf("[CAL] TDS%d Point 1: Raw=%.3f, Actual=%.0f uS/cm\n", sensorIndex + 1, rawValue, actualEC);
    return true;
  } else if (!cal.point2.valid) {
    cal.point2.rawValue = rawValue;
    cal.point2.actualValue = actualEC;
    cal.point2.temperature = temperature;
    cal.point2.valid = true;
    Serial.printf("[CAL] TDS%d Point 2: Raw=%.3f, Actual=%.0f uS/cm\n", sensorIndex + 1, rawValue, actualEC);
    return true;
  }
  
  Serial.printf("[CAL] TDS sensor %d already has 2 calibration points\n", sensorIndex + 1);
  return false;
}

bool CalibrationManager::finalizeTDSCalibration(int sensorIndex) {
  if (sensorIndex < 0 || sensorIndex >= 8) return false;
  
  TDSCalibration& cal = calibrationData.tds[sensorIndex];
  
  if (!cal.point1.valid || !cal.point2.valid) {
    Serial.printf("[CAL] TDS sensor %d needs 2 calibration points\n", sensorIndex + 1);
    return false;
  }
  
  cal.slope = calculateSlope(cal.point1, cal.point2);
  cal.offset = calculateOffset(cal.point1, cal.slope);
  cal.isCalibrated = true;
  cal.calibrationDate = getCurrentDateTime();
  
  Serial.printf("[CAL] TDS sensor %d calibrated: slope=%.6f, offset=%.6f, kFactor=%.3f\n", 
                sensorIndex + 1, cal.slope, cal.offset, cal.kFactor);
  
  saveCalibrationData();
  return true;
}

float CalibrationManager::getCalibratedTDS(int sensorIndex, float rawValue, float temperature) {
  if (sensorIndex < 0 || sensorIndex >= 8) return rawValue;
  
  const TDSCalibration& cal = calibrationData.tds[sensorIndex];
  
  if (!cal.isCalibrated) {
    return rawValue;
  }
  
  // Get calibrated EC first
  float ecValue = getCalibratedEC(sensorIndex, rawValue, temperature);
  
  // Convert EC to TDS (typically TDS = EC / 2)
  return ecValue / 2.0;
}

float CalibrationManager::getCalibratedEC(int sensorIndex, float rawValue, float temperature) {
  if (sensorIndex < 0 || sensorIndex >= 8) return rawValue;
  
  const TDSCalibration& cal = calibrationData.tds[sensorIndex];
  
  if (!cal.isCalibrated) {
    return rawValue;
  }
  
  float ecValue = (cal.slope * rawValue) + cal.offset;
  
  // Apply temperature compensation
  float tempDelta = temperature - 25.0; // Reference temperature
  ecValue = ecValue * (1.0 + (cal.tempCoeff * tempDelta));
  
  return ecValue;
}

bool CalibrationManager::isTDSCalibrated(int sensorIndex) {
  if (sensorIndex < 0 || sensorIndex >= 8) return false;
  return calibrationData.tds[sensorIndex].isCalibrated;
}

void CalibrationManager::printCalibrationStatus() {
  Serial.println("Calibration Status Summary:");
  Serial.println("===========================");
  
  // Temperature sensors
  Serial.println("Temperature Sensors:");
  for (int i = 0; i < 8; i++) {
    const TemperatureCalibration& cal = calibrationData.temperature[i];
    Serial.printf("  Temp%d: %s", i + 1, cal.isCalibrated ? "[CALIBRATED]" : "[NOT CALIBRATED]");
    if (cal.isCalibrated) {
      Serial.printf(" Date: %s", cal.calibrationDate.c_str());
    }
    Serial.println();
  }
  
  // pH sensors
  Serial.println("pH Sensors:");
  for (int i = 0; i < 8; i++) {
    const PHCalibration& cal = calibrationData.ph[i];
    Serial.printf("  pH%d: %s", i + 1, cal.isCalibrated ? "[CALIBRATED]" : "[NOT CALIBRATED]");
    if (cal.isCalibrated) {
      Serial.printf(" Date: %s", cal.calibrationDate.c_str());
    }
    Serial.println();
  }
  
  // TDS sensors
  Serial.println("TDS Sensors:");
  for (int i = 0; i < 8; i++) {
    const TDSCalibration& cal = calibrationData.tds[i];
    Serial.printf("  TDS%d: %s", i + 1, cal.isCalibrated ? "[CALIBRATED]" : "[NOT CALIBRATED]");
    if (cal.isCalibrated) {
      Serial.printf(" Date: %s", cal.calibrationDate.c_str());
    }
    Serial.println();
  }
  Serial.println("===========================");
}

String CalibrationManager::getCalibrationInstructions(const String& sensorType, int step) {
  if (sensorType == "temperature") {
    switch (step) {
      case 1:
        return "Place sensor in ice bath (0C) and wait for stable reading";
      case 2:
        return "Place sensor in room temperature water (measure with reference thermometer)";
      default:
        return "Temperature calibration complete";
    }
  } else if (sensorType == "ph") {
    switch (step) {
      case 1:
        return "Place sensor in pH 4.01 buffer solution and wait for stable reading";
      case 2:
        return "Rinse sensor and place in pH 6.86 buffer solution";
      case 3:
        return "Rinse sensor and place in pH 9.18 buffer solution";
      default:
        return "pH calibration complete";
    }
  } else if (sensorType == "tds") {
    switch (step) {
      case 1:
        return "Place sensor in 1413 uS/cm conductivity standard solution";
      case 2:
        return "Rinse sensor and place in 12880 uS/cm conductivity standard solution";
      default:
        return "TDS calibration complete";
    }
  }
  
  return "Unknown sensor type";
}

String CalibrationManager::getFullCalibrationStatus() {
  JsonDocument doc;
  
  // Temperature calibration status
  JsonArray tempArray = doc["temperature"].to<JsonArray>();
  for (int i = 0; i < 8; i++) {
    JsonObject tempStatus = tempArray.add<JsonObject>();
    const TemperatureCalibration& cal = calibrationData.temperature[i];
    tempStatus["isCalibrated"] = cal.isCalibrated;
    tempStatus["date"] = cal.calibrationDate;
    tempStatus["notes"] = cal.notes;
  }
  
  // pH calibration status
  JsonArray phArray = doc["ph"].to<JsonArray>();
  for (int i = 0; i < 8; i++) {
    JsonObject phStatus = phArray.add<JsonObject>();
    const PHCalibration& cal = calibrationData.ph[i];
    phStatus["isCalibrated"] = cal.isCalibrated;
    phStatus["date"] = cal.calibrationDate;
    phStatus["notes"] = cal.notes;
  }
  
  // TDS calibration status
  JsonArray tdsArray = doc["tds"].to<JsonArray>();
  for (int i = 0; i < 8; i++) {
    JsonObject tdsStatus = tdsArray.add<JsonObject>();
    const TDSCalibration& cal = calibrationData.tds[i];
    tdsStatus["isCalibrated"] = cal.isCalibrated;
    tdsStatus["date"] = cal.calibrationDate;
    tdsStatus["notes"] = cal.notes;
  }
  
  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}