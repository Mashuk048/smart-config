
#include "FS.h"
#include "esp_system.h"
#include <esp_wifi.h>
#include <Preferences.h>  // WiFi storage
#include <WiFi.h>
#include <PubSubClient.h>

/*namespace {
    key: value
}*/

#define LED_BUILTIN 2

int WFstatus;
int UpCount = 0;//for void loop counter

const  char* wifiSSID;       // NO MORE hard coded set AP, all SmartConfig
const  char* password;
String PrefSSID, PrefPassword;  // used by preferences storage  
String MAC;
String getSsid;
String getPass;
int32_t rssi;

//MQTT
const char* mqtt_server = "test.mosquitto.org";
WiFiClient espClient;
PubSubClient client(espClient);
//MQTT end
  
  // SSID storage
       Preferences preferences;  // declare class object
  // END SSID storage


void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.printf("\tWiFi Setup -- \n"  );
  
  wifiInit();
  IP_info();
  MAC = getMacAddress();

   //For MQTT
   client.setServer(mqtt_server, 1883);
   client.setCallback(callback);
   
   delay(2000);
}//  END setup()





void loop() {
  
  if ( WiFi.status() ==  WL_CONNECTED ){      // Main connected loop   
    //Write what to do if WiFi connected
        
     //For Mqtt reconnect
     if (!client.connected()) {
      reconnect();
     }
     client.loop();    
  }

  
  else{// WiFi DOWN! start LED flasher here

    Serial.println("WIFI DOWN LOOP");
    
    WFstatus = getWifiStatus( WFstatus );

     WiFi.begin( PrefSSID.c_str() , PrefPassword.c_str() );
     int WLcount = 0; //attempt to reconnect
     while (WiFi.status() != WL_CONNECTED && WLcount < 200 )
     {
      delay( 100 );
         Serial.printf(".");
    
         if (UpCount >= 60)  // keep from scrolling sideways forever
         {
            UpCount = 0;
               Serial.printf("\n");
         }
         ++UpCount;
         ++WLcount;
     }

    if(  WFstatus == 3 )   //wifi returns
    { 
    // stop LED flasher, wifi going up
    }
     delay( 1000 );
  }  // END WiFi down 

}// END loop()








void wifiInit()  // TRIES FOR A CERTAIN TIMEOUT
{
   Serial.printf("\t WiFi init!\n");
   WiFi.mode(WIFI_AP_STA);   // required to read NVR before WiFi.begin()
   
    //load ssid, pass from RAM
   // load credentials from NVR, a little RTOS code here
   wifi_config_t conf;
   esp_wifi_get_config(WIFI_IF_STA, &conf);  // requires esp_wifi.h. It loads wifi settings to struct comf
   wifiSSID = reinterpret_cast<const char*>(conf.sta.ssid);
   password = reinterpret_cast<const char*>(conf.sta.password);

      Serial.printf( "conf: %s\n", wifiSSID );
      Serial.printf( "conf: %s\n", password );

   // Open Preferences with "wifi" namespace. Namespace is limited to 15 chars
   preferences.begin("wifi", false);
       PrefSSID      =  preferences.getString("ssid", "none");      //NVS key ssid, default is none
       PrefPassword  =  preferences.getString("password", "none");  //NVS key password, default is none

      Serial.println("pref: "+PrefSSID);
      Serial.println("pref: "+PrefPassword);
   preferences.end();





   // keep from rewriting flash if not needed
   if( !checkIfSame() )      // If different then enter
   {              // not the same, setup with SmartConfig

    Serial.println("NVS and Prefs data different!");
    
    preferences.begin("wifi", false);
       preferences.putString("ssid", wifiSSID);      //copy from conf to preference
       preferences.putString("password", password);  //copy from conf to preference
   preferences.end();

   delay(1000);
   
   preferences.begin("wifi", false);
       PrefSSID = preferences.getString("ssid", "none");      //NVS key ssid, default is none
       PrefPassword = preferences.getString("password", "none");  //NVS key password, default is none
      
      Serial.printf( "conf: %s\n", wifiSSID );//NOTICE THAT BOTH STORAGE DATA ARE NOW SAME
      Serial.printf( "conf: %s\n", password );
      Serial.println("pref: "+PrefSSID);
      Serial.println("pref: "+PrefPassword);
   preferences.end();


    
   // I flash LEDs while connecting here
   Serial.println("NVS and Prefs data same, could be string \"none\" or empty string !");
   
      if( PrefSSID == "none" )  // Setup wifi as New
      {
        initSmartConfig(); // once got the IP, wait for 3 sec and restart
        delay( 3000);
        ESP.restart();   // reboot with configured wifi by smartConfig
      }
   }// END OF CHECK IF SAME

   
   
    if( PrefSSID == "" )  // Setup wifi as New
    {
      initSmartConfig(); // once got the IP, wait for 3 sec and restart
      delay( 3000);
//      ESP.restart();   // reboot with configured wifi by smartConfig
    }
    
   
   WiFi.begin( PrefSSID.c_str() , PrefPassword.c_str() );

   int WLcount = 0;
   while (WiFi.status() != WL_CONNECTED && WLcount < 200 ) // can take > 100 loops depending on router settings
   {
     delay( 100 );
        Serial.printf(".");
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
     ++WLcount;
   }
  delay( 3000 );

  //  stop the led flasher here and set it to glow steadily
  steadyGlow();

}  // END wifiInit()




