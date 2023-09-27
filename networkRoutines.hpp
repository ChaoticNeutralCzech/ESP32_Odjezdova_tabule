#ifndef Arduino_h
  #include <Arduino.h>
#endif
#include "time.h"           // NTP & time parsing
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#ifndef API_COPYRIGHT
  #include "szdc_api_config.h"
#endif
#ifndef SHORTEN
  #include <textparser.hpp>
#endif
#include <ArduinoJson.h>
DynamicJsonDocument doc(16384);
#ifndef MYWIFIPASS
#if __has_include("homeWifi.h") //needed check because homeWiFi.h has confidential info and is gitignore'd
  #include "homeWiFi.h"         //This repo is at someone's home. We will use the creds specified in homeWiFi.h
#else                           //If you just downloaded this, hi! You won't get my home Wi-Fi credentials...
  #include "guestWiFi.h"        //...so use the guest ones instead. Make your own homeWiFi.h file if you need one.
#endif
#endif

IPAddress local_IP(10, 0, 0, 118);

IPAddress gateway(10, 0, 0, 138);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(10, 0, 0, 138);
WiFiClientSecure client;
//bool wedonthavetable = true;
int64_t updated; //UNIX timestamp of when table was updated
unsigned long lastUpdate =0;

void connectToWifi() 
{
  Serial.print("Connecting to "); Serial.print(ssid);
  tft.print("Connecting to "); tft.print(ssid);
  WiFi.begin(ssid, password); //connect to Wi-Fi

  while (WiFi.status() != WL_CONNECTED) {delay(100); Serial.print("."); tft.print(".");}
  Serial.println(" CONNECTED"); tft.println(" CONNECTED");
//  if (!WiFi.config(local_IP, /* gateway, subnet,*/ primaryDNS/*, secondaryDNS*/)) {
//    Serial.println("IP config fail"); tft.println("IP config fail");
//  }
}
struct tm timeinfo;
char timeString[10] = "--:--:--";
void prepareNTP()
{
  const char* ntpServer = "tik.cesnet.cz";
  const long gmtOffset_sec = 3600;
  const int daylightOffset_sec = 3600;
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, "tak.cesnet.cz", "pool.ntp.org");
}

void printLocalTime()   //outputs to serial, only used for debug
{
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time!");
    return;
  }
  Serial.println(&timeinfo, "%d.%m.%Y %H:%M:%S");
}

int getDigits(int num)
{
  int digits = 0;
  do {
    num /= 10; digits++;
  } while (num > 0);
  return digits;
}

void sendRequest(int stationNumber, bool arr)
{
  const char* whichBoard = ADstring[arr];
  client.print(RQ_0); 
  client.print(RQ_HEADERS_LIS); 
  client.print((RQ_BASELENGTH + getDigits(stationNumber) + strlen(whichBoard)));
  client.print("\n\n");
  client.print(RQ_1STATIONIS);
  client.print(stationNumber);
  client.print(RQ_2MODEIS);
  client.print(whichBoard);
  client.print(RQ_3END);
}

void sendMsgRequest()
{
  client.print(RQMSG);
}

char httpStatus[32] = {0};
int headerSkim()  //reads and ignores headers unless error response, in which case returns 1; read httpStatus
{
  while (client.connected()) {
    if (client.read() == 'H') break;
    delay(1);
    Serial.print(".");
  }
  client.find(' ');
  client.readBytesUntil('\r', httpStatus, sizeof(httpStatus));
  if (httpStatus[0] != '2') {
    Serial.print("Unexpected HTTP response: ");
    Serial.println(httpStatus);
    //client.stop();
    return 1;
  }
  client.find("\r\n\r\n"); //start of JSON
  return 0;
}

