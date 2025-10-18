# ESP32 Aqua Project

This is a PlatformIO project for ESP32 development with Arduino framework.

## Project Structure

```
aqua_esp32/
├── src/              # Source files
│   └── main.cpp      # Main application code
├── include/          # Header files
├── lib/              # Project-specific libraries
├── test/             # Unit tests
└── platformio.ini    # PlatformIO configuration
```

## Hardware Requirements

### Main Components
- ESP32-DevKitC-32 development board
- USB cable (micro-USB) for programming and power
- 3x CD74HC4067 16-channel analog multiplexers
- 8x Temperature sensors (analog output)
- 8x pH sensors (analog output)  
- 8x TDS sensors (analog output)

#### ESP32 ADC Pin Analysis:

**Available ADC Pins on ESP32-DevKitC-32:**
- **ADC1**: GPIO 32, 33, 34, 35, 36, 39 (6 pins) - **WiFi Compatible**
- **ADC2**: GPIO 0, 2, 4, 12, 13, 14, 15, 25, 26, 27 (10 pins) - **WiFi Conflicts**
- **Total**: 16 ADC pins available

**WiFi Compatibility Issue:**
- ADC2 pins **cannot be used** when WiFi is active (hardware limitation)
- Only **6 ADC1 pins** are safely usable with WiFi enabled
- For 16 sensors (8 temp + 8 pH), we need multiplexers or alternative approach

#### Multiplexer Implementation:
Our system uses 3x CD74HC4067 16-channel analog multiplexers to efficiently manage 24 sensors using only 3 ADC pins:

- **GPIO 32** (ADC1_CH4): Temperature sensors multiplexer output  
- **GPIO 33** (ADC1_CH5): pH sensors multiplexer output
- **GPIO 35** (ADC1_CH7): TDS sensors multiplexer output
- **GPIO 4, 5, 18, 19**: Multiplexer control pins (S0-S3) - shared
- **GPIO 21**: Multiplexer enable pin (active LOW) - shared

**Advantages of Multiplexer Approach:**
- **WiFi Compatible**: Uses only ADC1 pins (ADC2 conflicts with WiFi)
- **Scalable**: 24 sensors using just 3 ADC pins
- **Cost Effective**: ~$6 for 48 analog channels total
- **Fast Switching**: ~1μs multiplexer switching time
- **Simple Wiring**: Shared control pins minimize ESP32 pin usage
- **Future Expansion**: Easy to add more sensor types

#### Current Sensor Configuration:
- **CD74HC4067 #1**: Temperature sensors 1-8 (channels 0-7) → GPIO 32
- **CD74HC4067 #2**: pH sensors 1-8 (channels 0-7) → GPIO 33  
- **CD74HC4067 #3**: TDS sensors 1-8 (channels 0-7) → GPIO 35
- **Control pins**: All multiplexers share S0-S3 control lines
- **Enable pin**: All multiplexers share enable control

## Getting Started

1. **Clone and Setup**
   ```bash
   git clone https://github.com/clugtu/aqua_esp32.git
   cd aqua_esp32
   ```

2. **Configure WiFi Credentials**
   ```bash
   # Copy the configuration template
   cp data/config.example.json data/config.json
   
   # Edit with your WiFi credentials
   nano data/config.json  # or use your preferred editor
   ```
   
   Update the WiFi section:
   ```json
   {
     "wifi": {
       "ssid": "YOUR_WIFI_SSID", 
       "password": "YOUR_WIFI_PASSWORD"
     }
   }
   ```

3. **Build and Upload**
   - Open this project in VS Code with PlatformIO IDE extension installed
   - Connect your ESP32 board to your computer via USB
   - Build the project: Click the checkmark (✓) in the PlatformIO toolbar
   - Upload the firmware: Click the arrow (→) in the PlatformIO toolbar
   - Open Serial Monitor: Click the plug icon in the PlatformIO toolbar

**Note**: The build system automatically creates `config.json` from the template if it doesn't exist.

## Configuration

### Initial Setup
1. Copy the configuration template:
   ```bash
   cp data/config.example.json data/config.json
   ```
2. Edit `data/config.json` with your specific settings:
   ```json
   {
     "wifi": {
       "ssid": "YOUR_WIFI_SSID",
       "password": "YOUR_WIFI_PASSWORD"
     }
   }
   ```

**Security Note**: The `data/config.json` file is ignored by git to protect your WiFi credentials. Always use the example file as a template.

### Upload Port
If you need to specify a specific COM port, uncomment and modify these lines in `platformio.ini`:
```ini
upload_port = COM3
monitor_port = COM3
```