void callback(char* topic, byte* payload, unsigned int length) {
 Serial.print("Message arrived [");
 Serial.print(topic);
 Serial.print("] ");
 for (int i=0;i<length;i++) {
    char receivedChar = (char)payload[i];
    Serial.print(receivedChar);
    if (receivedChar == '0')
    {
      // ESP8266 Huzzah outputs are "reversed"
//      digitalWrite(LED_BUILTIN, HIGH);


      preferences.begin("wifi", false);
//      preferences.clear();
      preferences.remove("ssid");
      preferences.end();

//      WiFi.disconnect(true,true);//doesn't work    
      esp_wifi_restore();
      delay(1000);
      ESP.restart();

      
      Serial.println("Pref SSID and Conf cleared. Reboot Now!");
      
    }
    if (receivedChar == '1')
    {
//       digitalWrite(LED_BUILTIN, LOW);
       ESP.restart();
    }
  }
  Serial.println();
}
 
 
void reconnect() {
 // Loop until we're reconnected
 while (!client.connected()) { 
   Serial.print("Attempting MQTT connection...");
   
   // Attempt to connect
   if (client.connect("ESP32 Client")) {
    Serial.println("connected");
    // ... and subscribe to topic
    client.subscribe("esp32/command");
    Serial.println("Subscribed to esp32/command");    
   }
   else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
    // Wait 5 seconds before retrying
    delay(5000);
    }
 }
}







// match WiFi IDs in NVS to Pref store,  assumes WiFi.mode(WIFI_AP_STA);  was executed
bool checkIfSame()   
{
    bool val = false;
    String NVssid, NVpass, prefssid, prefpass;

    NVssid = getSsidPass( "ssid" ); //get ssid from real-time os wifi config
    NVpass = getSsidPass( "pass" ); //get pass from real-time os wifi config

    // Open Preferences with my-app namespace. Namespace name is limited to 15 chars
    preferences.begin("wifi", false);
        prefssid  =  preferences.getString("ssid", "none");     //NVS key ssid
        prefpass  =  preferences.getString("password", "none"); //NVS key password
    preferences.end();

    if( NVssid.equals(prefssid) && NVpass.equals(prefpass) )
      { val = true; }

  return val;
}