bool clockSynced = false;
void clockGetsSynced(bool loud)    //call this when clock gets sync'd 
{ 
  clockSynced = true;
  if (loud) {
    tft.print("The time is "); tft.println(&timeinfo, "%d.%m.%Y %H:%M:%S");
    Serial.print("The time is "); Serial.println(&timeinfo, "%d.%m.%Y %H:%M:%S");
  }
  if (settings.clockType) prepareClockFor(C_TIME, settings.clockType-1);      //make clock display time
  if (settings.assumeDST!=timeinfo.tm_isdst) {
    tft.println("DST state cache update (vEEPROM write)!");
    //TODO: write to vEEPROM
  }
  settings.assumeDST = timeinfo.tm_isdst;                        //update cached DST info in RAM
}

void gimmeLocalTime(int attempts = 3, int wait = 500, bool loud = false)
{
  for (int i=0; i<attempts; i++)
  {
    if (getLocalTime(&timeinfo)) {if(!clockSynced) clockGetsSynced(loud); break;}
    delay(wait);
  }
  strftime(timeString, 10, "%H:%M:%S", &timeinfo);
}

char* timeStringForClock(bool colon = true, bool leadingZero = false)
{
  if (clockSize)   //clock is big so it does not show seconds
    timeString[5] = '\0';
  if (!colon) timeString[2] = ';';  //space is semicolon in the font, go figure
  if (timeString[0] == '0' && !leadingZero) timeString[0] = ';';
  return timeString;
}

String devMessage;
#define JSON_TABLE doc[0]["design"][0]["textOrTrain"]

int getTable(bool loud = true)
{
  client.stop();
  if (loud) {Serial.println("\nStarting connection to server..."); tft.println("Starting connection to server...");}
  const char* marqueeTextCharx;
  
  if (!client.connect(apiServer, 443))
    { if (loud) { Serial.println("Connection failed!"); tft.println("Connection failed!"); } return 1;}
  else {
    if (loud) {Serial.println("Sending request..."); tft.println("Sending request...");
      setClockText("Request..."); printClock(false);}
    sendRequest(settings.stationID, settings.arrival);
    if (loud) {Serial.println("Request sent."); tft.println("Request sent.");
    setClockText("Waiting..."); printClock(false);}
    delay(500);   //probably should wait more, lol
    if(client.connected()) {Serial.println("Still connected, receiving data:");}
    int length =0;
    char c = 0;

    if (headerSkim()) {Serial.print("HTTP error "); Serial.println(*httpStatus); return 1;}

    
    do {
    DeserializationError err = deserializeJson(doc, client);
      if (err) {
        tft.print("JSON parser error: "); tft.println(err.c_str());
        prepareClockFor(C_STATUS, C_SMALL, TFT_RED);
        setClockText(err.c_str()); printClock(false);
        client.stop();
        return 1;
      }
    } while (client.findUntil(",", "]"));

    int row = 0;
    while (JSON_TABLE[row] && !stationFlash)
    {
      stationFlash = !JSON_TABLE[row]["time"]["flash"];
      //Serial.print(row); Serial.println(stationFlash); 
      row++;
    };

    const char* snameCharx = doc[0]["head"]["value"];
    Serial.print("Station ");
    Serial.println(UTFUppercase(snameCharx, true));
    //if (loud) {tft.print(ADstringHuman[settings.arrival]); tft.print(" board at "); tft.println(snameCharx);}
    if (doc[0]["design"][0]["text"][0]) //there is a marquee
    {
      marqueeTextCharx = doc[0]["design"][0]["text"][0].as<const char*>();
      marqueeText = LOKOMOTIVA; marqueeText += marqueeTextCharx;
    } 
    //serializeJsonPretty(doc, Serial);
    return 0;
  }
}

void getMsg()
{
  client.stop();
  //setClockText("Msg..."); printClock(false);
  if (!client.connect(msgServer, 443))
    ;//Serial.println("Connection failed!");
  else {
    sendMsgRequest();
    delay(500);
    client.find("\"d\":\"");
    devMessage = client.readStringUntil('"');
    marqueeText = devMessage;
  }
}

#define PLATFORM_UNKNOWN ""


