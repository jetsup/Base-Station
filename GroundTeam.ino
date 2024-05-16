#include "utils.h"

void setup() {
  Serial.begin(115200);
  pinMode(pushBtnPin, INPUT_PULLUP);
  pinMode(CONNECTION_STATUS, OUTPUT);
  // Configure the NRF24 module
  nrfInit();
  ResetData();
}

void loop() {
  // Control Stick Calibration for channels
  blinkStyle();
  data.analogServoValueByte =
      Border_Map(analogRead(A5), 0, 512, 1023,
                 false);  // "true" or "false" for change signal direction
  data.releaseCarriage = !digitalRead(pushBtnPin);

  if (data.analogServoValueByte != prevSignal || data.releaseCarriage) {
    sendControlSignal();
  }

  // transition to receive mode and receive geo coordinates
  receiveGpsData();
  if (millis() - prevSerialLog > 20) {
    prevSerialLog = millis();
    // TODO: check if there is data first
    logConsoleData();
  }

  if (nrfConnected()) {
    shouldBlink = true;
  }
}
