#include "utils.h"

#include "Arduino.h"

RF24 radio(9, 10);  // select CE,CSN pin
String dronePositionBuffer = "";

int CONNECTION_STATUS, GREEN_STATUS = 3, RED_STATUS = 4, BLUE_STATUS = 2;

int prevSignal = 0;
long prevSerialLog = 0, prevReception = 0, prevRapidToggle = 0;

// double was rounding off
String baseLat, baseLng, baseAlt, currentLat, currentLng, currentAlt,
    currentCourse, currentSpeed, currentDistanceFromBase;
String currentTime;

Signal data;
GpsData gps;

int pushBtnPin = 6;

bool shouldBlink = false, state = false, logOutput = false,
     packageDropped = false, rapidToggle = false;
int blinkCount = 0;
long prevBlinkTime = 0;

void turnOffLEDs(int led1, int led2) {
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
}

void patternBlink() {
  if (blinkCount < 6) {
    if (millis() - prevBlinkTime > 100) {
      state = !state;
      blinkCount++;
      digitalWrite(CONNECTION_STATUS, state);
      prevBlinkTime = millis();
    }
  } else {
    if (millis() - prevBlinkTime > 1000) {
      blinkCount = 0;
      digitalWrite(CONNECTION_STATUS, LOW);
    }
  }
}

// TODO remove redundancy
void blinkStyle() {
  if (shouldBlink) {
    if (millis() - prevReception < 500) {
      if (!packageDropped) {
        CONNECTION_STATUS = GREEN_STATUS;  // package on board
        turnOffLEDs(RED_STATUS, BLUE_STATUS);
      } else {
        CONNECTION_STATUS = BLUE_STATUS;  // package dropped
        turnOffLEDs(RED_STATUS, GREEN_STATUS);
      }

      patternBlink();

    } else {
      // lost contact with the air module
      if (millis() - prevReception > 5000) {
        turnOffLEDs(GREEN_STATUS, BLUE_STATUS);
        CONNECTION_STATUS = RED_STATUS;
        patternBlink();
      } else {
        if (!packageDropped) {
          CONNECTION_STATUS = GREEN_STATUS;
        } else {
          CONNECTION_STATUS = BLUE_STATUS;
        }
        blinkRapidly();
      }
    }
  } else {
    // digitalWrite(CONNECTION_STATUS, LOW);
    CONNECTION_STATUS = RED_STATUS;
    patternBlink();
  }
}

void blinkRapidly() {
  if (millis() - prevRapidToggle > 50) {
    rapidToggle = !rapidToggle;
    digitalWrite(CONNECTION_STATUS, rapidToggle);
    prevRapidToggle = millis();
  }
}

int Border_Map(int val, int lower, int middle, int upper, bool reverse) {
  val = constrain(val, lower, upper);
  if (val < middle)
    val = map(val, lower, middle, 0, 128);
  else
    val = map(val, middle, upper, 128, 255);
  return (reverse ? 255 - val : val);
}

void nrfInit() {
  radio.begin();
  radio.openWritingPipe(pipeOut);

  radio.setChannel(100);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.stopListening();
}

bool nrfConnected() {
  if (millis() - prevReception > 5000) {
    shouldBlink = false;
    // TODO add a timeout
    nrfInit();
    return false;
  }

  if (!currentLat.equals("") && !currentLng.equals("") &&
      !currentTime.equals("")) {
    return true;
  }
  return false;
}

void logConsoleData() {
  // TODO: Simplify all this using a variable
  String consoleData = "<" + baseLat + "," + baseLng + "," + baseAlt + "|" +
                       currentLat + "," + currentLng + "," + currentTime + "," +
                       currentAlt + "," + currentSpeed + "," + currentCourse +
                       "," + currentDistanceFromBase + ">";

  Serial.println(consoleData);
}

void sendControlSignal() {
  radio.openWritingPipe(pipeOut);
  radio.stopListening();
  delay(10);

  prevSignal = data.analogServoValueByte;
  radio.write(&data, sizeof(data));
}

void receiveGpsData() {
  char gpsData[32] = {};
  radio.openReadingPipe(1, pipeOut);
  radio.startListening();
  delay(10);

  if (radio.available()) {
    radio.read(gpsData, sizeof(gpsData));

    String buf = String(gpsData);

    int commaPos;
    if (buf.startsWith("<B") && buf.endsWith("B>")) {
      buf.replace("<B", "");
      buf.replace("B>", "");
      commaPos = buf.indexOf(",");
      baseLat = buf.substring(0, commaPos);
      baseLng = buf.substring(commaPos + 1);
      prevReception = millis();
    } else if (buf.startsWith("<BA") && buf.endsWith("BA>")) {
      buf.replace("<BA", "");
      buf.replace("BA>", "");
      baseAlt = buf.toFloat();
      prevReception = millis();
    } else if (buf.startsWith("<C") && buf.endsWith("C>")) {
      buf.replace("<C", "");
      buf.replace("C>", "");
      commaPos = buf.indexOf(",");
      currentLat = buf.substring(0, commaPos);
      currentLng = buf.substring(commaPos + 1);
      prevReception = millis();
    } else if (buf.startsWith("<CA") && buf.endsWith("CA>")) {
      buf.replace("<CA", "");
      buf.replace("CA>", "");
      currentAlt = buf.toFloat();
      prevReception = millis();
    } else if (buf.startsWith("<T") && buf.endsWith("T>")) {
      buf.replace("<T", "");
      buf.replace("T>", "");
      currentTime = buf;
      prevReception = millis();
    } else if (buf.startsWith("<S") && buf.endsWith("S>")) {
      buf.replace("<S", "");
      buf.replace("S>", "");
      currentSpeed = buf.toFloat();
      prevReception = millis();
    } else if (buf.startsWith("<Q") && buf.endsWith("Q>")) {
      buf.replace("<Q", "");
      buf.replace("Q>", "");
      currentCourse = buf;
      prevReception = millis();
    } else if (buf.startsWith("<D") && buf.endsWith("D>")) {
      buf.replace("<D", "");
      buf.replace("D>", "");
      currentDistanceFromBase = buf;
      prevReception = millis();
    } else if (buf.startsWith("<PS") && buf.endsWith("PS>")) {
      buf.replace("<PS", "");
      buf.replace("PS>", "");
      packageDropped = (bool)buf.toInt();
      prevReception = millis();
    }
  }
}

void ResetData() {
  data.analogServoValueByte = 0;  // Signal lost position
  data.releaseCarriage = false;
}
