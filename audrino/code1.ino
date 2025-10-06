#define RXD1 17
#define TXD1 16

// GPS tracking variables
String lastLatitude = "";
String lastLongitude = "";
bool hasValidGPS = false;

// Dummy location (change to your preferred fallback coordinates)
const float DUMMY_LAT = 12.9716;  // Bangalore
const float DUMMY_LON = 77.5946;

// Timer
unsigned long lastPostTime = 0;
const unsigned long POST_INTERVAL = 60000; // 1 minute

void setup() {
  // Initialize Serial1 first (EC200U communication)
  Serial1.begin(115200, SERIAL_8N1, RXD1, TXD1);
  
  // Longer delay to ensure EC200U is fully powered and ready
  delay(8000);  // Increased delay for module stabilization
  
  // Initialize module
  for(int i = 0; i < 3; i++) {
    Serial1.println("AT");
    delay(500);  // Increased from 300ms
  }
  while(Serial1.available()) Serial1.read();
  
  sendCommand("ATE0", 1000);
  sendCommand("AT+QIDEACT=1", 5000);
  sendCommand("AT+QICSGP=1,1,\"airtelgprs.com\",\"\",\"\",1", 2000);
  sendCommand("AT+QIACT=1", 10000);
  sendCommand("AT+QSSLCFG=\"sslversion\",1,4", 1000);
  sendCommand("AT+QSSLCFG=\"seclevel\",1,0", 1000);
  sendCommand("AT+QSSLCFG=\"ciphersuite\",1,0xFFFF", 1000);
  sendCommand("AT+QHTTPCFG=\"contextid\",1", 1000);
  sendCommand("AT+QHTTPCFG=\"sslctxid\",1", 1000);
  sendCommand("AT+QHTTPCFG=\"requestheader\",1", 1000);
  sendCommand("AT+QHTTPCFG=\"responseheader\",0", 1000);
  sendCommand("AT+QGPS=1", 2000);
  
  // Initialize USB Serial last (optional, for debugging only)
  Serial.begin(115200);
  Serial.println("GPS Tracker Running");
  Serial.println("Setup complete. Posting every 60s\n");
}

void loop() {
  if (millis() - lastPostTime >= POST_INTERVAL) {
    getAndPostGPSLocation();
    lastPostTime = millis();
  }
  delay(100);
}

void getAndPostGPSLocation() {
  // Only print if Serial is actually connected
  if(Serial) Serial.println("Getting GPS...");
  
  Serial1.println("AT+QGPSLOC=2");
  delay(2000);
  
  String gpsResponse = "";
  while (Serial1.available()) {
    gpsResponse += Serial1.readString();
  }
  
  String latToPost = "";
  String lonToPost = "";
  String locationType = "";
  
  // Parse GPS data
  if (gpsResponse.indexOf("+QGPSLOC:") >= 0) {
    int startPos = gpsResponse.indexOf("+QGPSLOC:") + 10;
    String data = gpsResponse.substring(startPos);
    
    int comma1 = data.indexOf(',');
    int comma2 = data.indexOf(',', comma1 + 1);
    int comma3 = data.indexOf(',', comma2 + 1);
    
    String currentLat = data.substring(comma1 + 1, comma2);
    String currentLon = data.substring(comma2 + 1, comma3);
    
    currentLat.trim();
    currentLon.trim();
    
    if (currentLat.length() > 0 && currentLon.length() > 0) {
      lastLatitude = currentLat;
      lastLongitude = currentLon;
      hasValidGPS = true;
      latToPost = currentLat;
      lonToPost = currentLon;
      locationType = "Current GPS";
    }
  }
  
  // Fallback to last known location
  if (latToPost.length() == 0 && hasValidGPS) {
    latToPost = lastLatitude;
    lonToPost = lastLongitude;
    locationType = "Last known";
  }
  
  // Fallback to dummy location
  if (latToPost.length() == 0) {
    latToPost = String(DUMMY_LAT, 6);
    lonToPost = String(DUMMY_LON, 6);
    locationType = "Dummy";
  }
  
  if(Serial) Serial.println("Posting: " + locationType);
  
  bool success = postGPSData(latToPost.toFloat(), lonToPost.toFloat());
  
  if(Serial) {
    Serial.println(success ? "OK" : "FAIL");
    Serial.println("Next: 60s\n");
  }
}

bool postGPSData(float lat, float lon) {
  String url = "https://kapato.netlify.app/.netlify/functions/save-gps";
  String jsonBody = "{\"latitude\":" + String(lat, 6) + 
                    ",\"longitude\":" + String(lon, 6) + "}";
  
  Serial1.print("AT+QHTTPURL=" + String(url.length()) + ",80\r\n");
  if(!waitFor("CONNECT", 5000)) return false;
  delay(200);
  
  for(int i = 0; i < url.length(); i++) {
    Serial1.write(url[i]);
    delay(10);
  }
  
  if(!waitFor("OK", 10000)) return false;
  delay(500);
  
  String httpRequest = "POST /.netlify/functions/save-gps HTTP/1.1\r\n";
  httpRequest += "Host: kapato.netlify.app\r\n";
  httpRequest += "Content-Type: application/json\r\n";
  httpRequest += "Content-Length: " + String(jsonBody.length()) + "\r\n";
  httpRequest += "Connection: keep-alive\r\n\r\n";
  httpRequest += jsonBody;
  
  Serial1.print("AT+QHTTPPOST=" + String(httpRequest.length()) + ",60,60\r\n");
  if(!waitFor("CONNECT", 5000)) return false;
  delay(200);
  
  for(int i = 0; i < httpRequest.length(); i++) {
    Serial1.write(httpRequest[i]);
    delay(10);
  }
  
  if(!waitFor("OK", 10000)) return false;
  if(!waitFor("+QHTTPPOST: 0,200", 65000)) return false;
  
  return true;
}

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
    delay(1);  // Small delay to prevent watchdog issues
  }
  return false;
}

void sendCommand(String cmd, int timeout) {
  Serial1.print(cmd + "\r\n");
  delay(timeout);
  while(Serial1.available()) Serial1.read();
}