### Serial Monitor
The project is configured to use 115200 baud rate for serial communication.

## Web Interface & API

### Web Dashboard
After connecting to WiFi, access the dashboard at:
```
http://[ESP32_IP_ADDRESS]/
```
The IP address will be displayed in the Serial Monitor.

### REST API Endpoints

#### Get All Sensor Data
```http
GET /api/sensors
```
**Response:**
```json
{
  "temperature": [25.4, 26.1, 24.8, 25.9, 26.3, 25.1, 24.7, 25.6],
  "ph": [7.2, 7.1, 7.3, 7.0, 7.2, 7.4, 7.1, 7.3],
  "tds": [
    {"ppm": 150, "ec": 300}, {"ppm": 145, "ec": 290}, 
    {"ppm": 155, "ec": 310}, {"ppm": 148, "ec": 296}
  ],
  "lastUpdate": 12345678,
  "timestamp": 12345678
}
```

#### Get Temperature Sensors Only
```http
GET /api/temperature
```
**Response:**
```json
{
  "sensors": [
    {"id": 1, "value": 25.4, "unit": "degC"},
    {"id": 2, "value": 26.1, "unit": "degC"}
  ],
  "count": 8,
  "timestamp": 12345678
}
```

#### Get pH Sensors Only
```http
GET /api/ph
```
**Response:**
```json
{
  "sensors": [
    {"id": 1, "value": 7.2, "unit": "pH"},
    {"id": 2, "value": 7.1, "unit": "pH"}
  ],
  "count": 8,
  "timestamp": 12345678
}
```

#### Get TDS Sensors Only
```http
GET /api/tds
```
**Response:**
```json
{
  "sensors": [
    {"id": 1, "ppm": 150, "ec": 300, "unit": "ppm"},
    {"id": 2, "ppm": 145, "ec": 290, "unit": "ppm"}
  ],
  "count": 8,
  "timestamp": 12345678
}
```

#### System Status
```http
GET /api/status
```
**Response:**
```json
{
  "system": "ESP32 Aqua Monitor",
  "version": "1.0.0",
  "uptime": 12345678,
  "freeHeap": 200000,
  "wifi": {
    "ssid": "MyWiFiNetwork",
    "rssi": -45,
    "ip": "192.168.1.100"
  },
  "sensors": {
    "temperature": 8,
    "ph": 8,
    "tds": 8
  }
}
```

### Home Automation Integration

#### Home Assistant Example
```yaml
# configuration.yaml
sensor:
  - platform: rest
    resource: "http://192.168.1.100/api/temperature"
    name: "Aqua Temperature Sensors"
    json_attributes:
      - sensors
    value_template: "{{ value_json.count }}"
```

#### Node-RED Example
Use HTTP Request node with URL: `http://192.168.1.100/api/sensors`

#### Python Script Example
```python
import requests
import json

response = requests.get('http://192.168.1.100/api/sensors')
data = response.json()

for i, temp in enumerate(data['temperature']):
    print(f"Tank {i+1} Temperature: {temp} degC")
    
for i, ph in enumerate(data['ph']):
    print(f"Tank {i+1} pH: {ph}")
    
for i, tds in enumerate(data['tds']):
    print(f"Tank {i+1} TDS: {tds['ppm']} ppm ({tds['ec']} μS/cm)")
```

## Architecture & Code Organization

### Template-Based Web Interface
The ESP32 Aqua Monitor uses a modern template-based architecture for maintainable web interfaces:

**Template Manager System:**
- **Templates**: HTML files stored in `data/templates/` directory
- **Template Engine**: `TemplateManager.cpp` handles loading and variable substitution  
- **Template Caching**: Templates cached in memory for performance
- **Variable Substitution**: `{{VARIABLE}}` format for dynamic content

**Template Files:**
- `dashboard.html` - Main aquarium monitoring interface
- `admin_login.html` - Administrative authentication page  
- `config.html` - System configuration interface
- `calibration.html` - Sensor calibration interface (planned)
- `diagnostics.html` - System diagnostics page (planned)
- `help.html` - User guide and shopping recommendations (planned)

**Advantages:**
- **Maintainable**: Separate HTML/CSS from C++ code
- **Faster Development**: Edit templates without recompiling firmware
- **Better Organization**: Clear separation of presentation and logic
- **Template Reuse**: Common layouts and components
- **Memory Efficient**: Templates cached and loaded from SPIFFS

