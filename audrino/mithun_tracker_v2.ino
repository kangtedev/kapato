// Kapato Mithun Tracking Collar - Optimized Firmware
// ESP32-C6 + EC200U CN 4G LTE Module
// Default: 2-minute updates, server-configurable

#define RXD1 17
#define TXD1 16

// ============ CONFIGURATION ============
// Device ID - CHANGE THIS FOR EACH COLLAR!
const String DEVICE_ID = "ESP32-001";

// Default settings (used if server unreachable)
String default_phone_number = "6009202874";
int default_update_interval = 120;     // 2 minutes for testing
bool default_notify_sms = false;

// Current settings (updated from server)
String phone_number = "6009202874";
int update_interval = 120;
bool notify_sms = false;

// API endpoint
const String API_URL = "kapato.netlify.app";
const String SAVE_GPS_PATH = "/.netlify/functions/save-gps";
const String GET_DEVICE_INFO_PATH = "/.netlify/functions/get-device-info";

// ============ STATE VARIABLES ============
float current_lat = 0.0;
float current_lon = 0.0;
float last_known_lat = 0.0;
float last_known_lon = 0.0;

int network_failure_count = 0;
int gps_failure_count = 0;

unsigned long lastWakeTime = 0;

// ============ SETUP ============
void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=================================");
  Serial.println("Kapato Mithun Tracking Collar");
  Serial.println("Device ID: " + DEVICE_ID);
  Serial.println("=================================\n");

  // Initialize EC200U
  Serial1.begin(115200, SERIAL_8N1, RXD1, TXD1);
  delay(8000);  // Wait for module to power up

  initializeModule();

  Serial.println("Setup complete. Starting main loop...\n");
}

// ============ MAIN LOOP ============
void loop() {
  Serial.println("\n========== NEW CYCLE ==========");
  Serial.print("Time since boot: ");
  Serial.print(millis() / 1000);
  Serial.println(" seconds\n");

  // STEP 1: Get GPS (3 attempts)
  bool gps_success = getGPSLocation(current_lat, current_lon);

  if (!gps_success) {
    Serial.println("❌ GPS failed after 3 attempts");
    Serial.println("Skipping network operations...");
    gps_failure_count++;

    Serial.print("Sleeping for ");
    Serial.print(update_interval);
    Serial.println(" seconds\n");

    delay(update_interval * 1000);
    return;  // Skip rest, try again next cycle
  }

  Serial.println("✅ GPS acquired successfully!");
  Serial.print("Location: ");
  Serial.print(current_lat, 6);
  Serial.print(", ");
  Serial.println(current_lon, 6);
  gps_failure_count = 0;  // Reset GPS failure counter

  // Store as last known good location
  last_known_lat = current_lat;
  last_known_lon = current_lon;

  // STEP 2: Get device settings from server (optional, can fail)
  Serial.println("\n--- Fetching Device Settings ---");
  bool settings_success = fetchDeviceSettings();

  if (settings_success) {
    Serial.println("✅ Settings fetched from server");
    Serial.print("  Phone: ");
    Serial.println(phone_number);
    Serial.print("  Interval: ");
    Serial.print(update_interval);
    Serial.println("s");
    Serial.print("  SMS Notify: ");
    Serial.println(notify_sms ? "YES" : "NO");
  } else {
    Serial.println("⚠️  Using fallback settings");
    Serial.print("  Phone: ");
    Serial.println(phone_number);
    Serial.print("  Interval: ");
    Serial.print(update_interval);
    Serial.println("s");
  }

  // STEP 3: POST GPS data to server
  Serial.println("\n--- Sending GPS Data ---");
  bool post_success = postGPSData(current_lat, current_lon);

  if (post_success) {
    Serial.println("✅ GPS data sent successfully");
    network_failure_count = 0;  // Reset failure counter
  } else {
    Serial.println("❌ Failed to send GPS data");
    network_failure_count++;
  }

  // STEP 4: SMS Logic
  Serial.println("\n--- SMS Notification ---");

  if (network_failure_count >= 2) {
    Serial.println("🚨 EMERGENCY MODE: Network failures detected!");
    Serial.println("Sending emergency SMS...");
    sendEmergencySMS(current_lat, current_lon);
  } else if (notify_sms && post_success) {
    Serial.println("✅ Sending normal location SMS...");
    sendLocationSMS(current_lat, current_lon);
  } else {
    Serial.println("⏭️  SMS skipped (notify_sms=false or network issue)");
  }

  // STEP 5: Sleep
  Serial.println("\n--- Sleep Mode ---");
  Serial.print("Next update in ");
  Serial.print(update_interval);
  Serial.println(" seconds");
  Serial.println("========== CYCLE END ==========\n");

  delay(update_interval * 1000);
}

