#define PROJECT_VERSION "0.0"
#include <Arduino.h>
#include "defaultSettings.hpp"
#include "settingsRoutines.hpp"
#include "brightnessRoutines.hpp"
#include "displayRoutines.hpp"
#include "textparser.hpp"
#include "networkRoutines.hpp"
#include "powerRoutines.hpp"

//const char* downloadedPage[16384];
String tempStation = "5453613"; //"5453940"; //"5453120"; //temporary: the actual station code will be settable in the browser GUI //5453613
char tempAD[10] = DEP;   //temporary: the actual arrival control will be part of the settings class
String sname;
void setup()
{
  setupPins(); 
  setBrightness(linearize(measureLight()));
  setupDisplay();
  switchTextMode(true);
  printHeader();
  maybeRestoreDefaultSettings();

  //printHeader();
  prepareClockFor(C_STATUS, C_SMALL, TFT_BLACK);
  setClockText("Wi-Fi...");
  printClock(false);
  Serial.begin(115200);
  
  connectToWifi();
  setClockText("Server..."); printClock(false);
  prepareNTP();
  //Serial.println(ESP.getFreeHeap()); //see how much space we have available in the ~400 KB RAM
  //client.setCACert(root_ca);  //if you want to be safe, as opposed to:
  client.setInsecure(); //accept any certificate: saves space in flash but enables MITM (idc)
  getID("pardubice hl");
  while (wedonthavetable) {wedonthavetable = getTable(); delay(1000);}
  client.stop();
  jsonToTable();
  switchTextMode(false);
  printTable();
  timerAlarmEnable(marqueeShiftTimer);
  //printTableTextMode();
  

  tft.setTextColor(TFT_LIGHTGREY);
  //tft.print("Syncing time...");
  setClockText("Syncing..."); printClock(false);
  gimmeLocalTime(99, 0, false);
  //if (!clockSynced) {prepareClockFor(C_STATUS, clockSize, TFT_BLACK); tft.println("Failed to obtain time");}
  prepareMarquee();
  setMarqueeText(marqueeText);
  //xTaskCreatePinnedToCore(updateTableAsync, "updateTableAsync", 20000, NULL, 0, NULL, 0);
}

//int framesAfterSecond = 0;
//int lastMillis = 999;

void loop()
{
  delay(1);
  if(tableToBeUpdated) displayUpdater();
  if(marqueeToBeShifted) {shiftMarquee(); updateBrightness();} else (delay(1));
  if(millis()-lastUpdate > 60000) updateTableAsync();
//  if(tableJustDownloaded) {jsonToTable(); tableJustDownloaded = false;}
  //tft.fillRect(0,140,8,14,0x0000);
  //tft.fillRect(312,140,8,14,0x0000);
  if(enterSettings)
  {
    Serial.println(millis() - buttonLastPressMillis);
    switchTextMode(true);
    printHeader();
    Serial.println("Get ready for settings..."); 
    tft.println("Settings over Wi-Fi...");
    Serial.println("Creating Wi-Fi network...");
    server.client().stop(); 
    Serial.println("Preparing server...");
    captiveportal();
    Serial.println("Done!") ;
    tft.print("\nSSID:"); tft.setCursor(100, tft.getCursorY()); tft.println( "ESP32_Board");
    tft.print("Password:"); tft.setCursor(100, tft.getCursorY()); tft.println("jedejedevlak");
    tft.print("IP:"); tft.setCursor(100, tft.getCursorY()); tft.println("8.8.8.8");
    while(1) asyncServer();
    delay(50000);
  }
}