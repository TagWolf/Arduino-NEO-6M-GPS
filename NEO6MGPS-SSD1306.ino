#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// GPS pin configuration
const int GPS_TX_PIN = 10;
const int GPS_RX_PIN = 11;

SoftwareSerial mySerial(GPS_TX_PIN, GPS_RX_PIN);
TinyGPSPlus gps;

// OLED display configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Screen management
enum ScreenType {
    SCREEN_LOCATION_ALTITUDE_SPEED,
    SCREEN_DATETIME_SATELLITES,
    SCREEN_COUNT // Always keep this last
};
ScreenType currentScreen = SCREEN_LOCATION_ALTITUDE_SPEED;
unsigned long lastScreenUpdate = 0;
const unsigned long SCREEN_DURATION_MS = 5000; // Display each screen for 5 seconds

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

    // Initialize OLED display
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    display.clearDisplay();
    display.display();
}

void loop() {
    bool newData = false;  
    unsigned long start = millis();

    // Process GPS data within update interval
    while (millis() - start < UPDATE_INTERVAL_MS && mySerial.available()) {
        char c = mySerial.read();
        if(printRawData) {
            Serial.write(c);  // Print the raw data to the console.
        }
        if (gps.encode(c) && gps.location.isValid()) {
            newData = true;
        }
    }

    if (millis() - lastScreenUpdate > SCREEN_DURATION_MS) {
        displayGPSInfo();
        switch (currentScreen) {
            case SCREEN_LOCATION_ALTITUDE_SPEED:
                displayLocationAltitudeSpeed();
                break;
            case SCREEN_DATETIME_SATELLITES:
                displayDateTimeSatellites();
                break;
            default:
                break;
        }

        // Cycle to the next screen
        currentScreen = static_cast<ScreenType>((currentScreen + 1) % SCREEN_COUNT);
        lastScreenUpdate = millis();
    }
}

// Helper function to initialize display settings
void initDisplay() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
}

// Helper function to retrieve date and time
void retrieveDateTime(int &year, byte &month, byte &day, byte &hour, byte &minute, byte &second) {
    year = gps.date.year();
    month = gps.date.month();
    day = gps.date.day();
    hour = gps.time.hour();
    minute = gps.time.minute();
    second = gps.time.second();
}

void printFormattedFloat(double number, int digits, bool toOLED = false) {
    if (number < 0.0) {
        if (toOLED) display.print('-');
        else Serial.print('-');
        number = -number;
    }

    double rounding = 0.5;
    for (uint8_t i = 0; i < digits; ++i) {
        rounding /= 10.0;
    }

    number += rounding;
    unsigned long intPart = (unsigned long) number;
    double remainder = number - (double) intPart;
    if (toOLED) display.print(intPart);
    else Serial.print(intPart);

    if (digits > 0) {
        if (toOLED) display.print(".");
        else Serial.print(".");
    }

    while (digits-- > 0) {
        remainder *= 10.0;
        int toPrint = int(remainder);
        if (toOLED) display.print(toPrint);
        else Serial.print(toPrint);
        remainder -= toPrint;
    }
}

void displayLocationAltitudeSpeed() {
    initDisplay();
    
    if (gps.location.isValid()) {
        display.print("Lat: "); 
        printFormattedFloat(gps.location.lat(), 5, true); 
        display.print((char)247); // Degree symbol
        display.setCursor(0, 10);
        display.print("Lng: "); 
        printFormattedFloat(gps.location.lng(), 5, true);
        display.print((char)247);
        display.setCursor(0, 20);
        display.print("Alt:"); 
        printFormattedFloat(gps.altitude.meters(), 1, true); // Reduced precision to 1 decimal place
        display.print("m ");
        display.print("Spd:"); 
        printFormattedFloat(gps.speed.kmph(), 1, true); // Reduced precision to 1 decimal place
        display.print("kmh");
    } else {
        display.println("GPS Data: Acquiring");
    }
    display.display();
}


void displayDateTimeSatellites() {
    initDisplay();
    
    if (gps.date.isValid() && gps.time.isValid()) {
        int year, localHour;
        byte month, day, hour, minute, second;
        retrieveDateTime(year, month, day, hour, minute, second);

        localHour = hour + timeZoneOffset;
        adjustTimeForTimezone(&localHour, &day, &month, &year);

        char buffer[30];
        snprintf(buffer, sizeof(buffer), "Date: %02d/%02d/%d", month, day, year);
        display.print(buffer);
        display.setCursor(0, 10);
        snprintf(buffer, sizeof(buffer), "Time: %02d:%02d:%02d", localHour, minute, second);
        display.print(buffer);
        display.setCursor(0, 20);

        if (gps.satellites.isValid()) {
            display.print("Sats: "); display.print(gps.satellites.value());
        } else {
            display.println("Sats: Acquiring");
        }
    } else {
        display.println("Date & Time: Acquiring");
    }
    display.display();
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
        Serial.println("Location: Acquiring...");
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
        Serial.println("Date & Time: Acquiring...");
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
        Serial.println("Altitude: Acquiring...");
    }
    if (gps.speed.isValid()) {
        Serial.print("Speed: "); printFormattedFloat(gps.speed.kmph(), 2); Serial.println(" km/h");
    } else {
        Serial.println("Speed: Acquiring...");
    }

    Serial.println("---------");
    Serial.println();
}
