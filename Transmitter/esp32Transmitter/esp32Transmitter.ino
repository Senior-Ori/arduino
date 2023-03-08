#include <WiFi.h>
#include <Preferences.h>
#include <HTTPClient.h>

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
      Serial.print("ssid: ");
      Serial.print(ssid.c_str());
      Serial.print("\npassword: ");
      Serial.println(password.c_str());
      getTime();
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

   getTime();
}

void loop() {
}

void getTime() {
   HTTPClient http;
   http.begin("http://worldtimeapi.org/api/ip");
   int httpCode = http.GET();
   int retries = 0;

   while (httpCode <= 0 && retries < 3) {
     Serial.println("Error getting time. Retrying...");
     delay(1000);
     httpCode = http.GET();
     retries++;
   }

   if (httpCode >0) {
     String payload = http.getString();
     Serial.println(payload);
   } else {
     Serial.println("Error getting time after 3 attempts.");
   }

   http.end();
}