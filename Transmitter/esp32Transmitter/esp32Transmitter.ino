#include <WiFi.h>
#include <Preferences.h>
#include <HTTPClient.h>

Preferences preferences;

TaskHandle_t getTimeTaskHandle;
TaskHandle_t postTimeTaskHandle;
TaskHandle_t checkSensorsTaskHandle;

/** DEFINE GIOS**/
#define IR_SENSOR_0 34
#define IR_SENSOR_1 35
#define IR_SENSOR_2 32
#define IR_SENSOR_3 33

#define LED_FLAG 14

#define HT12E_D0 25
#define HT12E_D1 26
#define HT12E_D2 27
#define HT12E_D3 12
#define HT12E_TE 13

/** DEFINES **/
#define MAX_FAILURES 14  //max retries get time now!

/** GLOBALS **/
int ir_sensor_data[4] = { 0, 0, 0, 0 };
int previous_ir_sensor_data[4] = { -1, -1, -1, -1 };
String ir_times[] = { "0", "0", "0", "0" };
bool isChanged = true;
String payload;

/** Timers **/
unsigned long int unixtime;
unsigned long int msCounter;
String unixtimeStr;


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
  pinMode(LED_FLAG, OUTPUT);
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

  if (ssid.length() > 0) {
    WiFi.begin(ssid.c_str(), password.c_str());
    if (WiFi.waitForConnectResult() == WL_CONNECTED) {
      Serial.println("Connected to WiFi using saved credentials.");
      Serial.print("ssid: ");
      Serial.print(ssid.c_str());
      Serial.print("\npassword: ");
      Serial.println(password.c_str());


      //receive time now!
      HTTPClient http;
      http.begin("http://worldtimeapi.org/api/ip");
      int httpCode = http.GET();

      int retries = 0;
      int j = 10;
      while (httpCode <= 0 && retries < MAX_FAILURES) {
        Serial.println("Error getting time. Retrying...");

        delay(j);
        j = (j + 10) * 2;
        if (j > 720) j = 100;
        Serial.println(j);
        httpCode = http.GET();
        retries++;
      }
      payload = "";
      if (httpCode > 0) {

        payload = http.getString();
        unixtimeStr = payload.substring(payload.indexOf("\"unixtime\":") + 11);  // extract the value of the "unixtime" key
        unixtimeStr = unixtimeStr.substring(0, unixtimeStr.indexOf(','));        // remove the comma and any other characters after it
        unixtime = unixtimeStr.toInt();                                          // convert the string to an integer
        unixtime = unixtime + millis();
        Serial.println(payload);
        Serial.println(unixtimeStr);
        Serial.println(unixtime);

      } else {
        Serial.println("Error getting time after three attempts.");
        payload = "\"Error getting time after three attempts.\"";
      }




      xTaskCreate(
        checkSensorsTask,
        "CheckSensors",
        10000,
        NULL,
        2,  // Higher priority than other tasks
        &checkSensorsTaskHandle);

      return;
    }
  }

  WiFi.mode(WIFI_AP_STA);
  WiFi.beginSmartConfig();

  while (!WiFi.smartConfigDone()) {
    digitalWrite(LED_FLAG, HIGH);
    delay(500);
    digitalWrite(LED_FLAG, LOW);
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("SmartConfig received.");
  digitalWrite(LED_FLAG, LOW);

  ssid = WiFi.SSID();
  password = WiFi.psk();

  preferences.putString("ssid", ssid);
  preferences.putString("password", password);

  //receive time now!
  HTTPClient http;
  http.begin("http://worldtimeapi.org/api/ip");
  int httpCode = http.GET();

  int retries = 0;
  int j = 10;
  while (httpCode <= 0 && retries < MAX_FAILURES) {
    Serial.println("Error getting time. Retrying...");

    delay(j);
    j = (j + 10) * 2;
    if (j > 720) j = 100;
    Serial.println(j);
    httpCode = http.GET();
    retries++;
  }
  payload = "";
  if (httpCode > 0) {

    payload = http.getString();
    unixtimeStr = payload.substring(payload.indexOf("\"unixtime\":") + 11);  // extract the value of the "unixtime" key
    unixtimeStr = unixtimeStr.substring(0, unixtimeStr.indexOf(','));        // remove the comma and any other characters after it
    unixtime = unixtimeStr.toInt();                                          // convert the string to an integer
    unixtime = unixtime + millis();
    Serial.println(payload);
    Serial.println(unixtimeStr);
    Serial.println(unixtime);

  } else {
    Serial.println("Error getting time after three attempts.");
    payload = "\"Error getting time after three attempts.\"";
  }

  http.end();


  xTaskCreate(
    checkSensorsTask,
    "CheckSensors",
    10000,
    NULL,
    2,  // Higher priority than other tasks
    &checkSensorsTaskHandle);
}

