
#include "ControlLogic.h"


ControlLogic control;

bool toggle = false;

const int LED_PIN = 2;
const int interruptPin = 23;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);  // set the LED pin mode
  delay(10);

  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), flowMeterRead, RISING);

  control.Configure();
  
}

void loop() {

  // One second delay.
  delay(1000);

  // Toggle the LED so we know the loop is active.  Quick Sanity
  // If we can't connect to Wifi/mqtt it stops
  toggle = !toggle;
  digitalWrite(LED_PIN, toggle);

  control.loop();

}

void flowMeterRead()
{
  toggle = !toggle;
  digitalWrite(LED_PIN, toggle);
  control.HandleFlowMeterRead();
}


