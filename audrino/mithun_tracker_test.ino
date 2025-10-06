// Kapato Mithun Tracking Collar - TEST VERSION (No GPS Required)
// ESP32-C6 + EC200U CN 4G LTE Module
// Uses dummy GPS coordinates for indoor testing

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

// ============ TEST COORDINATES ============
// Dummy GPS coordinates for testing (Itanagar, Arunachal Pradesh)
float test_lat = 27.0844;
float test_lon = 93.6053;

// ============ STATE VARIABLES ============
float current_lat = 0.0;
float current_lon = 0.0;

int network_failure_count = 0;
int cycle_count = 0;

// ============ SETUP ============
void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=================================");
  Serial.println("Kapato Mithun Tracking Collar");
  Serial.println("*** TEST MODE - NO GPS REQUIRED ***");
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
  cycle_count++;

  Serial.println("\n========== NEW CYCLE #" + String(cycle_count) + " ==========");
  Serial.print("Time since boot: ");
  Serial.print(millis() / 1000);
  Serial.println(" seconds\n");

  // STEP 1: Use dummy GPS coordinates (add small variation for testing)
  current_lat = test_lat + (random(-100, 100) / 10000.0);  // Small random offset
  current_lon = test_lon + (random(-100, 100) / 10000.0);

  Serial.println("--- Using Test GPS Coordinates ---");
  Serial.print("✅ Latitude:  ");
  Serial.println(current_lat, 6);
  Serial.print("✅ Longitude: ");
  Serial.println(current_lon, 6);
  Serial.println("(Dummy coordinates for testing)");

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
  sendCommand("AT+QHTTPCFG=\"responseheader\",0", 1000);  // No request header needed

  Serial.println("✅ Module initialized (GPS skipped for test mode)\n");
}

