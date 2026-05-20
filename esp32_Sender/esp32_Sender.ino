// .....................esp32 Sender.........................

#include <SPI.h>
#include <LoRa.h>

#define TIMEOUT_DURATION 5000

// region dependent values
const int opFreq = 915E6; // operating frequency 

// pin assignment 
const int csPin = 44;
const int resetPin = 43;
const int irqPin = 4;
const int pirPin = 6;

// state tracking 
bool alertPending = false;
bool waitingForAck = false;
unsigned long ackTimer = 0;

// data tracking 
byte msgCount = 0;


// initializes pins and LoRa communication 
void setup() {
  Serial.begin(9600);

  // 2 second delay before running code if no serial monitor connected 
  unsigned long startWindow = millis();
  while (!Serial && (millis() - startWindow < 2000));

  Serial.println("Initializing LoRa...");
  LoRa.setPins(csPin, resetPin, irqPin);         // LoRa initialization 
  pinMode(pirPin, INPUT);

  if (!LoRa.begin(opFreq)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("Sender ready!\n");
}



void loop() {
  if (!waitingForAck) {
    int pirDetect = digitalRead(pirPin);

    if (pirDetect == HIGH) {
      alertPending = true;
    }
  }

  // sending 
  if (alertPending && !waitingForAck) {
    Serial.print("Sending packet #");
    Serial.println(msgCount);

    LoRa.beginPacket();
    LoRa.print("ALERT:");
    LoRa.print(msgCount);
    LoRa.endPacket();
    Serial.println("Packet sent — waiting for confirmation...");

    alertPending = false;
    waitingForAck = true;
    ackTimer = millis(); // Start the countdown clock
    
    LoRa.receive(); // Instantly pivot back to listening mode
  }

  if (waitingForAck) {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      String incoming = "";
      while (LoRa.available()) {
        incoming += (char)LoRa.read();
      }

      if (incoming.startsWith("CONFIRM:")) {
        Serial.print("Confirmation received: ");
        Serial.println(incoming);
        
        msgCount++;
        waitingForAck = false; // Release the lock
      }
    }

    // Handle Timeout without freezing the loop
    if (waitingForAck && (millis() - ackTimer >= TIMEOUT_DURATION)) {
      Serial.println("Error: No confirmation received within timeout window. Retrying sensor loop.\n");
      waitingForAck = false; // Clear lock to allow re-triggering
    }
  }
}