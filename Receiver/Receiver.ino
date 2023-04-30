#include <LiquidCrystal_I2C.h>
#include <Time.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  //initialize i2c lcd with address 0x27 and 16*2 lcd
int data[4], prevData[4];            // arrays to store data from the ir sensors
unsigned long sensor_time[4];        // Timers for each sensor
unsigned long prev_time = 0;
int vt = 7, d0 = 8, d1 = 9, d2 = 10, d3 = 11, index = 0;  //pins connected to ht12d
String dataText = "", dataTimer = "";

void setup() {
  Serial.begin(9600);
  lcd.begin();  //initialize the lcd
  // lcd.home();
  lcd.backlight();  //turn on the lcd backlight
  lcd.setCursor(4, 0);
  lcd.print("bootUp!");
  delay(2000);
  lcd.clear();
  pinMode(vt, INPUT);  //set vt pin as input
  pinMode(d0, INPUT);  //set d0 pin as input
  pinMode(d1, INPUT);  //set d1 pin as input
  pinMode(d2, INPUT);  //set d2 pin as input
  pinMode(d3, INPUT);  //set d3 pin as input
}
String formatElapsedTime(unsigned long elapsed_time) {
  unsigned long seconds = elapsed_time / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  seconds %= 60;
  minutes %= 60;
  hours %= 24;
  
  // int milliseconds = elapsed_time % 1000;
  return String(hours) + ":" + String(minutes) + ":" + String(seconds);
}
void loop() {
  if (digitalRead(vt) == HIGH) {  // check if VT pin is high
    data[0] = digitalRead(d0);    //read data from the first ir sensor
    data[1] = digitalRead(d1);    //read data from the second ir sensor
    data[2] = digitalRead(d2);    //read data from the third ir sensor
    data[3] = digitalRead(d3);    //read data from the fourth ir sensor

    bool update_display = false;
    dataText = "";
    for (int i = 0; i < 4; i++) {
      
      if (data[i] != prevData[i]) {
        update_display = true;
        prevData[i] = data[i];
        dataText += "Mail" + String(i + 1) + ": " + String(data[i]) + " ";
      } else dataText += "Mail" + String(i + 1) + ": " + String(data[i]) + " ";
      if (data[i] == 0) {             // check if sensor is receiving 0
        if (sensor_time[i] == 0) {    // check if timer for sensor is not started
          sensor_time[i] = millis();  // start timer for sensor
        }
      } else {
        if (sensor_time[i] != 0) {                                 // check if timer for sensor is started
          unsigned long elapsed_time = millis() - sensor_time[i];  // calculate elapsed time
          sensor_time[i] = 0;                                      // reset timer for sensor
        }
      }
    }
    if (millis() - prev_time >= 1001) {  // check if one minute has passed
      prev_time = millis();
      dataTimer = "";
      for (int i = 0; i < 4; i++) {
        if (sensor_time[i] != 0) {
          unsigned long elapsed_time = millis() - sensor_time[i];
          dataTimer += "Mail" + String(i + 1) + ": " + formatElapsedTime(elapsed_time) + " ";
        } else dataTimer += " ";
      }
    }
    if (update_display) {
      index = 0;
    }
  } else {
    dataText = "RF ERROR!       ";
    index = 0;
  }
  delay(500);
  if (index + 16 < dataText.length()||index + 16 < dataTimer.length()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(dataTimer.substring(index, index + 16));
    lcd.setCursor(0, 1);
    lcd.print(dataText.substring(index, index + 16));
    index += 1;
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(dataTimer.substring(index, index + 16));
    lcd.setCursor(0, 1);
    lcd.print(dataText.substring(index));
    index = 0;
  }
  delay(500);
}