// ============ FETCH DEVICE SETTINGS FROM SERVER ============
bool fetchDeviceSettings() {
  String url = "https://" + API_URL + GET_DEVICE_INFO_PATH + "?device_id=" + DEVICE_ID;

  Serial.print("GET ");
  Serial.println(url);

  // Close any previous HTTP session first
  Serial.println("Closing previous HTTP session...");
  Serial1.println("AT+QHTTPSTOP");
  delay(1000);
  while(Serial1.available()) Serial1.read();  // Clear buffer

  // Set URL
  Serial1.print("AT+QHTTPURL=" + String(url.length()) + ",80\r\n");

  if(!waitFor("CONNECT", 5000)) {
    Serial.println("❌ No CONNECT");
    return false;
  }

  delay(200);

  // Send URL
  for(int i = 0; i < url.length(); i++) {
    Serial1.write(url[i]);
    delay(10);
  }

  if(!waitFor("OK", 10000)) {
    Serial.println("❌ URL rejected");
    return false;
  }

  Serial.println("✓ URL set");
  delay(500);

  // Send GET - Wait for HTTP 200 response!
  Serial.println("Sending AT+QHTTPGET=60...");
  Serial1.print("AT+QHTTPGET=60\r\n");  // Using print with explicit \r\n

  // First wait for OK
  if(!waitFor("OK", 5000)) {
    Serial.println("❌ No OK after AT+QHTTPGET");
    printModuleResponse();
    return false;
  }
  Serial.println("✓ GET command accepted (OK)");

  // Now wait for +QHTTPGET: 0,200 URC
  Serial.println("Waiting for server response...");
  if(!waitFor("+QHTTPGET: 0,200", 65000)) {
    Serial.println("❌ GET failed (no 200 status or timeout)");
    printModuleResponse();
    return false;
  }

  Serial.println("✓ GET success (200 OK)");
  delay(1000);

  // Read response
  Serial.println("Reading response...");
  Serial1.print("AT+QHTTPREAD=60\r\n");

  String response = "";
  long readStart = millis();

  while(millis() - readStart < 10000) {
    while(Serial1.available()) {
      char c = Serial1.read();
      response += c;
    }
    if(response.indexOf("OK") > 0) break;
  }

  Serial.println("=== RESPONSE START ===");
  Serial.println(response);
  Serial.println("=== RESPONSE END ===");

  // Parse JSON response
  // Expected: {"success":true,"phone_number":"6009202874","update_interval":60,"notify_sms":true}

  if(response.indexOf("phone_number") >= 0) {
    // Parse phone_number
    int phoneStart = response.indexOf("\"phone_number\":\"") + 16;
    int phoneEnd = response.indexOf("\"", phoneStart);
    if(phoneStart > 16 && phoneEnd > phoneStart) {
      phone_number = response.substring(phoneStart, phoneEnd);
      Serial.print("✓ Phone: ");
      Serial.println(phone_number);
    }

    // Parse update_interval
    int intervalStart = response.indexOf("\"update_interval\":") + 18;
    int intervalEnd = response.indexOf(",", intervalStart);
    if(intervalEnd < 0) intervalEnd = response.indexOf("}", intervalStart);
    if(intervalStart > 18 && intervalEnd > intervalStart) {
      String intervalStr = response.substring(intervalStart, intervalEnd);
      update_interval = intervalStr.toInt();
      Serial.print("✓ Interval: ");
      Serial.print(update_interval);
      Serial.println("s");
    }

    // Parse notify_sms
    if(response.indexOf("\"notify_sms\":true") >= 0) {
      notify_sms = true;
      Serial.println("✓ SMS: Enabled");
    } else if(response.indexOf("\"notify_sms\":false") >= 0) {
      notify_sms = false;
      Serial.println("✓ SMS: Disabled");
    }

    return true;
  }

  Serial.println("⚠️ Failed to parse response");
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

  // Close any previous HTTP session
  Serial1.println("AT+QHTTPSTOP");
  delay(1000);
  while(Serial1.available()) Serial1.read();

  // Set Content-Type header
  Serial1.println("AT+QHTTPCFG=\"contenttype\",1");
  if(!waitFor("OK", 2000)) {
    Serial.println("Failed to set content type");
  }

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

  // POST request - Send ONLY the JSON body (not HTTP headers!)
  Serial1.print("AT+QHTTPPOST=" + String(jsonBody.length()) + ",60,60\r\n");
  if(!waitFor("CONNECT", 5000)) {
    Serial.println("POST connect failed");
    return false;
  }
  delay(200);

  // Send only the JSON body
  for(int i = 0; i < jsonBody.length(); i++) {
    Serial1.write(jsonBody[i]);
    delay(10);
  }

  // Wait for OK
  if(!waitFor("OK", 10000)) {
    Serial.println("POST send failed");
    return false;
  }
  Serial.println("✓ POST data sent");

  // Wait for +QHTTPPOST: 0,200
  Serial.println("Waiting for server response...");
  if(!waitFor("+QHTTPPOST: 0,200", 65000)) {
    Serial.println("POST response timeout or error");
    printModuleResponse();
    return false;
  }

  // Try to parse response for updated settings
  Serial1.println("AT+QHTTPREAD=80");
  delay(2000);

  String response = "";
  while(Serial1.available()) {
    response += Serial1.readString();
  }

  Serial.println("Response:");
  Serial.println(response);

  // Check if POST was successful (look for 200 status or success:true)
  bool success = (response.indexOf("200") >= 0 || response.indexOf("\"success\":true") >= 0);

  if(success) {
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
  }

  return success;
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

  // Check response
  String response = "";
  while(Serial1.available()) {
    response += Serial1.readString();
  }

  Serial.println("SMS Response:");
  Serial.println(response);

  if(response.indexOf("OK") >= 0 || response.indexOf("+CMGS:") >= 0) {
    Serial.println("✅ SMS sent successfully");
  } else {
    Serial.println("⚠️ SMS status unknown");
  }
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
      if(buffer.indexOf(keyword) >= 0) {
        return true;
      }
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

void printModuleResponse() {
  Serial.println("--- Module Response ---");
  delay(1000);
  if(Serial1.available()) {
    while(Serial1.available()) {
      Serial.write(Serial1.read());
    }
    Serial.println();
  } else {
    Serial.println("(No response from module)");
  }
  Serial.println("--- End Response ---");
}
