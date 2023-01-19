#include <LiquidCrystal_I2C.h>


LiquidCrystal_I2C lcd(0x27,16,2); //initialize i2c lcd with address 0x27 and 16*2 lcd
int data[4], prevData[4]; // arrays to store data from the ir sensors
int vt = 7,d0 = 8,d1 = 9,d2 = 10,d3 = 11; //pins connected to ht12d
int index = 0;
String dataText = "";

void setup() {
  lcd.begin(); //initialize the lcd
  //lcd.backlight(); //turn on the lcd backlight
  lcd.home();
  pinMode(vt, INPUT); //set vt pin as input
  pinMode(d0, INPUT); //set d0 pin as input
  pinMode(d1, INPUT); //set d1 pin as input
  pinMode(d2, INPUT); //set d2 pin as input
  pinMode(d3, INPUT); //set d3 pin as input
}

void loop() {
  if (digitalRead(vt) == HIGH) { // check if VT pin is high
    data[0] = digitalRead(d0); //read data from the first ir sensor
    data[1] = digitalRead(d1); //read data from the second ir sensor
    data[2] = digitalRead(d2); //read data from the third ir sensor
    data[3] = digitalRead(d3); //read data from the fourth ir sensor

    bool update_display = false;
    for (int i = 0; i < 4; i++) {
      if (data[i] != prevData[i]) {
        update_display = true;
        prevData[i] = data[i];
        dataText += "Mail" + String(i+1) + ": " + String(data[i]) + " ";
      }
      else dataText += "Mail" + String(i+1) + ": " + String(data[i]) + " ";
    }
    if (update_display) {
      index = 0;
    }
  }else {dataText="RF ERROR!       ";}
  delay(1000);  // wait for 400 milliseconds
  lcd.clear(); //clear the lcd
  lcd.print("IR Sensor Data:"); //print the title on the first line of lcd

  lcd.setCursor(0,1); //set cursor to second line
  if(index+16 < dataText.length()){
    lcd.print(dataText.substring(index, index+16));
    index += 1;
  }else{
    lcd.print(dataText.substring(index));
    index = 0;
  }
  
}
