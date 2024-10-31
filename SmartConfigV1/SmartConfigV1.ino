#include "WiFi.h"
int dotCount = 0;

#define LED_BUILTIN 2

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  //Init WiFi as Station, start SmartConfig
  WiFi.mode(WIFI_AP_STA);
  WiFi.beginSmartConfig();

  //Wait for SmartConfig packet from mobile
  Serial.println("Waiting for SmartConfig.");
  while (!WiFi.smartConfigDone()) {
    delay(500);
    Serial.print(".");
    rapidBlink();
    ++dotCount;

    if(!WiFi.smartConfigDone() && upCount >= 70){ //dotted line width of half window
      Serial.println("\n");
      dotCount = 0;
    }     
  }
  

  Serial.println("");
  Serial.println("SmartConfig received.");

  //Wait for WiFi to connect to AP
  Serial.println("Waiting for WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi Connected.");
  steadyGlow();

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // put your main code here, to run repeatedly:

}

void rapidBlink(){
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
}


void steadyGlow(){
    digitalWrite(LED_BUILTIN, HIGH);    
}

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