void jsonToTable()
{
  int jsonRow = 0;
  for (int row = 0; row < TABLE_LINES; row++)
  {
    if(!JSON_TABLE[row]) break;   //row does not exist
    table[row].schTimeMin = (JSON_TABLE[jsonRow]["time"]["value"].as<int>())/60000+60+(settings.assumeDST)*60;
    table[row].flashTime = JSON_TABLE[jsonRow]["time"]["flash"].as<bool>();
    if(JSON_TABLE[jsonRow]["time"]["value"]) table[row].schTime = stringifyAbsTime(table[row].schTimeMin); else table[row].schTime = "";
    table[row].delayMin = JSON_TABLE[jsonRow]["delay"].as<int>();
    if(table[row].delayMin)
      table[row].delayString = table[row].delayMin;
    else
      table[row].delayString = "";


    if(JSON_TABLE[jsonRow]["predict"])      //delayed
    {
      table[row].actTimeMin = (JSON_TABLE[jsonRow]["predict"]["value"].as<int>())/60000+60+(settings.assumeDST)*60;
      table[row].actTime = stringifyAbsTime(table[row].actTimeMin);
      if(JSON_TABLE[jsonRow]["predict"]["cancel"])    //cancelled string, null if not
      {
        table[row].actTime = table[row].delayString = JSON_TABLE[jsonRow]["predict"]["cancel"].as<String>();
      }
    } else 
    {
      table[row].actTimeMin = table[row].schTimeMin;
      table[row].actTime = table[row].schTime;
    }
    
    table[row].flashTime = (JSON_TABLE[jsonRow]["time"]["flash"] && stationFlash);

    if (JSON_TABLE[jsonRow]["type"])
      table[row].trainType = fancifyTrainType(JSON_TABLE[jsonRow]["type"].as<const char*>());
    else
      table[row].trainType = "";
    
    table[row].trainSpeed = guessTrainSpeed(table[row].trainType.c_str());


    if (JSON_TABLE[jsonRow]["number"])
      table[row].trainNumber = JSON_TABLE[jsonRow]["number"].as<String>();
    else
      table[row].trainNumber = "";
    table[row].trainNumber = smallNums(table[row].trainNumber.c_str()); //TODO: MAKE THIS OPTIONAL
    
    if(JSON_TABLE[jsonRow]["name"]) //train has name
      table[row].trainName = UTFLowercase(JSON_TABLE[jsonRow]["name"].as<const char*>());
    else 
      table[row].trainName = "";
    
    if(JSON_TABLE[jsonRow]["company"])
      table[row].trainCompany = JSON_TABLE[jsonRow]["company"].as<String>();
    else
      table[row].trainCompany = "";

    if(JSON_TABLE[jsonRow]["line"])
      table[row].trainLine = JSON_TABLE[jsonRow]["line"].as<String>();
    else
      table[row].trainLine = "";
    
    if(JSON_TABLE[jsonRow]["destination"])
      table[row].terminus = UTFUppercase(JSON_TABLE[jsonRow]["destination"].as<const char*>(), true);
      //table[row].terminus = JSON_TABLE[jsonRow]["destination"].as<String>();
    else
      table[row].terminus = "";
    
    if (JSON_TABLE[jsonRow]["direction"]) //could be NULL
    {/*
      table[row].viaNum = JSON_TABLE[jsonRow]["direction"]["station"].size(); //num of via st's/msg pages (those are also ["station"]s)
      if (table[row].viaNum > 6) table[row].viaNum = 6;
      for (int i = 0; i < table[row].viaNum; i++)
      {
        table[row].via[i] = JSON_TABLE[jsonRow]["direction"]["station"][i].as<String>();
      }*/
      
      for(table[row].viaNum = 0; table[row].viaNum < settings.viaMax; table[row].viaNum++)
      {
        if (!JSON_TABLE[jsonRow]["direction"]["station"][table[row].viaNum]) break;
        table[row].via[table[row].viaNum] = JSON_TABLE[jsonRow]["direction"]["station"][table[row].viaNum].as<String>();
      }
      
    }  else
    {
      table[row].viaNum = 0;
    }
    table[row].viaState = 0;

    if (JSON_TABLE[jsonRow]["platform"]) table[row].platform = JSON_TABLE[jsonRow]["platform"].as<String>();
      else table[row].platform = PLATFORM_UNKNOWN;
    table[row].platform.trim();
    
    if (JSON_TABLE[jsonRow]["track"]) {
      table[row].track = JSON_TABLE[jsonRow]["track"].as<String>();
      table[row].track.trim(); //removes leading and trailing whitespace
      if (JSON_TABLE[jsonRow]["sector"])  //most don't specify sector
      {
        String sector = JSON_TABLE[jsonRow]["sector"].as<String>();
        sector.trim();
        table[row].track += " ";
        table[row].track += sector;   //in this implementation, sector is part of line
      }
    } else {
      table[row].track = PLATFORM_UNKNOWN;
    }

    if(JSON_TABLE[jsonRow]["text"] && noteLines)  //there is a note and we are processing notes
    {
      row++;  //advance to next row
      if(row == TABLE_LINES) break;
      table[row].isNote = true;
      table[row].note = JSON_TABLE[jsonRow]["text"].as<String>();
      table[row].trainSpeed = table[row-1].trainSpeed;
      //tft.println(table[row].note);
    } else
    {
      table[row].isNote = false;
      table[row].note = "";
    }
    jsonRow++;
  }
}

