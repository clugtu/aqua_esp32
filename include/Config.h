#pragma once

// Default fallback values (used if JSON config fails to load)
// Actual values should be configured in data/config.json

// Default WiFi credentials (fallback only)
#define DEFAULT_WIFI_SSID "dilbert"
#define DEFAULT_WIFI_PASSWORD "windfall1969"

// Default hardware pin definitions (fallback only)  
#define DEFAULT_LED_PIN 2

// Default ADC1 pins (can be used anytime, even with WiFi)
#define DEFAULT_TEMP_ADC_PIN    32  // ADC1_CH4 - Temperature sensors multiplexer
#define DEFAULT_PH_ADC_PIN      33  // ADC1_CH5 - pH sensors multiplexer
#define DEFAULT_TDS_ADC_PIN     35  // ADC1_CH7 - TDS sensors multiplexer

// Default multiplexer control pins (CD74HC4067)
#define DEFAULT_MUX_S0          4   // Control pin 0
#define DEFAULT_MUX_S1          5   // Control pin 1  
#define DEFAULT_MUX_S2          18  // Control pin 2
#define DEFAULT_MUX_S3          19  // Control pin 3
#define DEFAULT_MUX_EN          21  // Enable pin (LOW = enabled)

// Default sensor configuration (fallback only)
#define DEFAULT_NUM_TEMP_SENSORS  8
#define DEFAULT_NUM_PH_SENSORS    8
#define DEFAULT_NUM_TDS_SENSORS   8

// Compatibility constants (for existing code)
#define NUM_TEMP_SENSORS  8
#define NUM_PH_SENSORS    8
#define NUM_TDS_SENSORS   8
#define TEMP_ADC_PIN      32
#define PH_ADC_PIN        33
#define TDS_ADC_PIN       35
#define MUX_S0            4
#define MUX_S1            5
#define MUX_S2            18
#define MUX_S3            19
#define MUX_EN            21
#define LED_PIN           2
#define SERIAL_BAUD_RATE  115200
#define SENSOR_READ_DELAY 5000
#define WEB_SERVER_PORT   80

// Default web server configuration
#define DEFAULT_WEB_SERVER_PORT   80

// Default system configuration  
#define DEFAULT_SERIAL_BAUD_RATE  115200
#define DEFAULT_SENSOR_READ_DELAY 5000  // 5 seconds between readings
#define DEFAULT_PRINT_INTERVAL 5000     // 5 seconds between serial prints

// ========================================
// NO ICONS POLICY - CRITICAL CONFIGURATION
// ========================================
// This system NEVER uses emoji or Unicode icons due to:
// - Terminal compatibility issues across different systems
// - Serial monitor display problems
// - Character encoding inconsistencies
// - Professional output requirements
#define USE_ICONS         false   // NEVER change this to true
#define USE_EMOJI         false   // NEVER change this to true
#define USE_UNICODE       false   // ASCII only for maximum compatibility
#define ASCII_ONLY        true    // All output must be pure ASCII

// Icon replacement policy
#define TEMP_SYMBOL       "C"     // Instead of °C symbol
#define DEGREE_SYMBOL     ""      // No degree symbol
#define WIFI_SYMBOL       "WiFi"  // Instead of WiFi icon
#define BULLET_SYMBOL     "-"     // Simple dash instead of bullet
#define CHECK_SYMBOL      "OK"    // Instead of checkmark
#define ERROR_SYMBOL      "ERR"   // Instead of X or error icons