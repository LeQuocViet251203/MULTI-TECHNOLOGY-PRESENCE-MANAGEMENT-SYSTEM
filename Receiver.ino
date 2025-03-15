/*
This code is not from first place built as the original one deleted from hardware crash
This code is built based on the document(made by the author)
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <LoRa.h>

WiFiClientSecure client;

#define RF_FREQUENCY                868000000 // Hz
#define BUFFER_SIZE                 256

char rxpacket[BUFFER_SIZE];
bool lora_idle = true;

const char* ssid = "Blurred**";
const char* password = "**********";
const char* host = "api.thingspeak.com";
const int httpPort = 443;  
const String writeApiKey = "*******";

int deviceCount = 0;

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  LoRa.setPins(18, 14, 26);
  if (!LoRa.begin(RF_FREQUENCY)) {
    Serial.println("LoRa init failed");
    while (1);
  }

  LoRa.onReceive(OnRxDone);
  LoRa.receive();
}

void loop() {
  if (lora_idle) {
    lora_idle = false;
    LoRa.receive();
  }
}

void OnRxDone(int packetSize) {
  if (packetSize == 0) return;

  int16_t rssi = LoRa.packetRssi();
  int rxSize = 0;

  while (LoRa.available()) {
    rxpacket[rxSize++] = (char)LoRa.read();
  }
  rxpacket[rxSize] = '\0';

  Serial.printf("\r\nReceived packet \"%s\" with rssi %d, length %d\r\n", rxpacket, rssi, rxSize);

  Serial.print("Raw received data in hex: ");
  for (int i = 0; i < rxSize; i++) {
    Serial.printf("%02X ", (uint8_t)rxpacket[i]);
  }
  Serial.println();

  String receivedData = String(rxpacket);
  Serial.println("Received Data:\n" + receivedData);

  if (receivedData.indexOf("Device name:") != -1) {
    String deviceName = receivedData.substring(receivedData.indexOf("Device name:") + 12);
    deviceName = deviceName.substring(0, deviceName.indexOf('\n'));
    Serial.println("Extracted Device Name: " + deviceName);
  }

  if (receivedData.indexOf("Device address:") != -1) {
    String deviceAddress = receivedData.substring(receivedData.indexOf("Device address:") + 15);
    deviceAddress = deviceAddress.substring(0, deviceAddress.indexOf('\n'));
    Serial.println("Extracted Device Address: " + deviceAddress);
  }

  if (receivedData.indexOf("Found ServiceUUID:") != -1) {
    String serviceUUID = receivedData.substring(receivedData.indexOf("Found ServiceUUID:") + 18);
    serviceUUID = serviceUUID.substring(0, serviceUUID.indexOf('\n'));
    Serial.println("Extracted Service UUID: " + serviceUUID);
  }

  if (receivedData.indexOf("Manufacturer Data:") != -1) {
    String manufacturerData = receivedData.substring(receivedData.indexOf("Manufacturer Data:") + 18);
    manufacturerData = manufacturerData.substring(0, manufacturerData.indexOf('\n'));
    Serial.println("Extracted Manufacturer Data: " + manufacturerData);
  }

  if (WiFi.status() == WL_CONNECTED) {
    client.setInsecure();
    if (client.connect(host, httpPort)) {
      String url = "/update?api_key=" + writeApiKey + "&field1=" + String(deviceCount);
      Serial.print("Requesting URL: ");
      Serial.println(url);

      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Connection: close\r\n\r\n");

      readResponse(&client);
    }
    client.stop();
  }

  lora_idle = true;
}

void readResponse(WiFiClientSecure* client) {
  while (client->connected()) {
    String line = client->readStringUntil('\n');
    if (line == "\r") break;
  }
  String response = client->readString();
  Serial.println("Server response:");
  Serial.println(response);
}
