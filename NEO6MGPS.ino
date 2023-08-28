#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>

// GPS pin configuration
const int GPS_TX_PIN = 10;
const int GPS_RX_PIN = 11;

SoftwareSerial mySerial(GPS_TX_PIN, GPS_RX_PIN);
TinyGPSPlus gps;

// Function declarations
void displayGPSInfo();
void printFormattedFloat(double number, int digits = 2);

// Time zone and update settings
const int timeZoneOffset = -8;
const char* timeZoneName = "UTC -08:00 Pacific";
const unsigned long UPDATE_INTERVAL_MS = 1000;
bool printRawData = false;  // Set to true if you want to print raw data, false otherwise.

void setup() {
    Serial.begin(9600);
    mySerial.begin(9600);
    delay(5000);
    Serial.println("Neo 6M with TinyGPS++");
    Serial.println("---------------------");
}

void loop() {
    bool newData = false;  
    unsigned long start = millis();

    // Process GPS data within update interval
    while (millis() - start < UPDATE_INTERVAL_MS) {
        while (mySerial.available()) {
            char c = mySerial.read();
            if(printRawData) {
                Serial.write(c);  // Print the raw data to the console.
            }
            if (gps.encode(c)) {
                if (gps.location.isValid() && !newData) {
                    displayGPSInfo();
                    newData = true;
                }
            }
        }
    }

    if (!newData) {
        Serial.println("Waiting for GPS signal...");
    }
}

// Helper function to determine the days in a month
byte daysInMonth(byte month, int year) {
    if(month == 2) {
        return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 29 : 28;
    }
    return (month == 4 || month == 6 || month == 9 || month == 11) ? 30 : 31;
}

void adjustTimeForTimezone(int *hour, byte *day, byte *month, int *year) {
    if (*hour < 0) {
        *hour += 24;
        (*day)--;
        if (*day == 0) {
            (*month)--;
            if (*month == 0) {
                (*month) = 12;
                (*year)--;
            }
            *day = daysInMonth(*month, *year);
        }
    } else if (*hour >= 24) {
        *hour -= 24;
        (*day)++;
        if (*day > daysInMonth(*month, *year)) {
            (*day) = 1;
            (*month)++;
            if (*month > 12) {
                (*month) = 1;
                (*year)++;
            }
        }
    }
}

// Display the processed GPS information
void displayGPSInfo() {
    Serial.println("GPS Data:");
    Serial.println("---------");
    
    if (gps.location.isValid()) {
        Serial.print("Latitude:  "); printFormattedFloat(gps.location.lat(), 5); Serial.println("°");
        Serial.print("Longitude: "); printFormattedFloat(gps.location.lng(), 5); Serial.println("°");
    } else {
        Serial.println("Location: Not available");
    }

    // Process and display date and time
    if (gps.date.isValid() && gps.time.isValid()) {
        int year = gps.date.year();
        byte month = gps.date.month();
        byte day = gps.date.day();
        byte hour = gps.time.hour();
        byte minute = gps.time.minute();
        byte second = gps.time.second();
        byte hundredths = 0;

        int localHour = hour + timeZoneOffset;
        adjustTimeForTimezone(&localHour, &day, &month, &year);

        char buffer[100];
        snprintf(buffer, sizeof(buffer), "Date: %02d/%02d/%d", month, day, year);
        Serial.println(buffer);
        snprintf(buffer, sizeof(buffer), "Time (%s): %02d:%02d:%02d.%02d", timeZoneName, localHour, minute, second, hundredths);
        Serial.println(buffer);
    } else {
        Serial.println("Date & Time: Not available");
    }

    if (gps.satellites.isValid()) {
        Serial.print("Satellites: "); Serial.println(gps.satellites.value());
    }
    if (gps.hdop.isValid()) {
        Serial.print("HDOP: "); Serial.println(gps.hdop.value());
    }
    if (gps.altitude.isValid()) {
        Serial.print("Altitude: "); printFormattedFloat(gps.altitude.meters(), 2); Serial.println(" meters");
    } else {
        Serial.println("Altitude: Not available");
    }
    if (gps.speed.isValid()) {
        Serial.print("Speed: "); printFormattedFloat(gps.speed.kmph(), 2); Serial.println(" km/h");
    } else {
        Serial.println("Speed: Not available");
    }

    Serial.println("---------");
    Serial.println();
}

// Print float numbers with specific digits
void printFormattedFloat(double number, int digits) {
    if (number < 0.0) {
        Serial.print('-');
        number = -number;
    }

    double rounding = 0.5;
    for (uint8_t i = 0; i < digits; ++i) {
        rounding /= 10.0;
    }

    number += rounding;
    unsigned long intPart = (unsigned long) number;
    double remainder = number - (double) intPart;
    Serial.print(intPart);

    if (digits > 0) {
        Serial.print(".");
    }

    while (digits-- > 0) {
        remainder *= 10.0;
        int toPrint = int(remainder);
        Serial.print(toPrint);
        remainder -= toPrint;
    }
}
