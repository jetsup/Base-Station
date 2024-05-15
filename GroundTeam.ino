#include "utils.h"

void setup() {
  Serial.begin(115200);
  pinMode(pushBtnPin, INPUT_PULLUP);
  pinMode(CONNECTION_STATUS, OUTPUT);
  // Configure the NRF24 module
  if (!radio.begin()) {
    Serial.println("NRF INIT FAILED");
  }
  radio.openWritingPipe(pipeOut);

  radio.setChannel(100);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.stopListening();
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