// ============ INITIALIZE EC200U MODULE ============
void initializeModule() {
  Serial.println("Initializing EC200U module...");

  // Wake up module
  for(int i = 0; i < 3; i++) {
    Serial1.println("AT");
    delay(500);
  }
  while(Serial1.available()) Serial1.read();

  sendCommand("ATE0", 1000);  // Echo off
  sendCommand("AT+QIDEACT=1", 5000);  // Deactivate context
  sendCommand("AT+QICSGP=1,1,\"airtelgprs.com\",\"\",\"\",1", 2000);  // Set APN
  sendCommand("AT+QIACT=1", 10000);  // Activate network

  // SSL/HTTPS config
  sendCommand("AT+QSSLCFG=\"sslversion\",1,4", 1000);
  sendCommand("AT+QSSLCFG=\"seclevel\",1,0", 1000);
  sendCommand("AT+QSSLCFG=\"ciphersuite\",1,0xFFFF", 1000);

  // HTTP config
  sendCommand("AT+QHTTPCFG=\"contextid\",1", 1000);
  sendCommand("AT+QHTTPCFG=\"sslctxid\",1", 1000);
  sendCommand("AT+QHTTPCFG=\"requestheader\",1", 1000);
  sendCommand("AT+QHTTPCFG=\"responseheader\",0", 1000);

  // Start GPS
  sendCommand("AT+QGPS=1", 2000);

  Serial.println("✅ Module initialized\n");
}

// ============ GET GPS LOCATION (3 ATTEMPTS) ============
bool getGPSLocation(float &lat, float &lon) {
  const int MAX_ATTEMPTS = 3;
  const int TIMEOUT_MS = 30000;  // 30 seconds per attempt

  Serial.println("--- GPS Acquisition ---");

  for(int attempt = 1; attempt <= MAX_ATTEMPTS; attempt++) {
    Serial.print("GPS Attempt ");
    Serial.print(attempt);
    Serial.print("/");
    Serial.println(MAX_ATTEMPTS);

    Serial1.println("AT+QGPSLOC=2");
    delay(TIMEOUT_MS);

    String response = "";
    while(Serial1.available()) {
      response += Serial1.readString();
    }

    // Parse GPS response: +QGPSLOC: <time>,<lat>,<lon>,<hdop>,<alt>,<fix>,<cog>,<spkm>,<spkn>,<date>,<nsat>
    if(response.indexOf("+QGPSLOC:") >= 0) {
      int startPos = response.indexOf("+QGPSLOC:") + 10;
      String data = response.substring(startPos);

      // Parse: skip time, get lat, get lon
      int comma1 = data.indexOf(',');
      int comma2 = data.indexOf(',', comma1 + 1);
      int comma3 = data.indexOf(',', comma2 + 1);

      String latStr = data.substring(comma1 + 1, comma2);
      String lonStr = data.substring(comma2 + 1, comma3);

      latStr.trim();
      lonStr.trim();

      if(latStr.length() > 0 && lonStr.length() > 0) {
        lat = latStr.toFloat();
        lon = lonStr.toFloat();

        Serial.println("✅ GPS Fix Acquired!");
        Serial.print("  Latitude: ");
        Serial.println(lat, 6);
        Serial.print("  Longitude: ");
        Serial.println(lon, 6);

        return true;
      }
    }

    Serial.print("❌ Attempt ");
    Serial.print(attempt);
    Serial.println(" failed");

    if(attempt < MAX_ATTEMPTS) {
      Serial.println("Waiting 5s before retry...");
      delay(5000);
    }
  }

  Serial.println("❌ All GPS attempts exhausted");
  return false;
}

// ============ FETCH DEVICE SETTINGS FROM SERVER ============
bool fetchDeviceSettings() {
  String url = "https://" + API_URL + GET_DEVICE_INFO_PATH + "?device_id=" + DEVICE_ID;

  Serial.print("GET ");
  Serial.println(url);

  // Set URL
  Serial1.print("AT+QHTTPURL=" + String(url.length()) + ",80\r\n");
  if(!waitFor("CONNECT", 5000)) {
    Serial.println("Failed to set URL");
    return false;
  }
  delay(200);

  for(int i = 0; i < url.length(); i++) {
    Serial1.write(url[i]);
    delay(10);
  }

  if(!waitFor("OK", 10000)) {
    Serial.println("URL setup failed");
    return false;
  }
  delay(500);

  // GET request
  Serial1.println("AT+QHTTPGET=60");
  if(!waitFor("+QHTTPGET:", 65000)) {
    Serial.println("GET request failed");
    return false;
  }

  // Read response
  Serial1.println("AT+QHTTPREAD=80");
  delay(2000);

  String response = "";
  while(Serial1.available()) {
    response += Serial1.readString();
  }

  // Parse JSON response (simple parsing)
  // Expected: {"phone_number":"6009202874","update_interval":120,"notify_sms":true}

  if(response.indexOf("phone_number") >= 0) {
    // Parse phone_number
    int phoneStart = response.indexOf("\"phone_number\":\"") + 16;
    int phoneEnd = response.indexOf("\"", phoneStart);
    if(phoneStart > 16 && phoneEnd > phoneStart) {
      phone_number = response.substring(phoneStart, phoneEnd);
    }

    // Parse update_interval
    int intervalStart = response.indexOf("\"update_interval\":") + 18;
    int intervalEnd = response.indexOf(",", intervalStart);
    if(intervalEnd < 0) intervalEnd = response.indexOf("}", intervalStart);
    if(intervalStart > 18 && intervalEnd > intervalStart) {
      String intervalStr = response.substring(intervalStart, intervalEnd);
      update_interval = intervalStr.toInt();
    }

    // Parse notify_sms
    if(response.indexOf("\"notify_sms\":true") >= 0) {
      notify_sms = true;
    } else if(response.indexOf("\"notify_sms\":false") >= 0) {
      notify_sms = false;
    }

    return true;
  }

  // If parsing failed, keep current/default settings
  return false;
}

