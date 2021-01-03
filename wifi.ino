bool shouldSaveConfig = false;

/******************************************
 * WifiManager
 ******************************************/
void wifiSetup() {
  Serial.println("Setting up wifi connection....");
  WiFiManager wifiManager;

  char apSid[20];
  sprintf (apSid, "garage_%08X", ESP.getChipId());
  
  wifiManager.setConfigPortalTimeout(300);
  wifiManager.setDebugOutput(false);
  wifiManager.setAPCallback(wifiConfigModeCallback);
  if (!wifiManager.autoConnect(apSid)) { 
    Serial.println("Failed to connect, trying again...");
    ESP.restart();
  }
}

//call by WifiManager when entering AP mode
void wifiConfigModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("In AP Mode");
  Serial.println(myWiFiManager->getConfigPortalSSID());
}
