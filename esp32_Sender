// XIAO_ESP32S3

#include <SPI.h>
#include <LoRa.h>

const int csPin = 44;
const int resetPin = 43;
const int irqPin = 4;
byte msgCount = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("Sender ready");
}

void loop() {
  Serial.print("Sending packet: ");
  Serial.println(msgCount);
  LoRa.beginPacket();
  LoRa.print("Packet ");
  LoRa.print(msgCount);
  LoRa.endPacket();
  msgCount++;
  delay(5000);
}