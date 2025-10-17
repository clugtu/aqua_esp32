#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

// Calibration point structure
struct CalibrationPoint {
  float rawValue;      // ADC reading or voltage
  float actualValue;   // Known reference value
  float temperature;   // Temperature during calibration (for compensation)
  bool valid;          // Whether this calibration point is valid
};

// Temperature sensor calibration data
struct TemperatureCalibration {
  CalibrationPoint point1;  // Ice bath (0°C) or known low temp
  CalibrationPoint point2;  // Boiling water (100°C) or known high temp
  float slope;              // Calculated slope
  float offset;             // Calculated offset
  bool isCalibrated;        // Whether calibration is complete
  String calibrationDate;   // When calibration was performed
  String notes;             // Calibration notes
};

// pH sensor calibration data  
struct PHCalibration {
  CalibrationPoint point1;  // pH 4.01 buffer
  CalibrationPoint point2;  // pH 6.86 buffer (optional for 2-point)
  CalibrationPoint point3;  // pH 9.18 buffer
  float slope;              // Calculated slope
  float offset;             // Calculated offset
  float tempCoeff;          // Temperature compensation coefficient
  bool isCalibrated;        // Whether calibration is complete
  String calibrationDate;   // When calibration was performed
  String notes;             // Calibration notes
};

// TDS sensor calibration data
struct TDSCalibration {
  CalibrationPoint point1;  // Low standard (84 or 1413 µS/cm)
  CalibrationPoint point2;  // High standard (12,880 µS/cm)
  float kFactor;            // K-factor for TDS calculation
  float slope;              // Calculated slope for EC
  float offset;             // Calculated offset for EC
  float tempCoeff;          // Temperature compensation coefficient (2%/°C typical)
  bool isCalibrated;        // Whether calibration is complete
  String calibrationDate;   // When calibration was performed
  String notes;             // Calibration notes
};

// Complete sensor calibration data
struct SensorCalibrationData {
  TemperatureCalibration temperature[8];  // One per temperature sensor
  PHCalibration ph[8];                    // One per pH sensor
  TDSCalibration tds[8];                  // One per TDS sensor
};

class CalibrationManager {
private:
  SensorCalibrationData calibrationData;
  bool dataLoaded;
  
  // Helper methods
  float calculateSlope(const CalibrationPoint& p1, const CalibrationPoint& p2);
  float calculateOffset(const CalibrationPoint& p1, float slope);
  String getCurrentDateTime();
  
public:
  CalibrationManager();
  
  // Initialize and load/save calibration data
  bool begin();
  bool loadCalibrationData();
  bool saveCalibrationData();
  
  // Temperature sensor calibration
  bool startTemperatureCalibration(int sensorIndex, const String& notes = "");
  bool addTemperatureCalibrationPoint(int sensorIndex, float rawValue, float actualTemp, float ambientTemp = 25.0);
  bool finalizeTemperatureCalibration(int sensorIndex);
  float getCalibratedTemperature(int sensorIndex, float rawValue);
  bool isTemperatureCalibrated(int sensorIndex);
  
  // pH sensor calibration
  bool startPHCalibration(int sensorIndex, const String& notes = "");
  bool addPHCalibrationPoint(int sensorIndex, float rawValue, float actualPH, float temperature = 25.0);
  bool finalizePHCalibration(int sensorIndex);
  float getCalibratedPH(int sensorIndex, float rawValue, float temperature = 25.0);
  bool isPHCalibrated(int sensorIndex);
  
  // TDS sensor calibration
  bool startTDSCalibration(int sensorIndex, const String& notes = "");
  bool addTDSCalibrationPoint(int sensorIndex, float rawValue, float actualEC, float temperature = 25.0);
  bool finalizeTDSCalibration(int sensorIndex);
  float getCalibratedTDS(int sensorIndex, float rawValue, float temperature = 25.0);
  float getCalibratedEC(int sensorIndex, float rawValue, float temperature = 25.0);
  bool isTDSCalibrated(int sensorIndex);
  
  // Calibration status and information
  void printCalibrationStatus();
  void printSensorCalibration(const String& sensorType, int sensorIndex);
  void clearCalibration(const String& sensorType, int sensorIndex);
  void clearAllCalibrations();
  
  // Get calibration data for web interface
  String getCalibrationJSON(const String& sensorType, int sensorIndex);
  String getFullCalibrationStatus();
  
  // Validation methods
  bool validateCalibrationPoint(float rawValue, float actualValue);
  String getCalibrationInstructions(const String& sensorType, int step);
};

// Standard calibration values
namespace CalibrationStandards {
  // Temperature standards
  constexpr float ICE_BATH_TEMP = 0.0;      // °C
  constexpr float BOILING_WATER_TEMP = 100.0; // °C at sea level
  constexpr float ROOM_TEMP = 23.0;         // °C typical
  constexpr float BODY_TEMP = 37.0;         // °C
  
  // pH buffer standards
  constexpr float PH_BUFFER_4 = 4.01;       // pH
  constexpr float PH_BUFFER_7 = 6.86;       // pH (or 7.00)
  constexpr float PH_BUFFER_9 = 9.18;       // pH
  
  // Conductivity/TDS standards (µS/cm)
  constexpr float EC_STANDARD_LOW = 84.0;    // µS/cm
  constexpr float EC_STANDARD_MID = 1413.0;  // µS/cm
  constexpr float EC_STANDARD_HIGH = 12880.0; // µS/cm
  
  // Default temperature compensation coefficients
  constexpr float TEMP_COEFF_PH = 0.02;      // pH units per °C
  constexpr float TEMP_COEFF_EC = 0.02;      // 2% per °C
}