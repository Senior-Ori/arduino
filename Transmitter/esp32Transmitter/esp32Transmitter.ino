#include <WiFi.h>
#include <Preferences.h>
#include <HTTPClient.h>

Preferences preferences;

TaskHandle_t getTimeTaskHandle;
TaskHandle_t postTimeTaskHandle;
TaskHandle_t checkSensorsTaskHandle;

/** DEFINE GIOS**/
#define IR_SENSOR_1 32
#define IR_SENSOR_2 33
#define IR_SENSOR_3 34
#define IR_SENSOR_4 35
#define HT12E_D0 18
#define HT12E_D1 19
#define HT12E_D2 21
#define HT12E_D3 22
#define HT12E_TE 23

/** DEFINES **/
#define MAX_FAILURES 10

/** GLOBALS **/
int ir_sensor_data[4] = {0, 0, 0, 0};
int previous_ir_sensor_data[4] = {-1, -1, -1, -1};
bool changed = true;

// Replace these with your sensor pins
// const int sensorPins[] = {32, 33, 34, 35};
// const int numSensors = sizeof(sensorPins) / sizeof(sensorPins[0]);

// int prevSensorValues[numSensors];



void setup() {
  Serial.begin(115200);
  
  preferences.begin("wifi", false);
  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");
  
  for (int i=0; i<numSensors; i++) {
    prevSensorValues[i] = -1;
   }
  
   if (ssid.length() >0) {
     WiFi.begin(ssid.c_str(), password.c_str());
     if (WiFi.waitForConnectResult() == WL_CONNECTED) {
       Serial.println("Connected to WiFi using saved credentials.");
       Serial.print("ssid: ");
       Serial.print(ssid.c_str());
       Serial.print("\npassword: ");
       Serial.println(password.c_str());
       
      //  xTaskCreate(
      //    getTimeTask,
      //    "GetTime",
      //    10000,
      //    NULL,
      //    1,
      //    &getTimeTaskHandle
      //  );
       
      //  xTaskCreate(
      //    postTimeTask,
      //    "PostTime",
      //    10000,
      //    NULL,
      //   1 ,
      //   &postTimeTaskHandle
      // );
      
      xTaskCreate(
        checkSensorsTask,
        "CheckSensors",
        10000,
        NULL,
        2, // Higher priority than other tasks
        &checkSensorsTaskHandle
      );
      
      return ;
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

  //  xTaskCreate(
  //    getTimeTask,
  //    "GetTime",
  //    10000,
  //    NULL,
  //    1,
  //    &getTimeTaskHandle
  //  );
   
  //  xTaskCreate(
  //    postTimeTask,
  //    "PostTime",
  //    10000,
  //    NULL,
  //   1 ,
  //   &postTimeTaskHandle
  //  );
   
   xTaskCreate(
    checkSensorsTask,
    "CheckSensors",
    10000,
    NULL,
    2, // Higher priority than other tasks
    &checkSensorsTaskHandle
   );
}

void loop() {
}

void getTimeTask(void * parameter) {
   for (;;) {
     HTTPClient http;
     http.begin("http://worldtimeapi.org/api/ip");
     int httpCode = http.GET();
     
      int retries =0 ;
     
      while (httpCode <=0 && retries <3) {
       Serial.println("Error getting time. Retrying...");
       delay(1000);
       httpCode = http.GET();
       retries++;
      }

      if (httpCode >0) {
        String payload = http.getString();
        preferences.putString("time", payload);
        Serial.println(payload);
      } else {
        Serial.println("Error getting time after three attempts.");
      }

      http.end();

      vTaskDelay(60000 / portTICK_PERIOD_MS); // Delay for one minute
   }
}

void postTimeTask(void * parameter) {
  for (;;) {
    if (WiFi.status() == WL_CONNECTED) { // Check WiFi connection status
      HTTPClient http;
      
      String time = preferences.getString("time", "");
      
      if (time.length() >0) {
        // Your Firebase database URL goes here
        String url = "https://ori-projects-default-rtdb.europe-west1.firebasedatabase.app/esp32project.json";
        
        http.begin(url);
        http.addHeader("Content-Type", "application/json");
        
        int httpResponseCode = http.PUT(time);
        
        if (httpResponseCode >0) {
          String response = http.getString();
          Serial.println(httpResponseCode);
          Serial.println(response);
        } else {
          Serial.print("Error on sending PUT: ");
          Serial.println(httpResponseCode);
         }
        
         http.end();
       }
    } else {
       Serial.println("WiFi Disconnected");
    }
    
    vTaskDelay(60000 / portTICK_PERIOD_MS); // Delay for one minute
   }
}

void checkSensorsTask(void * parameter) {
  for (;;) {
     changed = false;
    
    for (int i=0; i<numSensors; i++) {
      int sensorValue = digitalRead(sensorPins[i]);
      
      if (sensorValue != prevSensorValues[i]) {
        changed = true;
        prevSensorValues[i] = sensorValue;
      }
    }
    
    if (changed) {
      // Call function to send POST request with current sensor values
      sendPostWithSensorValues(prevSensorValues);
    }
    
    vTaskDelay(50 / portTICK_PERIOD_MS); // Delay for 50 milliseconds
  }
}

void sendPostWithSensorValues(int sensorValues[]) {
  // Create JSON object with sensor values
  String json = "{";
  
  for (int i=0; i<numSensors; i++) {
    json += "\"sensor";
    json += String(i+1);
    json += "\":";
    json += String(sensorValues[i]);
    
    if (i < numSensors-1) {
      json += ",";
     }
   }
  
   json += "}";
  
   Serial.println(json);
  
   // Send PUT request with JSON object
   if (WiFi.status() == WL_CONNECTED) { // Check WiFi connection status
     HTTPClient http;
     
     // Your Firebase database URL goes here
     String url = "https://ori-projects-default-rtdb.europe-west1.firebasedatabase.app/esp32project.json";
     
     http.begin(url);
     http.addHeader("Content-Type", "application/json");
     
     int httpResponseCode = http.PUT(json);
     
     if (httpResponseCode >0) {
       String response = http.getString();
       Serial.println(httpResponseCode);
       Serial.println(response);
     } else {
       Serial.print("Error on sending PUT: ");
       Serial.println(httpResponseCode);
      }
     
      http.end();
   } else {
     Serial.println("WiFi Disconnected");
   }
}