**File Organization:**
```
data/
├── templates/           # HTML template files
│   ├── dashboard.html
│   ├── admin_login.html
│   ├── config.html
│   └── ...
├── config.json         # Runtime configuration
└── config.example.json # Configuration template

src/
├── TemplateManager.cpp # Template loading and processing
├── AquaWebServer.cpp   # Web server logic (reduced size)
└── ...

include/
├── TemplateManager.h   # Template manager interface
├── AquaWebServer.h     # Web server interface
└── ...
```

## Current Features

- **Multi-sensor monitoring**: 8 temperature + 8 pH + 8 TDS sensors (24 total)
- **Web dashboard**: Real-time browser-based aquarium grid interface
- **REST API**: JSON endpoints for home automation integration
- **WiFi connectivity**: Remote monitoring and control
- **Analog multiplexing**: 3x CD74HC4067 for 24 sensor channels
- **Cost effective**: ~$6 total for 48 analog channels
- **Fast sampling**: Direct ESP32 ADC reads with 5-second updates
- **12-bit resolution**: ESP32's built-in ADC (0-4095 range)
- **Cross-platform access**: Works on phones, tablets, computers
- **Home automation ready**: CORS-enabled API for external systems
- **Professional calibration**: Multi-point calibration with stability detection
- **System diagnostics**: CPU, memory, and WiFi monitoring
- **Secure configuration**: WiFi credentials protected in gitignored config file

## Sensor Calibration

### Calibration Process
Each sensor requires 1-3 reference points for accurate calibration. The system uses statistical stability detection to automatically capture calibration points when readings stabilize.

**Calibration Steps:**
1. **Enter Reference Value** - Input the known value of your reference standard
2. **Place Sensor** - Immerse sensor in reference solution  
3. **Wait for Stability** - System detects when reading stabilizes automatically
4. **Add More Points** (Optional) - Up to 3 total points for enhanced accuracy

### pH Sensor Calibration

**Reference Solutions Needed:**
- **Basic Calibration**: 1-2 buffer solutions (e.g., pH 7.00, pH 4.01 or pH 10.01)
- **Enhanced Calibration**: 3 buffer solutions (e.g., pH 4.01, pH 7.00, pH 10.01)

**Common pH Buffer Values:**
- **Acidic**: pH 4.01, pH 1.68
- **Neutral**: pH 7.00, pH 6.86  
- **Basic**: pH 10.01, pH 9.18, pH 12.45