int getWifiStatus( int WiFiStatus  )
{
  WiFiStatus = WiFi.status();
  Serial.printf("\tStatus %d",  WiFiStatus );
  switch( WiFiStatus )
  {
    case WL_IDLE_STATUS :                         // WL_IDLE_STATUS     = 0,
          Serial.printf(", WiFi IDLE \n");
          break;
    case WL_NO_SSID_AVAIL:                        // WL_NO_SSID_AVAIL   = 1,
          Serial.printf(", NO SSID AVAIL \n");
          break;
    case WL_SCAN_COMPLETED:                       // WL_SCAN_COMPLETED  = 2,
          Serial.printf(", WiFi SCAN_COMPLETED \n");
          break;
    case WL_CONNECTED:                            // WL_CONNECTED       = 3,
          Serial.printf(", WiFi CONNECTED \n");
          break;
    case WL_CONNECT_FAILED:                       // WL_CONNECT_FAILED  = 4,
          Serial.printf(", WiFi WL_CONNECT FAILED\n"); 
          break;
    case WL_CONNECTION_LOST:                      // WL_CONNECTION_LOST = 5,
          Serial.printf(", WiFi CONNECTION LOST\n");
          WiFi.persistent(false);                 // don't write FLASH
          break;
    case WL_DISCONNECTED:                         // WL_DISCONNECTED    = 6
          Serial.printf(", WiFi DISCONNECTED ==\n");
          WiFi.persistent(false);                 // don't write FLASH when reconnecting
          break;
  }
  return WiFiStatus;
}
// END getWifiStatus()


void rapidBlink(){
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
}





void steadyGlow(){
    digitalWrite(LED_BUILTIN, HIGH);    
}




void initSmartConfig(){
  
  int dotCount = 0; //for smart config loop counter
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

    if(!WiFi.smartConfigDone() && dotCount >= 70){ //dotted line width of half window
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
}//END initSmartConfig()



// Requires; #include <esp_wifi.h>
// Returns String NONE, ssid or pass arcording to request 
// ie String var = getSsidPass( "pass" );
String getSsidPass( String s )
{
  String val = "NONE";  // return "NONE" if wrong key sent
  s.toUpperCase();
  if( s.compareTo("SSID") == 0 )
  {
     wifi_config_t conf;
     esp_wifi_get_config( WIFI_IF_STA, &conf );
     val = String( reinterpret_cast<const char*>(conf.sta.ssid) );
  }
  if( s.compareTo("PASS") == 0 )
  {
     wifi_config_t conf;
     esp_wifi_get_config( WIFI_IF_STA, &conf );
     val = String( reinterpret_cast<const char*>(conf.sta.password) );
  }
 return val;
}



// Return RSSI or 0 if target SSID not found
// const char* SSID = "YOUR_SSID";  // declare in GLOBAL space
// call:  int32_t rssi = getRSSI( SSID );
int32_t getRSSI( const char* target_ssid ) 
{
  byte available_networks = WiFi.scanNetworks();

  for (int network = 0; network < available_networks; network++) 
  {
    if ( strcmp(  WiFi.SSID( network).c_str(), target_ssid ) == 0) 
    {
      return WiFi.RSSI( network );
    }
  }
  return 0;
} //  END  getRSSI()


void IP_info()
{
   getSsid = WiFi.SSID();
   getPass = WiFi.psk();
//   rssi = getRSSI(  wifiSSID );
   WFstatus = getWifiStatus( WFstatus );
   MAC = getMacAddress();

      Serial.printf( "\n\n\tSSID\t%s, ", getSsid.c_str() );
      Serial.print( rssi);  Serial.printf(" dBm\n" );  // printf??
      Serial.printf( "\tPass:\t %s\n", getPass.c_str() ); 
      Serial.print( "\n\n\tIP address:\t" );  Serial.print(WiFi.localIP() );
      Serial.print( " / " );
      Serial.println( WiFi.subnetMask() );
      Serial.print( "\tGateway IP:\t" );  Serial.println( WiFi.gatewayIP() );
      Serial.print( "\t1st DNS:\t" );     Serial.println( WiFi.dnsIP() );
      Serial.printf( "\tMAC:\t\t%s\n", MAC.c_str() );
}

// Get the station interface MAC address.
// @return String MAC
String getMacAddress(void)
{
    WiFi.mode(WIFI_AP_STA);                    // required to read NVR before WiFi.begin()
    uint8_t baseMac[6];
    esp_read_mac( baseMac, ESP_MAC_WIFI_STA ); // Get MAC address for WiFi station
    char macStr[18] = { 0 };
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
    return String(macStr);
}
// END getMacAddress()