void sendSearchQuery(const char* query)
{
  client.print(RQ_SEARCH_0); 
  client.print(RQ_HEADERS_LIS);
  client.print(RQSEARCH_BASELENGTH + strlen(query));
  client.print("\n\n");
  client.print(RQSEARCH_1QUERY);
  client.print(query);
  client.print(RQSEARCH_2END);
}

int getStationNumber(const char* statname)
{
  sendSearchQuery(statname);
  if(headerSkim()) return 0;
  String firstResult = "{";
  client.find('{');
  firstResult += client.readStringUntil('}');
  firstResult += "}";
  Serial.println(firstResult);
  StaticJsonDocument<200> result;
  
  DeserializationError err = deserializeJson(result, firstResult);
  if (err || !result["stationName"]) {
    tft.print("JSON parser error: "); tft.println(err.c_str());
    client.stop();
    return 0;
  }
  settings.stationName = result["stationName"].as<String>();
  //tft.print("Found "); tft.println(settings.stationName);
  
  if(settings.arrival) updated = result["updatedArrival"].as<uint64_t>(); else updated = result["updatedDeparture"].as<uint64_t>();
  if(updated) {
    tft.print(ADstringHuman[settings.arrival]); tft.print(" board at "); tft.println(settings.stationName);
    /*tft.print("Last updated at "); tft.println(updated);*/
  } else {
    tft.print(ADstringHuman[settings.arrival]); tft.print(" board not available for "); tft.println(settings.stationName);}
  return result["sr70"].as<int>();
}

int getID(const char* statname){ //returns ID or 0 if not available
  client.stop();
  //tft.print("Searching for \""); tft.print(statname); tft.println("\"...");
  const char* marqueeTextCharx;
  
  if (!client.connect(apiServer, 443))
    { Serial.println("Connection failed!"); tft.println("Connection failed!"); return 0;}
  else {

    int nowstation;
    nowstation = getStationNumber(statname);
    settings.stationID = nowstation;
    //char * teStation;
    //nowstation.toCharArray(teStation, 10);
    //Serial.print("NewStNum: "); Serial.println(settings.stationID);
    return nowstation;
  }
  
}

void updateClock(int phase)
{
  if(phase == 0)
  {
    gimmeLocalTime(1, 0);
    prepareClockFor(C_TIME);
    setClockText(timeStringForClock(true));
    printClock(true);
  }
  if(phase == 3) printClock(false);
  if(phase == 15)
  {
    setClockText(timeStringForClock(false));
    printClock(false);
  }
}

