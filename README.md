# Neo 6M GPS with TinyGPS++

This project provides an Arduino code for interfacing with the uBlox Neo 6M GPS module using the TinyGPS++ library. The application decodes the GPS data to provide location, altitude, speed, and other essential data.

## Files

- NEO6MGPS.ino = Base example with the NEO-6M GPS
- NEO6MGPS-SSD1306.ino = Displays GPS data to an OLED display in addition to console

## Features

- Retrieves and displays:
  - **Latitude and Longitude** with precision customization
  - **Local Date and Time** based on configurable time zone offset
  - **Number of satellites** in view
  - **Horizontal Dilution of Precision (HDOP)**
  - **Altitude** in meters
  - **Speed** in kilometers per hour

- Adjusts local time based on the specified time zone offset
- Checks for leap years and adjusts date accordingly
- Uses `SoftwareSerial` to communicate with the GPS module, leaving the primary serial free for debugging and data output
- Provides diagnostic messages if GPS data is not available

## Setup

1. Connect the Neo 6M GPS module to your Arduino board:
   - `TX` of GPS to pin `11` (GPS_RX_PIN) of Arduino
   - `RX` of GPS to pin `10` (GPS_TX_PIN) of Arduino
   - Connect `GND` of GPS to `GND` of Arduino
   - Connect `VCC` of GPS to appropriate power (usually `3.3V` or `5V` depending on your module) of Arduino

2. Upload the Arduino code to your board.

3. Open the Arduino Serial Monitor at `9600` baud to view the GPS data.

## Configurations

- **timeZoneOffset**: Adjust this value to match your local time zone. Default is set to `-8` for Pacific Time (UTC-8).
- **timeZoneName**: Name or description of the local time zone. This is used for display purposes in the Serial Monitor.
- **UPDATE_INTERVAL_MS**: Controls how often the GPS data is read and displayed. Default is `1000` milliseconds (1 second).

## Additional Notes

If you don't see any GPS data right away, make sure the GPS module has a clear view of the sky. The first-time satellite lock might take a few minutes.

The provided `printFormattedFloat` function is a utility to display floating-point numbers with a specific number of decimal places in the Serial Monitor.

There is an addional example for displaying data to an SSD1306 OLED display.
