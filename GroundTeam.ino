// 1 Channel Transmitter | 1 Kanal Verici
// Input pin A5

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define MAX_PAYLOAD_SIZE 31

const uint64_t pipeOut = 000322;  // NOTE: The same as in the receiver 000322 | Alıcı kodundaki adres ile aynı olmalı
RF24 radio(9, 10);                // select CE,CSN pin | CE ve CSN pinlerin seçimi
String dronePositionBuffer = "";

int prevSignal = 0;

struct Signal {
  byte analogServoValueByte;
  bool releaseCarriage;
};

struct GpsData {
  char latLng[100] = {};
};

Signal data;
GpsData gps;

int pushBtnPin = 6;

void ResetData() {
  data.analogServoValueByte = 0;  // Signal lost position | Sinyal kesildiğindeki pozisyon
  data.releaseCarriage = false;
}

void setup() {
  Serial.begin(115200);
  pinMode(pushBtnPin, INPUT_PULLUP);
  //Configure the NRF24 module  | NRF24 modül konfigürasyonu
  if (!radio.begin()) { Serial.println("NRF INIT FAILED"); }
  radio.openWritingPipe(pipeOut);

  //
  // radio.openReadingPipe(2, pipeOut);
  //

  radio.setChannel(100);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);  // The lowest data rate value for more stable communication  | Daha kararlı iletişim için en düşük veri hızı.
  radio.setPALevel(RF24_PA_MAX);    // Output power is set for maximum |  Çıkış gücü maksimum için ayarlanıyor.
  radio.stopListening();            // Start the radio comunication for Transmitter | Verici için sinyal iletişimini başlatır.
  ResetData();
}

// Joystick center and its borders | Joystick merkez ve sınırları
int Border_Map(int val, int lower, int middle, int upper, bool reverse) {
  val = constrain(val, lower, upper);
  if (val < middle)
    val = map(val, lower, middle, 0, 128);
  else
    val = map(val, middle, upper, 128, 255);
  return (reverse ? 255 - val : val);
}

bool logOutput = false;

void loop() {
  // Control Stick Calibration for channels
  data.analogServoValueByte = Border_Map(analogRead(A5), 0, 512, 1023, false);  // "true" or "false" for change signal direction | "true" veya "false" sinyal yönünü değiştirir.
  data.releaseCarriage = !digitalRead(pushBtnPin);


  if (data.analogServoValueByte != prevSignal || data.releaseCarriage) {
    Serial.print(map(analogRead(A5), 0, 1023, 0, 180));
    Serial.print(" : ");
    Serial.print(data.analogServoValueByte);
    Serial.print(" : ");
    Serial.println(data.releaseCarriage);

    sendControlSignal();
  }

  // transition to receive mode and receive geo coordinates
  receiveGpsData();
}
void sendControlSignal() {
  radio.openWritingPipe(pipeOut);
  radio.stopListening();
  delay(10);

  prevSignal = data.analogServoValueByte;
  radio.write(&data, sizeof(data));
  Serial.println("Command sent");
}

void receiveGpsData() {
  char gpsData[100] = {};
  radio.openReadingPipe(1, pipeOut);
  radio.startListening();
  delay(10);

  if (radio.available()) {
    radio.read(gpsData, sizeof(gpsData));
    // radio.startListening();
    // Serial.print("$:");
    String buf = String(gpsData);
    if (buf.startsWith("<$") || dronePositionBuffer.length() > 110) {
      dronePositionBuffer = buf;
      // while (!buf.endsWith(">>")) {
      //   radio.read(gpsData, sizeof(gpsData));
      //   buf = String(gpsData);
      //   dronePositionBuffer += buf;
      // }
    } else {
      dronePositionBuffer += buf;
    }

    // Serial.println(dronePositionBuffer);

    if (dronePositionBuffer.startsWith("<$") && dronePositionBuffer.endsWith("#>")
        && dronePositionBuffer.length() > 95 && dronePositionBuffer.length() < 105) {
      // complete sentense received
      Serial.println(dronePositionBuffer);
      dronePositionBuffer = "";
    }
  }
}