bool tableJustDownloaded = false;

void updateTableAsync( /*void * pvParameters */)
{
  Serial.println("Async");
  //delay(60000);
  //while(true)
  //{
    delay(10);
    if(millis()-lastUpdate > 60000)
    {
      if(wedonthavetable) drawHeaders();
      wedonthavetable = getTable(false); 
      client.stop(); 
     // Serial.println("D.");
     // delay(2000);
      //Serial.println("DOne.");
      jsonToTable();
      //table[0].flashTime = true;
      //if(table[1].schTimeMin == table[0].schTimeMin) table[1].flashTime=true; 
      Serial.println(ESP.getFreeHeap());
      lastUpdate = millis();
      //tableJustDownloaded = true;
    //}
  }
}

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#define PROJECT_VERSION "0.0"
#include "defaultSettings.hpp"
#include "settingsRoutines.hpp"
#include "brightnessRoutines.hpp"
#include "displayRoutines.hpp"
#include "textparser.hpp"
#include "networkRoutines.hpp"
#include "powerRoutines.hpp"

#ifndef SSID
#define SSID "ESP32_Board"
#endif

const char *softAP_ssid = SSID;

// The access points IP address and net mask
// It uses the default Google DNS IP address 8.8.8.8 to capture all 
// Android dns requests
IPAddress apIP(8, 8, 8, 8);
IPAddress netMsk(255, 255, 255, 0);

// DNS server 
const byte DNS_PORT = 53; 
DNSServer dnsServer;

// Web server
WebServer server(80);

// check if this string is an IP address
boolean isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

