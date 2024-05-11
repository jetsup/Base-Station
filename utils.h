#ifndef UTILS_H_
#define UTILS_H_

#include <Arduino.h>
#include <RF24.h>
#include <SPI.h>
#include <nRF24L01.h>

#define MAX_PAYLOAD_SIZE 31

static const uint64_t pipeOut =
    000322;         // NOTE: The same as in the receiver 000322
extern RF24 radio;  // select CE,CSN pin
extern String dronePositionBuffer;

extern int CONNECTION_STATUS;

extern int prevSignal;
extern long prevSerialLog, prevReception;

// double was rounding off
extern String baseLat, baseLng, baseAlt, currentLat, currentLng, currentAlt,
    currentCourse, currentSpeed, currentDistanceFromBase;
extern String currentTime;

struct Signal {
  byte analogServoValueByte;
  bool releaseCarriage;
};

struct GpsData {
  char latLng[100] = {};
};

extern Signal data;
extern GpsData gps;

extern int pushBtnPin;

extern bool shouldBlink, state, logOutput;
extern int blinkCount;
extern long prevBlinkTime;

void ResetData();
void blinkStyle();
bool nrfConnected();
void logConsoleData();
void sendControlSignal();
void receiveGpsData();
int Border_Map(int val, int lower, int middle, int upper, bool reverse);

#endif  // UTILS_H_