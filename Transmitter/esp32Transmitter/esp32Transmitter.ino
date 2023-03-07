#include <WiFi.h>
#include <WiFiManager.h>
#include "BluetoothSerial.h"
#include "esp_blufi_api.h"
#include "esp_bt.h"

// Initialize BluetoothSerial object
BluetoothSerial SerialBT;

// Define BluFi callback functions
void blufi_event_callback(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param);
void blufi_dh_negotiate_data_handler(uint8_t *data, int len);
void blufi_security_connect_data_handler(uint8_t *data, int len);
void blufi_recv_wifi_conn_report_data_handler(uint8_t *data, int len);

// Define WiFiManager object
WiFiManager wm;

// Define WiFi credentials variables
char ssid[32] = "";
char password[64] = "";

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Initialize Bluetooth with device name
  SerialBT.begin("ESP32");

  // Register BluFi callback functions
  esp_blufi_callbacks_t blufi_callbacks;
blufi_callbacks.event_cb = blufi_event_callback; // your callback function
esp_blufi_register_callbacks(&blufi_callbacks); // pass a pointer to the structure
  esp_blufi_profile_init();

  // Start WiFiManager with autoConnect function
  wm.autoConnect();
}

void loop() {
  // Do nothing here
}

// BluFi event callback function
void blufi_event_callback(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param) {
  switch (event) {
    case ESP_BLUFI_EVENT_INIT_FINISH:
      Serial.println("BluFi initialized");
      break;
    case ESP_BLUFI_EVENT_DEINIT_FINISH:
      Serial.println("BluFi deinitialized");
      break;
    case ESP_BLUFI_EVENT_BLE_CONNECT:
      Serial.println("BluFi connected");
      break;
    case ESP_BLUFI_EVENT_BLE_DISCONNECT:
      Serial.println("BluFi disconnected");
      break;
    case ESP_BLUFI_EVENT_RECV_SLAVE_DISCONNECT_BLE:
      Serial.println("BluFi received slave disconnect request");
      break;
    case ESP_BLUFI_EVENT_REPORT_ERROR:
      Serial.println("BluFi sent Wi-Fi connection report");
      break;
    case ESP_BLUFI_EVENT_RECV_SLAVE_DISCONNECT_BLE: 
      blufi_recv_wifi_conn_report_data_handler(param->wifi_conn_report.data,
                                               param->wifi_conn_report.data_len);
      break;
    default:
      break;
  }
}

// BluFi data handler functions

void blufi_dh_negotiate_data_handler(uint8_t *data, int len) {
  // Handle Diffie-Hellman key exchange data here
}

void blufi_security_connect_data_handler(uint8_t *data, int len) {
  // Handle security connection data here
}

void blufi_recv_wifi_conn_report_data_handler(uint8_t *data, int len) {
  // Handle Wi-Fi connection report data here

  // Parse SSID and password from data buffer
  memcpy(ssid, &data[3], data[2]);
  memcpy(password, &data[data[2] + 5], data[data[2] +4]);

  // Print SSID and password for debugging purposes
  Serial.print("SSID: ");
  Serial.println(ssid);
  
   Serial.print("Password: ");
   Serial.println(password);

   // Connect to Wi-Fi using WiFiManager library 
   wm.setSTAStaticIPConfig(IPAddress(192,168,1,100), IPAddress(192,168,1,1), IPAddress(255,255,255,0));
   wm.connectWifi(ssid,password);

   // Save Wi-Fi credentials using WiFiManager library 
   wm.setSaveCredentials(true);

}