void loop() {
}


void checkSensorsTask(void* parameter) {
  for (;;) {
    isChanged = false;
    ir_sensor_data[0] = digitalRead(IR_SENSOR_0);
    ir_sensor_data[1] = digitalRead(IR_SENSOR_1);
    ir_sensor_data[2] = digitalRead(IR_SENSOR_2);
    ir_sensor_data[3] = digitalRead(IR_SENSOR_3);
    Serial.println("sensors has been initialized");
    Serial.print("\n[");
    Serial.print(ir_sensor_data[0]);
    Serial.print(",");
    Serial.print(ir_sensor_data[1]);
    Serial.print(",");
    Serial.print(ir_sensor_data[2]);
    Serial.print(",");
    Serial.print(ir_sensor_data[3]);
    Serial.print("]\n");
    for (int i = 0; i < numSensors; i++)
      if (ir_sensor_data[i] != previous_ir_sensor_data[i]) {
        isChanged = true;
      }

    //RF UPDATE VALUES (Send data to ht12e)
    digitalWrite(HT12E_D0, ir_sensor_data[0]);
    digitalWrite(HT12E_D1, ir_sensor_data[1]);
    digitalWrite(HT12E_D2, ir_sensor_data[2]);
    digitalWrite(HT12E_D3, ir_sensor_data[3]);
    // Trigger data transmission
    digitalWrite(HT12E_TE, HIGH);
    delay(10);
    // vTaskDelay(10 / portTICK_PERIOD_MS);
    digitalWrite(HT12E_TE, LOW);
    Serial.print("current state has executed order to RF!");

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

    vTaskDelay(1000 / portTICK_PERIOD_MS);  // Delay for 50 milliseconds
  }
}

void sendPostWithSensorValues() {

  // Create JSON object with sensor values
  String json = "{";

  for (int i = 0; i < numSensors; i++) {
    json += "\"sensor";
    json += String(i + 1);
    json += "\":";
    json += "[";
    json += String(ir_sensor_data[i]);
    json += ",";
    if (ir_sensor_data[i]) {
      ir_times[i] = "0";
      json += "0";
    } else if (ir_times[i] == "0") {
      ir_times[i] = "{\"unixtime\":";
      ir_times[i] += String(unixtime+millis());
      ir_times[i] += "}";
      json += String(ir_times[i]);
    } else {
      json += String(ir_times[i]);
    }
    json += "]";
    if (i < numSensors - 1) {
      json += ",";
    }
  }
  // Finish JSON object
  json += "}";

  Serial.println(json);

  // Send PUT request with JSON object
  if (WiFi.status() == WL_CONNECTED) {  // Check WiFi connection status
    HTTPClient http;

    // Your Firebase database URL goes here
    String url = "https://ori-projects-default-rtdb.europe-west1.firebasedatabase.app/esp32project.json";

    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.PUT(json);

    if (httpResponseCode > 0) {
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
    digitalWrite(LED_FLAG, HIGH);
  }
}