**Recommended pH Buffer Solutions:**
- [pH Buffer Solution Set (pH 4.01, 7.00, 10.01)](https://www.amazon.com/s?k=ph+buffer+solution+4.01+7.00+10.01+calibration)
- [pH 7.00 Buffer Solution](https://www.amazon.com/s?k=ph+7.00+buffer+solution+calibration)
- [pH 4.01 Buffer Solution](https://www.amazon.com/s?k=ph+4.01+buffer+solution+calibration)
- [pH 10.01 Buffer Solution](https://www.amazon.com/s?k=ph+10.01+buffer+solution+calibration)

### TDS Sensor Calibration

**Reference Solutions Needed:**
- **Basic Calibration**: 1 conductivity standard (e.g., 1413 μS/cm)
- **Enhanced Calibration**: 2-3 standards (e.g., 84 μS/cm, 1413 μS/cm, 12880 μS/cm)

**Common TDS Standard Values:**
- **Low**: 84 μS/cm, 447 μS/cm
- **Medium**: 1413 μS/cm (1.413 mS/cm)
- **High**: 5000 μS/cm, 12880 μS/cm (12.88 mS/cm)

**Recommended TDS Calibration Solutions:**
- [TDS Calibration Solution Set](https://www.amazon.com/s?k=tds+conductivity+calibration+solution+1413+12880)
- [1413 μS/cm Conductivity Standard](https://www.amazon.com/s?k=1413+conductivity+standard+solution)
- [12880 μS/cm Conductivity Standard](https://www.amazon.com/s?k=12880+conductivity+standard+solution)
- [TDS Meter Calibration Kit](https://www.amazon.com/s?k=tds+meter+calibration+kit+solution)

### Temperature Sensor Calibration

**Reference Standards:**
- **Ice Bath**: 0.0 degC (32.0°F) - Distilled water + ice
- **Room Temperature**: Use calibrated reference thermometer
- **Warm Water**: 37.0 degC (98.6°F) or other known temperature

**Recommended Reference Thermometers:**
- [Digital Instant-Read Thermometer (±0.1 degC accuracy)](https://www.amazon.com/s?k=digital+instant+read+thermometer+0.1+degree+accuracy)
- [Calibrated Laboratory Thermometer](https://www.amazon.com/s?k=laboratory+thermometer+calibrated+certificate)
- [Thermocouple Temperature Probe](https://www.amazon.com/s?k=thermocouple+temperature+probe+digital+accurate)
- [NIST Traceable Thermometer](https://www.amazon.com/s?k=NIST+traceable+digital+thermometer)

### Calibration Tips

**Best Practices:**
- **Clean sensors** between different reference solutions
- **Allow time** for thermal equilibrium (especially temperature)
- **Gentle stirring** can help with solution homogeneity
- **Room temperature** solutions work best (unless testing temperature extremes)
- **Fresh solutions** provide best accuracy

**Storage:**
- Store pH buffer solutions in cool, dark place
- Check expiration dates on reference solutions
- TDS standards can be reused multiple times
- Replace solutions if they become contaminated

- **Quality Levels:**
- **Basic**: Hobbyist/home use - ±0.1 pH, ±2% TDS, ±0.5 degC
- **Good**: Aquarium/hydroponics - ±0.05 pH, ±1% TDS, ±0.2 degC  
- **Laboratory**: Research/commercial - ±0.01 pH, ±0.5% TDS, ±0.1 degC

## Hardware Components & Sensors

### ESP32 Development Board
- [ESP32-DevKitC-32 Development Board](https://www.amazon.com/s?k=ESP32-DevKitC-32+development+board)
- [ESP32 DevKit V1 WROOM-32 Board](https://www.amazon.com/s?k=ESP32+DevKit+V1+WROOM-32+development+board)

### Multiplexers
- [CD74HC4067 16-Channel Analog Multiplexer (3-pack)](https://www.amazon.com/s?k=CD74HC4067+16+channel+analog+multiplexer+breakout)
- [74HC4067 Multiplexer Module](https://www.amazon.com/s?k=74HC4067+multiplexer+module+16+channel)

### Temperature Sensors
- [DS18B20 Waterproof Temperature Sensors (8-pack)](https://www.amazon.com/s?k=DS18B20+waterproof+temperature+sensor+8+pack)
- [Analog Temperature Sensor LM35 (8-pack)](https://www.amazon.com/s?k=LM35+analog+temperature+sensor+8+pack)
- [NTC Thermistor Temperature Sensors](https://www.amazon.com/s?k=NTC+thermistor+temperature+sensor+waterproof+8+pack)

### pH Sensors
- [Analog pH Sensor Kit (8-pack)](https://www.amazon.com/s?k=analog+pH+sensor+kit+aquarium+8+pack)
- [pH Electrode Probe BNC Connector](https://www.amazon.com/s?k=pH+electrode+probe+BNC+connector+8+pack)
- [pH Sensor Module for Arduino](https://www.amazon.com/s?k=pH+sensor+module+arduino+analog+8+pack)

### TDS Sensors
- [TDS EC Conductivity Sensor (8-pack)](https://www.amazon.com/s?k=TDS+EC+conductivity+sensor+analog+8+pack)
- [Total Dissolved Solids Sensor Module](https://www.amazon.com/s?k=total+dissolved+solids+sensor+module+analog)
- [EC TDS Meter Sensor Probe](https://www.amazon.com/s?k=EC+TDS+meter+sensor+probe+analog+arduino)
- [Conductivity Sensor Electrode](https://www.amazon.com/s?k=conductivity+sensor+electrode+TDS+analog+8+pack)

### Wiring & Connectors
- [DuPont Jumper Wires (Male-Female)](https://www.amazon.com/s?k=dupont+jumper+wires+male+female+40+pin)
- [Breadboard & Jumper Wire Kit](https://www.amazon.com/s?k=breadboard+jumper+wire+kit+electronics)
- [Screw Terminal Blocks](https://www.amazon.com/s?k=screw+terminal+blocks+2+pin+electronics)

### Power Supply
- [5V 3A Power Adapter](https://www.amazon.com/s?k=5V+3A+power+adapter+barrel+connector)
- [USB Power Supply 5V 2A](https://www.amazon.com/s?k=USB+power+supply+5V+2A+wall+adapter)

## Development

To add new features:
1. Modify `src/main.cpp` for your application logic
2. Add header files to the `include/` directory
3. Add external libraries via PlatformIO Library Manager or `lib_deps` in `platformio.ini`

## Common Commands

- **Build**: `Ctrl+Alt+B`
- **Upload**: `Ctrl+Alt+U`
- **Serial Monitor**: `Ctrl+Alt+S`
- **Clean**: `Ctrl+Alt+C`