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

#if __has_include("homeWifi.h") //needed check because homeWiFi.h has confidential info and is gitignore'd
  #include "homeWiFi.h"         //This repo is at someone's home. We will use the creds specified in homeWiFi.h
#else                           //If you just downloaded this, hi! You won't get my home Wi-Fi credentials...
  #include "guestWiFi.h"        //...so use the guest ones instead. Make your own homeWiFi.h file if you need one.
#endif

IPAddress local_IP(10, 0, 0, 118);
IPAddress gateway(10, 0, 0, 138);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(10, 0, 0, 138);
WiFiClientSecure client;

void connectToWifi() 
{
  Serial.print("Connecting to "); Serial.print(ssid);
  tft.print("Connecting to "); tft.print(ssid);
  WiFi.begin(ssid, password); //connect to Wi-Fi

  while (WiFi.status() != WL_CONNECTED) {delay(100); Serial.print("."); tft.print(".");}
  Serial.println(" CONNECTED"); tft.println(" CONNECTED");
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("IP config fail"); tft.println("IP config fail");
  }
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

void sendRequest(char* stationNumber, char* whichBoard)
{
  client.print(RQ_0HEAD_LENGTHIS); //first part of URL; constant
  client.print((RQ_BASELENGTH + strlen(stationNumber) + strlen(whichBoard)));
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
bool clockSync = false;

void gimmeLocalTime(int attempts = 3, int wait = 500)
{
  for (int i=0; i<attempts; i++)
  {
    if (getLocalTime(&timeinfo)) {clockSync = true; break;}
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

String getMsg()
{
  client.find(":\"");
  return client.readStringUntil('\"');
}
#define PLATFORM_UNKNOWN "-"
#define JSON_TABLE doc[0]["design"][0]["textOrTrain"]
void jsonToTable()
{
  int jsonRow = 0;
  for (int row = 0; row < TABLE_LINES; row++)
  {
    table[row].schTimeMin = (JSON_TABLE[jsonRow]["time"]["value"].as<int>())/60000+60+(timeinfo.tm_isdst)*60;
    table[row].schTime = stringifyAbsTime(table[row].schTimeMin);
    table[row].delayMin = JSON_TABLE[jsonRow]["delay"].as<int>();
    if(table[row].delayMin)
      table[row].delayString = table[row].delayMin;
    else
      table[row].delayString = "";

    if(JSON_TABLE[jsonRow]["predict"])      //delayed
    {
      table[row].actTimeMin = (JSON_TABLE[jsonRow]["predict"]["value"].as<int>())/60000+60+(timeinfo.tm_isdst)*60;
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
    
    if (JSON_TABLE[jsonRow]["type"])
      table[row].trainType = JSON_TABLE[jsonRow]["type"].as<String>();
    else
      table[row].trainType = "";
    
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
    {
      table[row].viaNum = JSON_TABLE[jsonRow]["direction"]["station"].size(); //num of via st's/msg pages (those are also ["station"]s)
      if (table[row].viaNum > 6) table[row].viaNum = 6;
      for (int i = 0; i < table[row].viaNum; i++)
      {
        table[row].via[i] = JSON_TABLE[jsonRow]["direction"]["station"][i].as<String>();
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

    tft.print(fancifyTrainType(table[row].trainType.c_str())); tft.setCursor(23, tft.getCursorY());
    tft.print(table[row].trainNumber); tft.setCursor(60, tft.getCursorY());
    tft.print(table[row].terminus); Serial.print(table[row].terminus);
    if(table[row].viaNum) {tft.print("-"); Serial.println(table[row].via[0]); tft.print(table[row].via[0]);}
    tft.setCursor(233, tft.getCursorY());
    tft.print(bigNums(table[row].schTime.c_str())); tft.setCursor(270, tft.getCursorY());
    tft.print(table[row].platform); tft.setCursor(280, tft.getCursorY());
    tft.print(table[row].track); tft.setCursor(296, tft.getCursorY());
    tft.print(table[row].delayString); tft.print('\n');

    if(JSON_TABLE[jsonRow]["text"] && noteLines)  //there is a note and we are processing notes
    {
      row++;  //advance to next row
      table[row].isNote = true;
      Serial.print(table[row].note = JSON_TABLE[jsonRow]["text"].as<String>());
      tft.println(table[row].note);
    } else
    {
      table[row].isNote = false;
      table[row].note = "";
    }
    jsonRow++;
  }
}