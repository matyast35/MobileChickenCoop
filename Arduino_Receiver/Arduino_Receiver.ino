// .................Arduino Receiver......................

#include <SPI.h>
#include <LoRa.h>

// region dependent values
const int opFreq = 915E6; // operating frequency 

// pin assignment 
const int csPin    = 53;
const int resetPin = 49;
const int irqPin   =  2;

unsigned long lastHeartbeat = 0;
int heartbeatCount = 0;

void setup() {
  Serial.begin(9600);

  // 2 second delay before running code if no serial monitor connected 
  unsigned long startWindow = millis();
  while (!Serial && (millis() - startWindow < 2000));

  Serial.println("Initializing LoRa...");    
  LoRa.setPins(csPin, resetPin, irqPin);          // LoRa initialization 
  
  if (!LoRa.begin(opFreq)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("Receiver ready!\n");
}

void loop() {
  // Heartbeat every 3 seconds so you can confirm
  // Mega is still running between packets
  if (millis() - lastHeartbeat > 3000) {
    Serial.print("Heartbeat #");
    Serial.println(heartbeatCount++);
    lastHeartbeat = millis();
  }

  // Check for incoming packet
  int packetSize = LoRa.parsePacket(0);

  if (packetSize) {
    String received = "";
    while (LoRa.available()) {
      received += (char)LoRa.read();
    }

    int rssi = LoRa.packetRssi();

    Serial.print("Received: ");
    Serial.println(received);
    Serial.print("RSSI: ");
    Serial.println(rssi);

    // Validate packet is from our ESP32-S3
    if (received.startsWith("ALERT:")) {
      Serial.println("Valid alert packet — sending confirmation");

      // Small delay to let ESP32-S3 switch to receive mode
      delay(100);

      // Send confirmation back
      LoRa.idle();
      delay(50);

      LoRa.beginPacket();
      LoRa.print("CONFIRM: ");
      LoRa.print(received);  // echo back what was received
      LoRa.endPacket(true);  // blocking transmit

      Serial.println("Confirmation sent");

      // Return to receive mode
      LoRa.receive();

    } else {
      Serial.print("Rejected invalid packet: ");
      Serial.println(received);
      LoRa.receive();
    }
  }
}
