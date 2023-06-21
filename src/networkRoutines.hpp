#ifndef Arduino_h
  #include <Arduino.h>
#endif
#include "time.h"           // NTP & time parsing
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "szdc_api_config.h"

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
  WiFi.begin(ssid, password); //connect to Wi-Fi

  while (WiFi.status() != WL_CONNECTED) {delay(500); Serial.print(".");}
  Serial.println(" CONNECTED");
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Chyba konfigurace IP");
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

char httpStatus[32] = {0};
int headerSkim()  //reads and ignores headers unless error response, in which case returns 1; read httpStatus
{
  client.readBytesUntil('\r', httpStatus, sizeof(httpStatus));
  if (strcmp(httpStatus + 9, "200") != 0) {
    Serial.print("Unexpected HTTP response: ");
    Serial.println(httpStatus);
    //client.stop();
    return 1;
  }
  client.find('['); //start of JSON
  return 0;
}

void gimmeLocalTime(int attempts = 3, int wait = 500)
{
  for (int i=0; i<attempts; i++)
  {
    if (getLocalTime(&timeinfo)) break;
    delay(wait);
  }
  strftime(timeString, 10, "%H:%M:%S", &timeinfo);
}

