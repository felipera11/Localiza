#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Sensitive_data.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID SensitiveData::wifi_ssid
#define WIFI_PASSWORD SensitiveData::wifi_password
#define API_KEY SensitiveData::firebase_api_key
#define DATABASE_URL SensitiveData::firebase_database_url
#define scanTime 5

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

BLEScan *pBLEScan;
BLEAdvertisedDevice actualDevice;
BLEScanResults scanResults;

unsigned long time_now;
unsigned long sendDataPrevMillis = 0;

std::string scanner_mac_address;
std::string beacon_address;

void firebase_upload(std::string beacon_address, int beacon_rssi)
{
  std::string path_upload_firebase;

  path_upload_firebase = "localiza/beacons/" + beacon_address + "/" + scanner_mac_address + "/rssi";
  Firebase.RTDB.setInt(&fbdo, path_upload_firebase, beacon_rssi) ? Serial.println("Valor rssi subiu ok no path: " + fbdo.dataPath()) : Serial.println("Deu erro: " + fbdo.errorReason());

  path_upload_firebase = "localiza/beacons/" + beacon_address + "/" + scanner_mac_address + "/time";
  Firebase.RTDB.setInt(&fbdo, path_upload_firebase, time_now) ? Serial.println("Valor time subiu ok no path: " + fbdo.dataPath()) : Serial.println("Deu erro: " + fbdo.errorReason());
}

void wifi_connect()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(300);
  }
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());

  scanner_mac_address = WiFi.macAddress().c_str();

  Serial.print("MAC Address: ");
  Serial.println(scanner_mac_address.c_str());
}

void firebase_connect()
{

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("Firebase connection successful!");
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
  }
  else
  {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
    Serial.println("Firebase connection failed!");
    ESP.restart();
  }
}

void ble_scan()
{
  int count;
  int beacon_rssi;

  Serial.println("Scanning...");
  pBLEScan->start(scanTime, false);
  Serial.println("Scan done!");
  scanResults = pBLEScan->getResults();
  count = scanResults.getCount();
  for (int i = 0; i < count; i++)
  {

    Serial.print("Device ");
    actualDevice = scanResults.getDevice(i);
    beacon_address = actualDevice.getAddress().toString();
    beacon_rssi = actualDevice.getRSSI();
    Serial.println(beacon_address.c_str());

    firebase_upload(beacon_address, beacon_rssi);
  }
  pBLEScan->clearResults();
}

void setup()
{
  Serial.begin(115200);

  wifi_connect();

  firebase_connect();

  timeClient.begin();
  timeClient.setTimeOffset(0);

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
}

void loop()
{
  if (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)
  {
    if (Firebase.ready())
    {
      sendDataPrevMillis = millis();

      timeClient.forceUpdate();
      time_now = timeClient.getEpochTime();
      Serial.print("Time: ");
      Serial.println(time_now);

      ble_scan();

      Serial.println("---------------------------------");
    }
  }
}