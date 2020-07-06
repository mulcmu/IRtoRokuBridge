/*
 * IRtoRokuBridge
 * Allows you to use a universal IR based remote (ex Harmony) to control a Roku stick so you can use a single device
 * Roku Sticks use some sort of RF or bluetooth so can't be controlled by a classic IR only remote
 * This also has direct channel activation which I use via customized buttons in the Watch Roku activity programmed in my Harmony 550
 * This gives you a button on the screen labeled Syfy for instance which directly kicks off that app
 * To customize the apps just obtain the appID by selecting the channel and then from a browser get the app id via
 * http://RokuIP:8060/query/active-app (Replace RokuIP with the IP of your device)
 * 
 * or use http://RokuIP:8060/query/apps to get all installed apps
 * 
 * This uses pieces from a number of programs
 * IRremoteESP8266: https://github.com/markszabo/IRremoteESP8266/
 * Telnet Debug https://github.com/JoaoLopesF/ESP8266-RemoteDebug-Telnet
* To customize
* 1) Replace the IR reciever pin with yours 
* 2) Update the ROKU_BASE_URL with IP address of your Roku
* 3) Update your wifi information in Secret.h file
* 4) Configure Harmony remote and update codes below as needed.  I used a Toshiba MW-24FM1 TV/VCR/DVD combo.  It had enough unique LCD soft button commands 
*    to allow two full screens of app shortcuts.
* */

#include <IRremoteESP8266.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoOTA.h>
#include "RemoteDebug.h" 
#include <IRrecv.h>
#include <IRutils.h>
#include "Secret.h"

#define ROKU_BASE_URL "http://192.168.0.150:8060"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

const long NECRepeat=0xFFFFFFFF;
int RepeatCount=0;
String RokuCmd,RokuCmdLastValid;
long IRCode=0x0;

IRrecv irrecv(14);  //Update IR pin here
HTTPClient http;
decode_results results;
RemoteDebug Debug;
  


void setup()
{

  //Start Serial
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  Serial.println("Booted");

 //Start Debug
  Debug.begin("IRtoRokuBridge");
  
  //Start OTA
  ArduinoOTA.begin();
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  
  //Start IR
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("Waiting for IR");

  // Start Network- Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (irrecv.decode(&results)) {
    IRCode=results.value;
    //This has basic support for NEC repeat code - it can be improved I'm sure
    if (IRCode != NECRepeat){
      returnCmd(IRCode);
    }
    else{
      ++RepeatCount;
      //The filtering of the first two repeats is based on trial and error - may need to adjust based on your remote/click speed
      if (RepeatCount >2){
         RokuCmd=RokuCmdLastValid; }
      else{
         RokuCmd="Ignore";}
    }
    //Process Cmd
    if ( RokuCmd !="Ignore"){
      String url = ROKU_BASE_URL + RokuCmd;
      http.begin(url);
      http.POST("");
      http.end();
      Serial.println(url);
    }
    irrecv.resume(); // Receive the next value
  } 
  ArduinoOTA.handle(); //This does impact IR reading a bit
  Debug.handle(); //This does impact IR reading a bit
  //delay(20);
}

//Set Harmony Remote for Roku to be a Toshiba MW-24FM1 TV/VCR/DVD combo.  Looks like NEC repeats


void returnCmd(long IRCode){
 switch (IRCode) {
    case 0x22DDB24D:
      RokuCmd= "/keypress/Right";
      break;
    case 0x22DD8A75:
       RokuCmd= "/keypress/Left";
       break;
    case 0x22DD01FE:
       RokuCmd= "/keypress/Up";
       break;
    case 0x22DD817E:
       RokuCmd= "/keypress/Down";
       break;
    case 0x22DD728D:  
       RokuCmd= "/keypress/Select";
       break;
    case 0x2FDE817:  //Previous & exit on Harmony remote
       RokuCmd= "/keypress/Back";
       break;
    case 0x2FD708F:   //Menu
       RokuCmd= "/keypress/Home";
       break;
    case 0x22DDA857:
       RokuCmd= "/keypress/Play";  //Play/Pause are the same on roku, harmony play pause and stop all same ir 
       break;
    case 0x22DDC837:
       RokuCmd= "/keypress/Fwd";   
       break;
    case 0x22DD9867:
       RokuCmd= "/keypress/Rev"; 
       break;
    case 0x22DD08F7:
       RokuCmd= "/keypress/InstantReplay";
       break;
    case 0x2FD38C7:
       RokuCmd= "/keypress/Info";  //* ???
       break;
    case 0x2FDD827: 
      RokuCmd= "/keypress/ChannelUp";  
      break;
    case 0x2FDF807:    
      RokuCmd= "/keypress/ChannelDown"; 
      break;
    case 0x22DDAF50:           
      //RokuCmd= "/launch/140474"; //DirectTV Now
      RokuCmd= "/launch/123132";  //Xfinity App
      Serial.println("Xfinity");
      break;
    case 0x22DD8877:                          
      RokuCmd= "/launch/12";  //Netflix 
      Serial.println("netflix");
      break;
    case 0x2FDF00F:         
      RokuCmd= "/launch/13"; //Amazon
      Serial.println("a");
      break;
    case 0x22DDCA35:                        
      RokuCmd= "/launch/291097"; //disney +
      Serial.println("dn");
      break;
    case 0x2FD9867:           
      RokuCmd="/launch/66595"; //nick jr 
      Serial.println("nj");
      break;
    case 0x2FDEA15:           
      RokuCmd="/launch/28"; //Pandora
      Serial.println("pan");
      break;
    case 0x22DD04FB:           
      RokuCmd="/launch/837"; //YouTube
      Serial.println("yt");
      break;   
    case 0xA23DCD32:           
      RokuCmd="/launch/23333"; //PBS Kids
      Serial.println("pbs");
      break;
   case 0x22DDD926:               
      RokuCmd= "/launch/2213"; //Roku Media Player      
      Serial.println("usb");
      break;
    case 0x22DDB04F:           
      RokuCmd="/launch/123095"; //Fish Tank
      Serial.println("fish");
      break;
    case 0x22DDE11E:           
      RokuCmd= "/launch/32828"; //disney now (Mote is still extra3)
      Serial.println("e3");
      break;         
    case 0x22DDFB04:           
      RokuCmd="Ignore"; //Extra4
      Serial.println("e4");
      break; 
    case 0x2FDA857:           
      RokuCmd="Ignore"; //Extra5
      Serial.println("e5");
      break;         
    case 0x22DD3AC5:           
      RokuCmd="Ignore"; //extra6
      Serial.println("e6");
      break; 
    case 0x22DDBA45:           
      RokuCmd="Ignore"; //extra7
      Serial.println("e7");
      break;         
    case 0x22DD6996:           
      RokuCmd="Ignore"; //extra8
      Serial.println("e8");
      break; 
       
    default: 
      Serial.printf("ERROR: No match for code 0x%6X \n", IRCode);
      if (Debug.isActive(Debug.VERBOSE)) {    Debug.printf("IR CODE NOT RECOGNIZED: %8X \n", IRCode);  }
      RokuCmd= "Ignore";
      break;
   }
   //Not detected as a repeat so if it gets here reset repeat info
   RokuCmdLastValid=RokuCmd;
   RepeatCount=0;
  
}