// checks if the request is for the controllers IP, if not we redirect automatically to the
// captive portal 
boolean captivePortal() {
  if (!isIp(server.hostHeader())) {
    Serial.println("Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send(302, "text/plain", "");   
    server.client().stop(); 
    return true;
  }
  return false;
}

void handleRoot() {
  if (captivePortal()) { 
    return;
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  String p;
  p += F(
            "<!DOCTYPE html>\r\n<html>\r\n<head> <meta charset=\"UTF-8\">\r\n\t<style>\r\n\t\t:root { --stn: rgb(34, 85, 255); }\r\n\t\t* {font-family: Helvetica, Nimbus Sans, Liberation Sans, sans-serif; color: #fff;}\r\n\t\tbody  {display:grid; margin-top: -40px; place-items:center; background-color: #000}\r\n\t\tp, table {margin-bottom: 20px}\r\n\t\t\r\n\t\ttd, p {background-color: var(--stn); font-family: Bahnschrift, Segoe UI, Noto Sans; padding: 0px 10px}\r\n\t\tp {padding: 2px 10px}\r\n\t\tinput[type=checkbox] {transform: scale(1.5); width: 50%; background: #ff0; margin-left: -21%}\r\n\t\t.s:active {border: 5px green; background-color: yellow}\r\n\t\t.i, input[type=button] {accent-color: var(--stn); background-color: var(--stn); font-size: 12pt; border: none}\r\n\t\t.i {width: 98%; padding: 3px}\r\n\t\tth{padding-left: 10px; padding-top: 10px; text-align: left;}\r\n\t\t#b {position:relative; display:grid; place-items:center; background-color: #111; max-width: 520px; padding: 20px; margin-top: 15vh; border-radius: 8px; box-shadow:inset 0px 0px 0px 8px #aaa;}\r\n\t\t#b:before, #b:after, #b>:first-child:before, #b>:first-child:after {\r\n\t\t\tposition:absolute;\r\n\t\t\tborder:#0006 solid 5px;\r\n\t\t\tcontent:\' \'\r\n\t\t}\r\n\t\t#b:before {top:0;left:0;border-width: 10px 10px 0 0}\r\n\t\t#b:after {top:0;right:0;border-width: 10px 10px 0 0}\r\n\t\t#b>:first-child:before {bottom:0;right:0;border-width: 0 10px 10px 0}\r\n\t\t#b>:first-child:after {bottom:0;left:0;border-width: 0 0 10px 10px}\r\n\t</style>\r\n\r\n</head>\r\n<body>\r\n\t<div id=\"b\">\r\n\t<h2 id=\"je\">JS error</h2>\r\n\t\r\n\r\n\t<p>Congratulations on reaching the ESP32 model train board setup page! On Android, you may need to click \"Keep using this network\". On desktop, please ignore warnings about insecure password input: it is transmitted securely unless the attacker gains access to this setup network or physically connects to the ESP32.</br>Version 0.0</p>\r\n\r\n\t<table id = \"tb\" style=\"width:100%; border-spacing: 0px\">\r\n\t  <tr>\r\n\t\t<th>Key</th>\r\n\t\t<th>&#x202f;Value</th>\r\n\t  </tr>\r\n\r\n\t  \r\n\t</table>\r\n\r\n\t<input type=\"button\" class=\"s\" id=\"sb\" style=\"background-color:#ccc; width: 200px; height: 40px; border-radius: 4px;\" value=\"Settings saved\" disabled=true>\r\n\t\r\n<script>\r\ndocument.getElementById(\"je\").innerText = \"BOARD SETUP\";\r\n//var inputs;\r\n//inputs = document.getElementsByTagName(\'input\');\t\r\n//inputs[0].addEventListener(\'change\', handleChange);\r\nvar t = document.getElementById(\"tb\");\r\nsettingsObj = JSON.parse(\'{\"Wi-Fi 1: SSID\":\"Internet-D8-JON\",\"Wi-Fi 1: password\":\"sNs7sHJKXW\",\"Wi-Fi 2: SSID\":\"FREE WIFI\",\"Wi-Fi 2: password\":\"\",\"Wi-Fi 3: SSID\":\"NETLAB\",\"Wi-Fi 3: password\":\"NETLAB\",\"Config Wi-Fi SSID\":\"ESP32_Board\",\"Config Wi-Fi password\":\"jedejedevlak\",\"Config Wi-Fi on boot\":false,\"Show config Wi-Fi pass​word\":true,\"NTP server\":\"tik.cesnet.cz\",\"Assume DST on boot\":true,\"Station\":null,\"Station name/search query\":\"Uherské Hradiště\",\"Boot to Arrival board\":false,\"Announcement marquee\":true,\"API status marquee\":true,\"Allow note lines\":true,\"Appearance\":null,\"LCD color: R\":34,\"LCD color: G\":85,\"LCD color: B\":255,\"Board X-coord\":16,\"Board Y-coord\":60,\"Board Width\":288,\"Board Count\":3,\"Board Spacing\":7,\"Show Clock\":true,\"Clock X-coord\":254,\"Clock Y-coord\":23,\"Clock leading zero\":false,\"Brightness coefficient A\":300,\"Brightness coefficient B\":-2700,\"Brightness coefficient C\":6077,\"Brightness rolling samples\":8,\"Column positions\":null,\"Train type\":1,\"Train number\":24,\"Train line\":-1,\"Train company\":-1,\"Train name\":-1,\"Destination\":-1,\"Via\":-1,\"Destination - Via\":67,\"Scheduled time\":222,\"Actual time\":-1,\"Time leading zero\":true,\"Track\":254,\"Platform\":-1,\"Delay\":287}\');\r\nObject.entries(settingsObj).forEach(entry => {\r\n\tconst [k, v] = entry;\r\n\tvar r = t.insertRow(-1);\r\n\t\r\n\tif (v === null) \r\n\t{var kc = document.createElement(\"TH\"); r.appendChild(kc); \r\n\t var vc = document.createElement(\"TH\"); r.appendChild(vc);}\r\n\telse {var kc = r.insertCell(0); var vc = r.insertCell(1);}\r\n\tkc.innerText = k;\r\n\tvar p = typeof(v);\r\n\tvar x = \"!\";\r\n\tif (v) {x = \"\"};\r\n\tconsole.log(k, typeof(v));\r\n\tif (p == \"string\") { vc.innerHTML = \'<input class=\"i\" id=\"\' + k + \'\" type=\"text\" value=\"\' + v + \'\">\' };\r\n\tif (p == \"boolean\") { vc.innerHTML = \'<input class=\"i\" id=\"\' + k + \'\" type=\"checkbox\" \' + x + \'checked>\' };\r\n\tif (p == \"number\") { vc.innerHTML = \'<input class=\"i\" id=\"\' + k + \'\" type=\"number\" value=\"\' + v + \'\">\' };\r\n\tif (p == \"object\") { vc.innerHTML = \'<input class=\"i\" id=\"\' + k + \'\" type=\"radio\" style= \"display:none\">\' };\r\n\t\r\n\tvar i = document.getElementById(k);\r\n\tif (k.includes(\"password\")) {i.type = \"password\";}\r\n\tif(i) { i.addEventListener(\'input\', handleChange); i.addEventListener(\'change\', handleChange);}\r\n\t\r\n});\r\nb = document.getElementById(\"sb\");\r\n\r\nfunction handleChange(){\r\n\tb.value = \"Save changes\"; b.style.backgroundColor= \"#25f\"; b.disabled = false; b.style.cursor = \"pointer\";\r\n}\r\n\r\nfunction submitForm() {\r\n\tsb.style.backgroundColor = \"white\";\r\n\tvar jd = {};\r\n\tvar is = document.getElementsByClassName(\"i\");\r\n\tfor ( var j = 0; j<is.length; j++)\r\n\t{\r\n\t\ti = is.item(j);\r\n\t\tk = i.id;\r\n\t\tp = i.type;\r\n\t\tv = i.value;\r\n\t\tif (p == \"checkbox\"){v=(i.checked);}\r\n\t\tif (p == \"radio\") {v=null;}\r\n\t\t\r\n\t\tjd[k] = v;\r\n\t}\r\n\tconsole.log(JSON.stringify(jd));\r\n\tsetTimeout(() => { window.scrollTo(0,0); alert(\"Settings saved.\\nReboot to apply changes.\"); }, 480);\r\n\t\r\n\t\r\n\t/*\r\n\tlocation.reload();*/\r\n\tvar xr = new XMLHttpRequest();\r\n\tvar fd = new FormData(f);\r\n\txr.open(\'POST\',\'http://8.8.8.8/s\');\r\n\txr.setRequestHeader(\"Content-Type\", \"application/json\");\r\n\t\r\n\treturn true;\r\n\t//return false;\r\n}\r\n\r\nsb.onclick = function() {\r\n\tsubmitForm();\r\n}\r\n\r\n</script>\r\n</div>\r\n</body>\r\n</html>");

  server.send(200, "text/html", p);
}

void handleNotFound() {
  if (captivePortal()) { 
    return;
  }
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");

  for (uint8_t i = 0; i < server.args(); i++) {
    message += String(F(" ")) + server.argName(i) + F(": ") + server.arg(i) + F("\n");
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(404, "text/plain", message);
}

void captiveportal() {
  delay(1000);
  Serial.begin(115200);
    setupPins(); 
  setBrightness(linearize(measureLight()));
  setupDisplay();
  switchTextMode(true);
  printHeader();


  Serial.println();
  Serial.println("Configuring access point...");
  WiFi.softAPConfig(apIP, apIP, netMsk);
  // its an open WLAN access point without a password parameter
  WiFi.softAP(softAP_ssid, "jedejedevlak");
  delay(1000);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  /* Setup the web server */
  server.on("/", handleRoot);
  server.on("/generate_204", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin(); // Web server start
  Serial.println("HTTP server started");

}

void asyncServer() {
  
  //DNS
  dnsServer.processNextRequest();
  //HTTP
  server.handleClient();
}