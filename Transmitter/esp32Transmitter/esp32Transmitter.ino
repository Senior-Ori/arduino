#include <WiFi.h>
#include <Preferences.h>
#include <HTTPClient.h>

Preferences preferences;

TaskHandle_t getTimeTaskHandle;
TaskHandle_t postTimeTaskHandle;
TaskHandle_t checkSensorsTaskHandle;

/** DEFINE GIOS**/
#define IR_SENSOR_0 32
#define IR_SENSOR_1 33
#define IR_SENSOR_2 34
#define IR_SENSOR_3 35
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
String ir_times[]= {"0","0","0","0"};
bool isChanged = true;

// Replace these with your sensor pins
// const int sensorPins[] = {32, 33, 34, 35};
const int numSensors = sizeof(ir_sensor_data) / sizeof(ir_sensor_data[0]);

// int prevSensorValues[numSensors];



void setup() {
  Serial.begin(115200);
  pinMode(IR_SENSOR_0, INPUT);
  pinMode(IR_SENSOR_1, INPUT);
  pinMode(IR_SENSOR_2, INPUT);
  pinMode(IR_SENSOR_3, INPUT);
  pinMode(HT12E_D0, OUTPUT);
  pinMode(HT12E_D1, OUTPUT);
  pinMode(HT12E_D2, OUTPUT);
  pinMode(HT12E_D3, OUTPUT);
  pinMode(HT12E_TE, OUTPUT);
  

  preferences.begin("wifi", false);
  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");
  
  // for (int i=0; i<numSensors; i++) {
  //   prevSensorValues[i] = -1;
  //  }
  
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
    isChanged = false;
    ir_sensor_data[0] = digitalRead(IR_SENSOR_0);
    ir_sensor_data[1] = digitalRead(IR_SENSOR_1);
    ir_sensor_data[2] = digitalRead(IR_SENSOR_2);
    ir_sensor_data[3] = digitalRead(IR_SENSOR_3);
    Serial.println("sensors has been initialized");
    Serial.print("\n[");
    Serial.print(ir_sensor_data[0]);Serial.print(",");
    Serial.print(ir_sensor_data[1]);Serial.print(",");
    Serial.print(ir_sensor_data[2]);Serial.print(",");
    Serial.print(ir_sensor_data[3]);Serial.print("]\n");
    for (int i=0; i<numSensors; i++)
    if (ir_sensor_data[i] != previous_ir_sensor_data[i]) {
      isChanged = true;
    }
    
      //RF UPDATE VALUES (Send data to ht12e)
      digitalWrite(HT12E_D0,ir_sensor_data[0]);
      digitalWrite(HT12E_D1,ir_sensor_data[1]);
      digitalWrite(HT12E_D2,ir_sensor_data[2]);
      digitalWrite(HT12E_D3,ir_sensor_data[3]);
      // Trigger data transmission
      digitalWrite(HT12E_TE, HIGH);
      vTaskDelay(10 / portTICK_PERIOD_MS);
      digitalWrite(HT12E_TE, LOW);      
    
    if (isChanged) {
      // Call function to send POST request with current sensor values

      // sensor values has been changed
      Serial.println("sensor values has been changed");
      previous_ir_sensor_data[0] = ir_sensor_data[0];
      previous_ir_sensor_data[1] = ir_sensor_data[1];
      previous_ir_sensor_data[2] = ir_sensor_data[2];
      previous_ir_sensor_data[3] = ir_sensor_data[3];

      
      sendPostWithSensorValues();
    }
    
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 50 milliseconds
  }
}

void sendPostWithSensorValues() {
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
      String payload="";
      if (httpCode >0) {
        payload = http.getString();
        Serial.println(payload);
      } else {
        Serial.println("Error getting time after three attempts.");
        payload ="Error getting time after three attempts.";
      }

      http.end();
      

  // Create JSON object with sensor values
  String json = "{";
  
  for (int i=0; i<numSensors; i++) {
    json += "\"sensor";
    json += String(i+1);
    json += "\":";
    json += "[";
    json += String(ir_sensor_data[i]);
    json += ",";
    if (ir_sensor_data[i]) {
    ir_times[i]="0";
    json += "0";
    }else if(ir_times[i]=="0"){
      ir_times[i]=String(payload);
      json += String(ir_times[i]);
    }else{
      json += String(ir_times[i]);
    }
    json += "]";
    if (i < numSensors-1) {
      json += ",";
     }
   }
  // Finish JSON object
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