
#include "ControlLogic.h"


ControlLogic control;

void setup() {
  Serial.begin(115200);
  pinMode(5, OUTPUT);  // set the LED pin mode
  pinMode(2, OUTPUT);  // set the LED pin mode
  delay(10);

  control.Configure();
  
}

void loop() {

  delay(4500);

  control.loop();

}


