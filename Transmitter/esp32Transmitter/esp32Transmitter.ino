#include <WiFi.h>
#include <Preferences.h>

Preferences preferences;

void setup() {
  Serial.begin(115200);
  
  preferences.begin("wifi", false);
  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");
  
  if (ssid.length() > 0) {
    WiFi.begin(ssid.c_str(), password.c_str());
    if (WiFi.waitForConnectResult() == WL_CONNECTED) {
      Serial.println("Connected to WiFi using saved credentials.");
      return;
    }
  }
  
  WiFi.mode(WIFI_AP_STA);
  WiFi.beginSmartConfig();
  
  while (!WiFi.smartConfigDone()) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("SmartConfig received.");
  
  ssid = WiFi.SSID();
  password = WiFi.psk();
  
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
}

void loop() {
}