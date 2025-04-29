#include <ESP8266WiFi.h>
#include <ESP_Mail_Client.h>

// WiFi Configuration
#define WIFI_SSID "WIFI"
#define WIFI_PASSWORD "12345678"

// Email Configuration
#define SMTP_HOST "smtp.yourprovider.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "your_email@domain.com"
#define AUTHOR_PASSWORD "your_email_password"
#define RECIPIENT_EMAIL "recipient@domain.com"

// Hardware Configuration
#define MQ5_PIN A0        // MQ5 analog input pin
#define BUZZER_PIN D5     // Buzzer pin (active-low)
#define LED_PIN D6        // Visual alarm LED

// Threshold Values
#define GAS_THRESHOLD 200  // Adjust based on your MQ5 calibration
#define BUZZER_INTERVAL 500 // 0.5 second beep interval when active
#define EMAIL_COOLDOWN 300000 // 5 minutes cooldown between emails

SMTPSession smtp;
unsigned long lastEmailTime = 0;
bool emailOnCooldown = false;
unsigned long lastBuzzerTime = 0;
bool buzzerState = HIGH; // Start with buzzer OFF (active-low)
bool alarmActive = false;

// Calibration values
float RO_CLEAN_AIR = 10.0; // Will be calibrated in setup()

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH); // Initialize buzzer OFF
  digitalWrite(LED_PIN, LOW);
  
  connectToWiFi();
  
  // Setup email (won't actually connect until needed)
  smtp.debug(1);
  ESP_Mail_Session session;
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";
  
  smtp.connect(&session);

  // Calibrate MQ5 sensor (ensure clean air during startup)
  Serial.println("\nCalibrating MQ5 sensor (ensure clean air)...");
  RO_CLEAN_AIR = calibrateMQ5();
  Serial.print("Calibration complete. RO (clean air) = ");
  Serial.println(RO_CLEAN_AIR);
  
  Serial.println("\nSystem initialized - MQ5 Flammable Gas Detector");
  Serial.println("----------------------------------------------");
}

void loop() {
  float gasConcentration = readGasConcentration();
  
  Serial.print("Gas Concentration: ");
  Serial.print(gasConcentration);
  Serial.println(" ppm");
  
  // Handle email cooldown
  if (emailOnCooldown && (millis() - lastEmailTime >= EMAIL_COOLDOWN)) {
    emailOnCooldown = false;
    Serial.println("Email cooldown period ended");
  }
  
  if (gasConcentration > GAS_THRESHOLD) {
    // Activate alarm (non-blocking)
    unsigned long currentMillis = millis();
    if (currentMillis - lastBuzzerTime >= BUZZER_INTERVAL) {
      lastBuzzerTime = currentMillis;
      buzzerState = !buzzerState; // Toggle state
      digitalWrite(BUZZER_PIN, buzzerState);
      digitalWrite(LED_PIN, buzzerState == LOW ? HIGH : LOW); // LED sync with buzzer
    }
    alarmActive = true;
    
    // Try to send email if not on cooldown and WiFi is available
    if (!emailOnCooldown && WiFi.status() == WL_CONNECTED) {
      if (sendEmailAlert(gasConcentration)) {
        lastEmailTime = millis();
        emailOnCooldown = true;
        Serial.println("Email sent, cooldown period started");
      }
    }
  } else {
    // Gas level is safe - turn off alarms
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, LOW);
    alarmActive = false;
    
    // Reset email cooldown only when gas returns to safe level
    emailOnCooldown = false;
  }
  
  // Handle WiFi reconnection if needed
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }
  
  delay(1000); // Delay between readings
}

float calibrateMQ5() {
  float sensorValue = 0;
  // Take multiple samples for calibration
  for (int i = 0; i < 50; i++) {
    sensorValue += analogRead(MQ5_PIN);
    delay(100);
  }
  sensorValue = sensorValue / 50.0;
  
  // Calculate sensor resistance in clean air
  float sensorVoltage = sensorValue * (5.0 / 1023.0);
  float RS_AIR = ((5.0 * 1.0) / sensorVoltage) - 1.0; // Assuming RL = 1kΩ
  return RS_AIR;
}

float readGasConcentration() {
  int rawValue = analogRead(MQ5_PIN);
  
  // Convert to voltage
  float sensorVoltage = rawValue * (5.0 / 1023.0);
  
  // Calculate sensor resistance
  float RS_GAS = ((5.0 * 1.0) / sensorVoltage) - 1.0; // Assuming RL = 1kΩ
  
  // Calculate ratio (RS/RO)
  float ratio = RS_GAS / RO_CLEAN_AIR;
  
  // Convert to ppm (approximation for LPG - adjust based on your gas)
  float ppm = 1000 * pow(ratio, -2.95);
  
  return ppm;
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi. Continuing without network.");
  }
}

bool sendEmailAlert(float gasConcentration) {
  Serial.println("Preparing to send email alert...");
  
  SMTP_Message message;
  message.sender.name = "Gas Detector";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "FLAMMABLE GAS ALERT!";
  message.addRecipient("Recipient", RECIPIENT_EMAIL);
  
  String textMsg = "WARNING! Flammable gas detected.\n\n";
  textMsg += "Gas Concentration: " + String(gasConcentration) + " ppm\n";
  textMsg += "Threshold: " + String(GAS_THRESHOLD) + " ppm\n\n";
  textMsg += "Time: " + String(millis() / 1000) + " seconds since startup\n";
  textMsg += "Take appropriate safety measures immediately!";
  
  message.text.content = textMsg.c_str();
  
  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.println("Error sending Email, " + smtp.errorReason());
    return false;
  }
  
  Serial.println("Email sent successfully");
  return true;
}