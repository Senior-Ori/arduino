#include <WiFi.h>
#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

void setup() {
  SerialBT.begin("ESP32test");
}

void loop() {
  if (SerialBT.available()) {
    String newSSID = "";
    String newPassword = "";
    char incomingChar = SerialBT.read();
    if(incomingChar == '#') {
      while(SerialBT.available()) {
        incomingChar = SerialBT.read();
        if(incomingChar == ',') break;
        newSSID += incomingChar;
      }
      while(SerialBT.available()) {
        incomingChar = SerialBT.read();
        if(incomingChar == '#') break;
        newPassword += incomingChar;
      }
      
      WiFi.disconnect();
      
      WiFi.begin(newSSID.c_str(), newPassword.c_str());
      while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
      }
      
    }
    
  }
}