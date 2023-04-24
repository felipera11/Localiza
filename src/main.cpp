
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Sensitive_data.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID SensitiveData::wifi_ssid
#define WIFI_PASSWORD SensitiveData::wifi_password

// Insert Firebase project API Key
#define API_KEY SensitiveData::firebase_api_key

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL SensitiveData::firebase_database_url

int scanTime = 5; //In seconds

BLEScan* pBLEScan;
BLEAdvertisedDevice actualDevice;
BLEScanResults scanResults;

FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;


void setup() {
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

  // Assign the api key (required) 
  config.api_key = API_KEY;

  // Assign the RTDB URL (required) 
  config.database_url = DATABASE_URL;

  // Sign up 
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  // Assign the callback function for the long running token generation task 
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);


  Serial.println("Initiating BLE");
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value

  
}

void loop() { 

  pBLEScan->start(scanTime, false);
  Serial.println("Scan done!");

  scanResults = pBLEScan->getResults();
  int count_devices = scanResults.getCount();

  for(int i = 0;i<count_devices;i++){
    Serial.print("Device n");
    Serial.println(i+1);
    actualDevice = scanResults.getDevice(i);
    std::string address_device = "Address: " + actualDevice.getAddress().toString();
    int rssi_device = actualDevice.getRSSI();      
    Serial.println(address_device.c_str());
    Serial.print("RSSI: ");
    Serial.println(rssi_device);
  }

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    // Write an Int number on the database path test/int
    if (Firebase.RTDB.setInt(&fbdo, "test/int", count)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    count++;
    
    // Write an Float number on the database path test/float
    if (Firebase.RTDB.setFloat(&fbdo, "test/float", 0.01 + random(0,100))){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }

  delay(5000);

}






















