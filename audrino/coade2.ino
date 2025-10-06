#define RXD1 12
#define TXD1 13

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, RXD1, TXD1);
  
  Serial.println();
  Serial.println("\n\nESP32 EC200U SMS Test");
  Serial.println("Waiting for module to initialize...");
  delay(20000);  // 20 second delay as per docs
 
  // Send the SMS
  sendSMS();
}

void loop() {
  // Monitor any responses from the module
  if(Serial1.available()) {
    String response = Serial1.readString();
    Serial.print("Module response: ");
    Serial.println(response);
  }
}
                                            
void sendSMS() {
  Serial.println("Setting SMS text mode...");
  Serial1.print("AT+CMGF=1\r");
  delay(1000);
  
  Serial.println("Sending SMS to +918131058338...");
  Serial1.println("AT+CMGS=\"+918131058338\"");
  delay(1000);
 
  Serial.println("Sending message content...");
  Serial1.println("Hello! EC200U SMS working. Check location: https://www.google.com/maps/place/C-+Sector,+Itanagar+791111/@27.0967548,93.6211865,16z/data=!3m1!4b1!4m6!3m5!1s0x3744070bea50ef9d:0xbf5bfd94613a2f27!8m2!3d27.0960797!4d93.6255798!16s%2Fg%2F12hn57cpd?entry=ttu&g_ep=EgoyMDI1MDkyNC4wIKXMDSoASAFQAw%3D%3D");
  delay(1000);
  
  // End AT command with Ctrl+Z (ASCII 26)
  Serial1.println((char)26);
  delay(500);
  Serial1.println();
  
  Serial.println("SMS command sent. Waiting for confirmation...");
  delay(5000);
  
  // Check for any responses
  if(Serial1.available()) {
    String response = Serial1.readString();
    Serial.print("Final response: ");
    Serial.println(response);
  }
}