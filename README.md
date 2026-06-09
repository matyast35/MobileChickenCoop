# Mobile Predator-Proof Chicken Coop: LoRa Telemetry Subsystem

This subsystem provides long-range, low-power telemetry between the mobile chicken coop (Sender) and a base monitoring station (Receiver). It ensures immediate, verified transmission of predator alerts using a non-blocking handshaking protocol over sub-GHz RF.

-------------------------------------------------------------------------------
1. SYSTEM ARCHITECTURE & HARDWARE ROLES
-------------------------------------------------------------------------------

ESP32 Sender (Coop Node)
- Purpose: Monitors peripheral coop sensors (PIR motion detectors) and immediately broadcasts alerts upon predator detection.
- Microcontroller: ESP32 / ESP32-S3
- Pin Configuration:
    * CS (Chip Select): Pin 44
    * RESET: Pin 43
    * IRQ (Interrupt): Pin 4
    * PIR Sensor Input: Pin 6 (Active HIGH)

Arduino Receiver (Base Station Node)
- Purpose: Continuously listens for alerts, validates data origins, echoes confirmations back to the coop node, and logs localized Diagnostic Signal Strength Indicators (RSSI).
- Microcontroller: ATmega2560 (Arduino Mega)
- Pin Configuration:
    * CS (Chip Select): Pin 53
    * RESET: Pin 49
    * IRQ (Interrupt): Pin 2

-------------------------------------------------------------------------------
2. RF OPERATIONAL PARAMETERS
-------------------------------------------------------------------------------

- Operating Frequency: 915 MHz (US ISM Band, region-dependent software constant `opFreq = 915E6`).
- Communication Protocol: SPI interface to Semtech SX127x transceiver modules via the `LoRa.h` library.
- Network Layer: Custom synchronous handshaking protocol with a strict 5-second non-blocking timeout window.

-------------------------------------------------------------------------------
3. COMMUNICATION PROTOCOL & STATE LOGIC
-------------------------------------------------------------------------------

The subsystem prevents packet loss and message dropouts via a state-locked acknowledgment structure.

Data Flow Sequence:
1. Detection: The ESP32 reads a HIGH signal from the PIR sensor, locking the state machine with an `alertPending` flag.
2. Transmission: The ESP32 transmits a string packet formatted as `ALERT:[msgCount]`, instantly pivots to listening mode (`LoRa.receive()`), locks operations via `waitingForAck`, and records a timestamp (`ackTimer`).
3. Reception & Validation: The base station checks incoming payloads. If the string starts with `ALERT:`, it processes the packet, displays the connection strength (RSSI), and initiates a confirmation sequence.
4. Handshake Echo: The base station executes a 100 ms propagation delay (allowing the ESP32 to completely clear its TX buffer and stabilize its RX stage), switches to transmission mode, and echoes back `CONFIRM: ALERT:[msgCount]`.
5. Lock Release:
    * Success Path: The ESP32 receives the explicit `CONFIRM:` header matching its state. It increments the `msgCount`, releases `waitingForAck`, and resumes normal sensor polling.
    * Timeout Path: If RF interference or range limits break the link, the ESP32 hits the `TIMEOUT_DURATION` (5000 ms). It breaks the lock without freezing the core loop execution, logs an error to Serial, and forces a reset of the sensor polling sequence to guarantee continuous monitoring.

-------------------------------------------------------------------------------
4. CONSOLE DIAGNOSTIC LOGS
-------------------------------------------------------------------------------

Sender Node (Normal Execution -> Acknowledgment Match):text
Initializing LoRa...
Sender ready!

Sending packet #0
Packet sent — waiting for confirmation...
Confirmation received: CONFIRM: ALERT:0

Sender Node (Timeout Fault Path):text
Sending packet #1
Packet sent — waiting for confirmation...
Error: No confirmation received within timeout window. Retrying sensor loop.

Receiver Node (Continuous Standby & Alert Intercept):text
Initializing LoRa...
Receiver ready!

Heartbeat #0
Heartbeat #1
Received: ALERT:0
RSSI: -74
Valid alert packet — sending confirmation
Confirmation sent

-------------------------------------------------------------------------------
5. SETUP AND DEPLOYMENT
-------------------------------------------------------------------------------

1. Dependencies: Ensure the latest version of the `LoRa` library by Sandeep Mistry is compiled via the Arduino IDE Library Manager.
2. Serial Debugging: Both boards default to a 9600 baud rate. Hardware initialization includes a hardware-independent 2000 ms startup window to allow serial connection capture during prototyping.
3. Antenna Warning: NEVER power on either microcontroller module without an appropriate 915 MHz tuned antenna securely attached to the U.FL / SMA connector. Operating the transceivers without a load will permanently damage the RF frontend output stages.
