
/******************************************
 * mDNS
 ******************************************/
void mdnsSetup() {
  Serial.println("Starting mDNS server....");
  MDNS.begin("j_lamp");
  MDNS.addService("socket", "tcp", 80);
}
