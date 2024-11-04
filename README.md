# Smart Hub Device

## Overview

The Smart Hub Device is an IoT solution built on the ESP32/ESP8266 microcontroller. It connects to Wi-Fi networks using SmartConfig and communicates with MQTT brokers for real-time data handling. The device supports automatic reconnection and LED indicators to provide visual feedback on Wi-Fi status.

## Features

- **Wi-Fi Connectivity**: Connects to Wi-Fi using SmartConfig or stored credentials.
- **MQTT Communication**: Publishes and subscribes to MQTT topics for command and control.
- **LED Indicators**: Provides visual feedback on Wi-Fi connection status.
- **Network Information Display**: Outputs Wi-Fi status, MAC address, and IP configuration to the serial console.

## Hardware Requirements

- ESP32 or ESP8266 microcontroller
- LED (for status indication)
- Breadboard and jumper wires
- Power supply (5V)

## Software Requirements

- Arduino IDE (or compatible IDE)
- Required libraries:
  - `WiFi`
  - `PubSubClient`
  - `Preferences`

## Installation

1. **Wiring the Hardware**:
   - Connect the LED to the specified GPIO pin (default is pin 2).
   - Ensure power connections are secure.

2. **Setting Up the Software**:
   - Install the Arduino IDE.
   - Add the ESP32/ESP8266 board manager.
   - Install required libraries via the Library Manager.

3. **Uploading the Code**:
   - Clone this repository or download the source code.
   - Open the project in Arduino IDE and upload the code to your ESP module.

## Usage

After uploading the code, the device will attempt to connect to a Wi-Fi network using stored credentials. If no credentials are found, it will enter SmartConfig mode, allowing configuration via a mobile app. The device will print network information to the serial console, including SSID, RSSI (signal strength), and IP address.

### Serial Output

The device provides real-time feedback on the following:
- SSID
- RSSI
- IP address
- Gateway IP
- DNS IP
- MAC address

## MQTT Integration

The device is configured to connect to an MQTT broker (`test.mosquitto.org`). It subscribes to the topic `esp32/command` and can receive commands to reset Wi-Fi credentials.

## Functions Overview

- `wifiInit()`: Initializes Wi-Fi and loads stored credentials.
- `initSmartConfig()`: Starts SmartConfig for Wi-Fi setup.
- `callback()`: Handles incoming MQTT messages.
- `reconnect()`: Reconnects to the MQTT broker if the connection is lost.
- `getWifiStatus()`: Retrieves the current Wi-Fi connection status.
- `getRSSI()`: Measures the RSSI of the connected Wi-Fi network.

## Contributing

Contributions are welcome! Feel free to submit pull requests or open issues for suggestions and improvements.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Thanks to the open-source community for the libraries and resources that made this project possible.
- Inspired by various IoT projects and tutorials available online.
