// Arduino Mega Receiver - Direct 5V Polling Mode
#include <SPI.h>
#include <LoRa.h>

const int csPin = 53;
const int resetPin = 49;
const int irqPin = 2; // Not strictly used in polling, but good to define

void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait for serial monitor
  
  Serial.println("Initializing LoRa...");
  LoRa.setPins(csPin, resetPin, irqPin);
  
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed! Check SPI wiring.");
    while (1);
  }
  Serial.println("Receiver ready! Waiting for packets...");
}

void loop() {
  // Check if a packet has arrived
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Received packet: '");
    
    // Read packet data
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }
 
    Serial.print("' with RSSI: ");
    Serial.println(LoRa.packetRssi());
  }
}
