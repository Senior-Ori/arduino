#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

const unsigned int MAX_INPUT = 256;
const byte LEDs[] ={0b00000001,0b00000010,0b00000100,0b00001000,0b00010000,0b00100000,0b01000000,0b10000000};
byte ledMapB = 0b00000000;

void setup() {
@ -23,60 +24,67 @@

  const char* command = doc["command"];
  int ledOnOff[4];
  for (int i = 0; i < 4; i++) ledOnOff[i] = doc["leds"][i];





  if (ledOnOff[0] == 1) {
    ledMapB = 0b00100000;
  int count=0,flag=1;
  for (int i = 0; i < 4; i++) {
    ledOnOff[i] = doc["leds"][i];
    if (ledOnOff[i]==1) {
    ledMapB |= LEDs[i+2];
    PORTB = ledMapB;
    Serial.write("Led Is On");
  }
  if (ledOnOff[0] == 0) {
    ledMapB = 0b00000000;
    count++;
    //Serial.write("Led Is On");
  } else if(ledOnOff[i]==0) {
    ledMapB &= ~LEDs[i+2];
    PORTB = ledMapB;
    Serial.write("Led Is Off");
    //Serial.write("Led Is Off");
  }else flag=0;
  }
  if(flag)
  if (count) Serial.write("Led Is On");
  else Serial.write("Led Is Off");



  
  //for (int i = 0; i < 4; i++)
    

  if (String(command) == "triggerLed") {
    //i think there is a interruption because of the interruption bit
    ledMapB ^= -1;
    PORTB = ledMapB;
    Serial.write("all LED has been triggered");
  }
}

void processIncomingByte(const byte inByte) {
  static char inputLine[MAX_INPUT];
  static unsigned int inputPos = 0;

  switch (inByte) {
    case '\n':
      inputLine[inputPos] = 0;  //terminating null byte

      //terminator reached! process inputLine here...
      processData(inputLine);


      //reset buffer for next time
      inputPos = 0;
      //return inputLine;
      break;

    case '\r': break;  //discard carriae return


    default:
      // keep adding if not full ...allow for terminating null byte
      if (inputPos < (MAX_INPUT - 1)) inputLine[inputPos++] = inByte;
      break;
  }
}

void loop() {
  while (Serial.available() > 0)
    processIncomingByte(Serial.read());
}
