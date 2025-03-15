/*
This code is not from first place built as the original one deleted from hardware crash
This code is built based on the document(made by the author)
*/


#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEEddystoneURL.h>
#include <BLEBeacon.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <LoRa.h>

WiFiClientSecure client;

#define RF_FREQUENCY                868000000 // Hz
#define TX_OUTPUT_POWER             5        // dBm
#define LORA_BANDWIDTH              0        // [0: 125 kHz,
#define LORA_SPREADING_FACTOR       7        // [SF7..SF12]
#define LORA_CODINGRATE             1        // [1: 4/5,
#define LORA_PREAMBLE_LENGTH        8        // Same for Tx and Rx
#define LORA_FIX_LENGTH_PAYLOAD_ON  false
#define LORA_IQ_INVERSION_ON        false

#define RX_TIMEOUT_VALUE            1000
#define BUFFER_SIZE                 256     // Increased buffer size for BLE data

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];
double txNumber;

int scanTime = 10; // Scan duration for each BLE scan

BLEScan *pBLEScan;

int deviceCount = 0; // Flag variable to track the number of device count

String txData; // Declare a String variable to store information of detected devices

void setup() {
  Serial.begin(115200);

  // Initialize WiFi
  WiFi.begin("Blurred**", "**********");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // Initialize BLE
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);

  // Initialize LoRa
  LoRa.setPins(18, 14, 26);
  if (!LoRa.begin(RF_FREQUENCY)) {
    Serial.println("LoRa init failed");
    while (1);
  }
  LoRa.setTxPower(TX_OUTPUT_POWER);
  LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);
  LoRa.setCodingRate4(LORA_CODINGRATE + 4);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setPreambleLength(LORA_PREAMBLE_LENGTH);
  LoRa.enableCrc();
}

void loop() {
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  deviceCount = foundDevices.getCount();

  txData = "";
  for (int i = 0; i < deviceCount; i++) {
    BLEAdvertisedDevice device = foundDevices.getDevice(i);
    txData += "Device address: " + device.getAddress().toString().c_str() + "\n";
    if (device.haveName())
      txData += "Device name: " + String(device.getName().c_str()) + "\n";
    if (device.haveManufacturerData())
      txData += "Manufacturer Data: " + device.getManufacturerData().c_str() + "\n";
    if (device.haveServiceUUID())
      txData += "Found ServiceUUID: " + String(device.getServiceUUID().toString().c_str()) + "\n";
  }

  LoRa.beginPacket();
  LoRa.print(txData);
  LoRa.endPacket();

  Serial.println("Sending Data: ");
  Serial.println(txData);

  delay(10000); // Wait before next scan
}

void OnTxDone() {
  Serial.println("LoRa transmission completed.");
  lora_idle = true;
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
  int16_t receivedRssi = rssi;
  int rxSize = size;

  memcpy(rxpacket, payload, size);
  rxpacket[size] = '\0'; 

  String receivedData = String(rxpacket);
  Serial.println("Received Data:\n" + receivedData);

  lora_idle = true; 
}
