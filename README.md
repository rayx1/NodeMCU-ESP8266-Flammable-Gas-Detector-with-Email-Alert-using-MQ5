# NodeMCU-ESP8266-Flammable-Gas-Detector-with-Email-Alert-using-MQ5
A WiFi-connected gas leakage alarm system using an ESP8266, MQ-5 gas sensor, buzzer, LED, and email notifications via SMTP.

## 🚀 Features

- Detects flammable gases (e.g., LPG, natural gas) using MQ-5
- Activates buzzer and LED on high gas concentration
- Sends email alert with ppm reading
- Cooldown period to prevent email spamming
- Continues to function offline (alarm still works)
- Automatically reconnects to WiFi if disconnected

---

## 🛠️ Hardware Required

- ESP8266 (e.g., NodeMCU)
- MQ-5 Gas Sensor (analog)
- Active buzzer
- LED with resistor (220Ω recommended)
- Jumper wires
- Breadboard or soldered circuit

---

## 📷 Wiring Diagram (Text-Based)

      MQ-5 Sensor              ESP8266 (NodeMCU)
    +-------------+          +------------------+
    |    VCC      | -------- |     3V3          |
    |    GND      | -------- |     GND          |
    |    AOUT     | -------- |     A0           |
    +-------------+          +------------------+

     Buzzer                     ESP8266
    +-------+                +------------------+
    | +     | --------------|     D5 (GPIO14)   |
    | -     | --------------|     GND           |
    +-------+                +------------------+

     LED + Resistor           ESP8266
    +-----/\/\/-----+       +------------------+
    |               |-------|     D6 (GPIO12)   |
    |               |       |     GND           |
    +---------------+       +------------------+


---

## 🔧 Configuration

In `main.ino`, set your WiFi and email credentials:

```cpp
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

#define SMTP_HOST "smtp.yourprovider.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "your_email@domain.com"
#define AUTHOR_PASSWORD "your_email_password"
#define RECIPIENT_EMAIL "recipient@domain.com"

## 📦 Libraries Used 

ESP8266WiFi

ESP-Mail-Client

Install them via the Arduino Library Manager or clone the GitHub repos.

## ⚠️ Notes
Email only works when connected to WiFi.

Alarm (buzzer and LED) will work even without internet.

Make sure to calibrate the MQ-5 sensor in clean air during startup.

Sensitive data (passwords) should not be committed to public repositories.

## 📜 License
This project is licensed under the MIT License.
