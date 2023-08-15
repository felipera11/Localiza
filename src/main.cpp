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

//defining the wifi credentials and firebase credentials
#define WIFI_SSID SensitiveData::wifi_ssid
#define WIFI_PASSWORD SensitiveData::wifi_password
#define API_KEY SensitiveData::firebase_api_key
#define DATABASE_URL SensitiveData::firebase_database_url
#define scanTime 5

//creating firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

//creating NTP objects for time sync
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

//creating BLE objects for the scan
BLEScan *pBLEScan;
BLEAdvertisedDevice actualDevice;
BLEScanResults scanResults;

//creating auxiliar variables
unsigned long time_now;
std::string scanner_mac_address;
std::string beacon_address;
std::string path_update;

bool update_status = 0;

//function to upload data to firebase
void firebase_upload(std::string beacon_address, int beacon_rssi)
{
  std::string path_upload_firebase;

  //uploading beacon rssi to firebase
  path_upload_firebase = "localiza/beacons/" + beacon_address + "/" + scanner_mac_address + "/rssi";
  Firebase.RTDB.setInt(&fbdo, path_upload_firebase, beacon_rssi) ? Serial.println("Valor rssi subiu ok no path: " + fbdo.dataPath()) : Serial.println("Deu erro: " + fbdo.errorReason());

  //uploading beacon time to firebase
  path_upload_firebase = "localiza/beacons/" + beacon_address + "/" + scanner_mac_address + "/time";
  Firebase.RTDB.setInt(&fbdo, path_upload_firebase, time_now) ? Serial.println("Valor time subiu ok no path: " + fbdo.dataPath()) : Serial.println("Deu erro: " + fbdo.errorReason());
}

//function to connect to wifi
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

//function to connect to firebase
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

//function to scan BLE devices
void ble_scan()
{
  int count;
  int beacon_rssi;

  Serial.println("Scanning...");

  //starting the scan
  pBLEScan->start(scanTime, false);
  Serial.println("Scan done!");

  //getting the results
  scanResults = pBLEScan->getResults();
  count = scanResults.getCount();

  //iterating through the results and uploading to firebase
  for (int i = 0; i < count; i++)
  {

    Serial.print("Device ");
    actualDevice = scanResults.getDevice(i);
    beacon_address = actualDevice.getAddress().toString();
    beacon_rssi = actualDevice.getRSSI();
    Serial.println(beacon_address.c_str());

    //uploading to firebase
    firebase_upload(beacon_address, beacon_rssi);
  }

  //clearing the results to deallocate memory
  pBLEScan->clearResults();
}

//setup function
void setup()
{
  Serial.begin(115200);

  //initializing wifi and firebase
  wifi_connect();
  firebase_connect();

  //initializing time client
  timeClient.begin();
  timeClient.setTimeOffset(0);

  //initializing BLE scan
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);

  path_update = "localiza/update_status/" + scanner_mac_address;
  Serial.println(path_update.c_str());
}

//loop function
void loop()
{
  bool success = Firebase.RTDB.getBool(&fbdo, path_update, &update_status);
  Serial.println(update_status);
  
  //Serial.println("chegoiu aqui");
  //checking if already updated and checking if firebase is ready
  if (!update_status && Firebase.ready())
  {
    Serial.println("Updating...");
    //updating the time
    timeClient.forceUpdate();
    time_now = timeClient.getEpochTime();
    Serial.print("Time: ");
    Serial.println(time_now);

    //scanning BLE devices
    ble_scan();

    //updating the status
    Firebase.RTDB.setBool(&fbdo, path_update, true);

    Serial.println("---------------------------------");
  }
}