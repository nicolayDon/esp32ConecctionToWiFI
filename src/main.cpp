#include <Arduino.h>

#include "OLED.h"
#include <WiFi.h>
#include <FS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

Preferences preferences;
OLED display(17, 16);
int r = 0, c = 0;

const char* assid = "espAccessPoint";
const char* asecret = "hello";
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output26State = "off";
String output27State = "off";

// Assign output variables to GPIO pins
const int output26 = 26;
const int output27 = 27;


const char CONFIG_HTML[] PROGMEM = "<!doctype html><html><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1,maximum-scale=1\"><link rel=\"shortcut icon\" href=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAACXBIWXMAAAsTAAALEwEAmpwYAAAAIGNIUk0AAHolAACAgwAA+f8AAIDpAAB1MAAA6mAAADqYAAAXb5JfxUYAAAMuSURBVHja7NdbiJVVFAfw38SopRk2aCWVoVFDCtFlTKywyKILPTSZRUVQ0EMQaJEERXQjIiomLCINujxVQgllBGkYFUG9xNRLWUlDYnea1CEbszm9/D/YHb8zM2cs5qUFH5xvrW+v9d9r/dfa+3Q0Gg2TKYeYZJl0AB1tfj8Lx+MoHImp0f+JXfgZO/HTvwnyWNyCt+K4kWc/hvFHflf63Xgfa9B9MBlYhNtxPQ7FNrwX59vxI4YwgsMxBydgKZbjtPjZhMezblwyB2uzm9/xNBZPIHOL8GCANrABC8ZadCEGsmBtdlRJJ3qwGuuxGZ+gH+/iedyNZclIJV3R78Ue3NQq+KoE/gznFPrjcC++Kuq8Ex8mva+nNAMhY2Xvw8LCTzfejr2v7MBOPBDDKzgi+ukJPBTb5qDvxrSaDUzFPFyZdI9k3bOYW7T9Q9G/WGXq0ij6C2eL8Wn0LxeEakdOxJPx8QMuL2wbol8NM/BUFPdgZdrrW1zWwvkxqXVvdrw8u6+TM/Bx/K/CDfG/CbPLD58oavxOgjQHvRlvNs2D6hkMF9bUsH16SFp9+wYOq5sDz2Rxb1qwmn534tbwo5oH/fg+DmenTEtxJv7CC2nDHYX/dZiPK9IVtYNpSvF+XsH+l3D2OM6PhSnpCH7DdU32Kf94azQaBzyRG7EPn+OCJiczcTrOj60n/V7Kqdga8Pe3jNUCwLVZuLVpqPTgOXxXw4FfsDFdVWZ0fewP18XqbJHGL9MFC3L6DeCx8GAIr2JLwYGuZGNF+LMxB9j+jOQ96YQDZZQSnBIC7SjaqK8YKnUyE7dlIm4LUYdxUbslKAlVpfuqpmDTcHKAzmyyLcGvWXfJqJsdA4C0VTnF5uERfJ1WGk6W1jVNzCW4eMxs1ylHAdUbslXD5D7clXG9NzW/Y7SABwNgWQJ/hJNq6j83h1kDj6JrPH7buZTuyridkWtYswyGC/vwTUoztrRZgrNCru1NWZiV8dzANf9VCcqjejAg5icjHyT4ynY3NxEAFYjd+KIIvmIi2Z0oADn5qtvS1RMtb8d4/xt2dNTe4M/F0XitFYAx/f7/53SyAfw9APqczAGYp85sAAAAAElFTkSuQmCC\"/><title>Network Settings</title><style>html{font-family:sans-serif}form fieldset{margin:auto;width:80%;border:0}form legend{padding:.3em 0;margin-bottom:.3em;text-align:center;color:#666;border-bottom:1px solid #ccc}form input,form legend{display:block;width:100%}form input{-webkit-appearance:none;padding:.6em;margin:6px 0;font-size:100%;border-radius:3px;outline:0;box-sizing:border-box}form input::-moz-focus-inner{padding:0;border:0}form input:focus:required:invalid{border-color:#e9322d}form input[type=submit]{-webkit-tap-highlight-color:transparent;cursor:pointer;color:#eee;border:0;background-color:#26a69a}form input[type=submit]:active{box-shadow:inset 0 0 12px #777}form input[type=submit][disabled]{opacity:.4;cursor:not-allowed;box-shadow:none}form input[type=password],form input[type=text]{color:#666;border:1px solid #ccc;box-shadow:inset 0 1px 3px #ddd}form input[type=password]:focus,form input[type=text]:focus{border-color:#26a69a}form input[type=password][disabled],form input[type=text][disabled]{cursor:not-allowed;background-color:#eee;color:#aaa}</style></head><body ontouchstart=\"\"><form action=\"wifi\" method=\"get\"><fieldset><legend>Network Settings</legend><input name=\"ssid\" type=\"text\" placeholder=\"SSID\" required> <input name=\"pass\" type=\"password\" placeholder=\"Password\"> <input type=\"submit\" value=\"Connect\"></fieldset></form></body></html>";
AsyncWebServer AsyncServer(80);

bool ConnectUsClient(String _SSID,String _Pass) 
{   
    WiFi.persistent(false);
    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);

    WiFi.mode(WIFI_STA);
    delay(500);
    WiFi.begin((char*)_SSID.c_str(),(char*)_Pass.c_str());
    for(int i=0;i<10;i++)
    if  (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print("wifi");
     
     
    }else{
      
      display.print((char*)WiFi.localIP().toString().c_str(),r++,c++);
      Serial.println(WiFi.localIP());
      return true;
    }
    
  return false;
};
void _initServerCallbacks() {
  //request to change connection point 
  AsyncServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){    
    request->send(200, "text/html", CONFIG_HTML);
  });
  AsyncServer.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request){
  // read 'get' parametr's of request   
    String _SSID=request->arg("ssid");
    String _PASS=request->arg("pass");
  //write login and passwort to flash memory  
    preferences.begin("my-app",false);
    preferences.putString("ssid",_SSID.c_str());
    preferences.putString("pass",_PASS.c_str());
    Serial.println(_SSID+_PASS);
    preferences.end();
  //
    ConnectUsClient(_SSID,_PASS); 
    request->send(200, "text/html", CONFIG_HTML);

  });
         
  AsyncServer.begin();
};
void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
  // Initialize display
    display.begin();
  //read flash memory
    preferences.begin("my-app",false);
    String data_ssid= preferences.getString("ssid","test");
    String data_pass= preferences.getString("pass","test");
    Serial.println(data_ssid+data_pass);
    preferences.end();
  //if wifi connection didn't sucess start acces point 
    if (!ConnectUsClient(data_ssid,data_pass)){
      WiFi.mode(WIFI_AP_STA);
       delay(1000);
      //access point part
      Serial.println("Creating Accesspoint");
      IPAddress myIP =WiFi.softAP(assid,asecret,7,0,5);
      Serial.print("IP address:\t");
      Serial.println(WiFi.softAPIP());
      display.print((char*)WiFi.softAPIP().toString().c_str(),r++,c++);         
    }

    _initServerCallbacks();
  //station part
    Serial.print("connecting to...");
}
void loop() {
  r = r % 8;
  c = micros() % 6;

  if (r == 0)
    display.clear();
 
  delay(500);
  Serial.println("-Loop-");
    // put your main code here, to run repeatedly:
}