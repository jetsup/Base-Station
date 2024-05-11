#include "utils.h"

RF24 radio(9, 10);  // select CE,CSN pin
String dronePositionBuffer = "";

int CONNECTION_STATUS = A4;

int prevSignal = 0;
long prevSerialLog = 0, prevReception = 0;

// double was rounding off
String baseLat, baseLng, baseAlt, currentLat, currentLng, currentAlt,
    currentCourse, currentSpeed, currentDistanceFromBase;
String currentTime;

Signal data;
GpsData gps;

int pushBtnPin = 6;

bool shouldBlink = false, state = false, logOutput = false;
int blinkCount = 0;
long prevBlinkTime = 0;

void blinkStyle() {
  if (shouldBlink) {
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
  } else {
    digitalWrite(CONNECTION_STATUS, LOW);
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

bool nrfConnected() {
  if (millis() - prevReception > 5000) {
    return false;
  }

  if (!currentLat.equals("") && !currentLng.equals("") &&
      !currentTime.equals("")) {
    return true;
  }
  return false;
}

void logConsoleData() {
  Serial.print("<");
  Serial.print(baseLat);
  Serial.print(",");
  Serial.print(baseLng);
  Serial.print(",");
  Serial.print(baseAlt);
  Serial.print("|");
  Serial.print(currentLat);
  Serial.print(",");
  Serial.print(currentLng);
  Serial.print(",");
  Serial.print(currentTime);
  Serial.print(",");
  Serial.print(currentAlt);
  Serial.print(",");
  Serial.print(currentSpeed);
  Serial.print(",");
  Serial.print(currentCourse);
  Serial.print(",");
  Serial.print(currentDistanceFromBase);
  Serial.println(">");
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
    }
  }
}

void ResetData() {
  data.analogServoValueByte = 0;  // Signal lost position
  data.releaseCarriage = false;
}
