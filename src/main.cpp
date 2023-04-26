
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

// ssid e senha do wifi
#define WIFI_SSID SensitiveData::wifi_ssid
#define WIFI_PASSWORD SensitiveData::wifi_password

//api do projeto do firebase
#define API_KEY SensitiveData::firebase_api_key

//url do realtime database
#define DATABASE_URL SensitiveData::firebase_database_url 

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

unsigned long time_now;
String path_with_time;

std::string scanner_mac_address;

int scanTime = 5; //In seconds
BLEScan* pBLEScan;
BLEAdvertisedDevice actualDevice;
BLEScanResults scanResults;

std::string path_upload_firebase;
std::string beacon_address;

void setup(){
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  scanner_mac_address = WiFi.macAddress().c_str();
  Serial.print("MAC Address: ");
  Serial.println(scanner_mac_address.c_str());

  config.api_key = API_KEY;

  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("Conectado ao Firebase");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  timeClient.begin();
  timeClient.setTimeOffset(-10800);  //configura o fuso horario para -3 horas

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);

}

void loop(){

  timeClient.forceUpdate();    //atualiza o tempo do servidor NTP
  time_now = timeClient.getEpochTime();   //retira só o tempo da string
  Serial.print("Time: ");
  Serial.println(time_now);


  pBLEScan->start(scanTime, false);
  Serial.println("Scan done!");
  scanResults = pBLEScan->getResults();
  int count = scanResults.getCount();
  for(int i = 0;i<count;i++){

    Serial.print("Device n");
    Serial.println(i+1);
    actualDevice = scanResults.getDevice(i);
    beacon_address = actualDevice.getAddress().toString();
    int beacon_rssi = actualDevice.getRSSI();      
    Serial.println(beacon_address.c_str());
    Serial.print("RSSI: ");
    Serial.println(beacon_rssi);

    path_upload_firebase = "beacons/" + beacon_address + "/" + scanner_mac_address + "/rssi";

    if (Firebase.RTDB.setInt(&fbdo, path_upload_firebase, beacon_rssi)){
      Serial.println("PASSED");
      Serial.print("PATH: ");
      Serial.println(fbdo.dataPath());
      Serial.print("TYPE: ");
      Serial.println(fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.print("REASON: ");
      Serial.println(fbdo.errorReason());
    }

    path_upload_firebase = "beacons/" + beacon_address + "/" + scanner_mac_address + "/time";
    if (Firebase.RTDB.setInt(&fbdo, path_upload_firebase, time_now)){
      Serial.println("PASSED");
      Serial.print("PATH: ");
      Serial.println(fbdo.dataPath());
      Serial.print("TYPE: ");
      Serial.println(fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.print("REASON: ");
      Serial.println(fbdo.errorReason());
    }

  }
  pBLEScan->clearResults();
  delay(12000);
}