// ============ POST GPS DATA TO SERVER ============
bool postGPSData(float lat, float lon) {
  String url = "https://" + API_URL + SAVE_GPS_PATH;
  String jsonBody = "{\"device_id\":\"" + DEVICE_ID + "\"," +
                    "\"latitude\":" + String(lat, 6) + "," +
                    "\"longitude\":" + String(lon, 6) + "}";

  Serial.print("POST ");
  Serial.println(url);
  Serial.print("Body: ");
  Serial.println(jsonBody);

  // Set URL
  Serial1.print("AT+QHTTPURL=" + String(url.length()) + ",80\r\n");
  if(!waitFor("CONNECT", 5000)) return false;
  delay(200);

  for(int i = 0; i < url.length(); i++) {
    Serial1.write(url[i]);
    delay(10);
  }

  if(!waitFor("OK", 10000)) return false;
  delay(500);

  // Build HTTP POST request
  String httpRequest = "POST " + SAVE_GPS_PATH + " HTTP/1.1\r\n";
  httpRequest += "Host: " + API_URL + "\r\n";
  httpRequest += "Content-Type: application/json\r\n";
  httpRequest += "Content-Length: " + String(jsonBody.length()) + "\r\n";
  httpRequest += "Connection: keep-alive\r\n\r\n";
  httpRequest += jsonBody;

  // POST request
  Serial1.print("AT+QHTTPPOST=" + String(httpRequest.length()) + ",60,60\r\n");
  if(!waitFor("CONNECT", 5000)) return false;
  delay(200);

  for(int i = 0; i < httpRequest.length(); i++) {
    Serial1.write(httpRequest[i]);
    delay(10);
  }

  if(!waitFor("OK", 10000)) return false;
  if(!waitFor("+QHTTPPOST: 0,200", 65000)) return false;

  // Try to parse response for updated settings
  Serial1.println("AT+QHTTPREAD=80");
  delay(2000);

  String response = "";
  while(Serial1.available()) {
    response += Serial1.readString();
  }

  // Parse update_interval from response if available
  if(response.indexOf("\"update_interval\":") >= 0) {
    int start = response.indexOf("\"update_interval\":") + 18;
    int end = response.indexOf(",", start);
    if(end < 0) end = response.indexOf("}", start);
    if(start > 18 && end > start) {
      String intervalStr = response.substring(start, end);
      int newInterval = intervalStr.toInt();
      if(newInterval > 0) {
        update_interval = newInterval;
        Serial.print("📡 Server updated interval to: ");
        Serial.print(update_interval);
        Serial.println("s");
      }
    }
  }

  return true;
}

// ============ SEND LOCATION SMS ============
void sendLocationSMS(float lat, float lon) {
  String mapsLink = "https://maps.google.com/?q=" + String(lat, 6) + "," + String(lon, 6);
  String message = "Mithun location: " + mapsLink;

  sendSMS(phone_number, message);
}

// ============ SEND EMERGENCY SMS ============
void sendEmergencySMS(float lat, float lon) {
  String mapsLink = "https://maps.google.com/?q=" + String(lat, 6) + "," + String(lon, 6);
  String message = "ALERT: Collar OFFLINE. Last location: " + mapsLink;

  sendSMS(phone_number, message);
}

// ============ SEND SMS (Generic) ============
void sendSMS(String number, String message) {
  Serial.print("Sending SMS to ");
  Serial.println(number);
  Serial.print("Message: ");
  Serial.println(message);

  // Set SMS text mode
  Serial1.println("AT+CMGF=1");
  delay(1000);

  // Set recipient
  Serial1.print("AT+CMGS=\"");
  Serial1.print(number);
  Serial1.println("\"");
  delay(1000);

  // Send message
  Serial1.println(message);
  delay(1000);

  // End with Ctrl+Z
  Serial1.write(26);
  delay(5000);

  Serial.println("✅ SMS sent");
}

// ============ UTILITY FUNCTIONS ============
bool waitFor(String keyword, int timeout) {
  String buffer = "";
  long start = millis();

  while(millis() - start < timeout) {
    while(Serial1.available()) {
      char c = Serial1.read();
      buffer += c;
      if(buffer.length() > 500) buffer = buffer.substring(250);
      if(buffer.indexOf(keyword) >= 0) return true;
    }
    delay(1);
  }
  return false;
}

void sendCommand(String cmd, int timeout) {
  Serial1.print(cmd + "\r\n");
  delay(timeout);
  while(Serial1.available()) Serial1.read();
}
