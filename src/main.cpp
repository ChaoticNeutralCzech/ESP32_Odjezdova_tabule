
#include <Arduino.h>
#include "defaultSettings.hpp"
#include "settingsRoutines.hpp"
#include "brightnessRoutines.hpp"
#include "displayRoutines.hpp"
#include "textparser.hpp"
#include "networkRoutines.hpp"


const char* downloadedPage[16384];
char tempStation[10] = "5453613"; //"5453940"; //"5453120"; //temporary: the actual station code will be settable in the browser GUI //5453613
char tempAD[10] = DEP;   //temporary: the actual arrival control will be part of the settings class
String sname;
void setup()
{
  setupPins(); 
  setBrightness(linearize(measureLight()));
  setupDisplay();
  tft.print("\n\n");
  prepareClockFor(C_STATUS, C_SMALL, TFT_BLACK);
  setClockText("Wi-Fi...");
  printClock(false);
  Serial.begin(115200);
  connectToWifi();
  setClockText("Server..."); printClock(false);
  tft.print(smallNums("0123456789")); tft.print(bigNums(" 0123456789:")); 
  //tft.println("\xC2\x80\xC2\x81\xC2\x82\xC2\x83\xC2\x84\xC2\x85\xC2\x86\xC2\x87\xC2\x88\xC2\x89\xC2\x8A\xC2\x90\xC2\x91\xC2\x92\xC2\x93\xC2\x94\xC2\x95\xC2\x96\xC2\x97\xC2\x98\xC2\x99\xC2\x9C\xC2\x9D\xC2\x9E");
  //Serial.println("\xC2\x80\xC2\x81\xC2\x82\xC2\x83\xC2\x84\xC2\x85\xC2\x86\xC2\x87\xC2\x88\xC2\x89\xC2\x8A\xC2\x90\xC2\x91\xC2\x92\xC2\x93\xC2\x94\xC2\x95\xC2\x96\xC2\x97\xC2\x98\xC2\x99\xC2\x9C\xC2\x9D\xC2\x9E");
  //tft.println(UTFUppercase("Příliš žluťoučký kůň úpěl ďábelské ódy.")/*, 16, 40, GFXFF*/);
  //tft.println(UTFUppercase("Vypätá dcéra grófa Maxwella s IQ nižším ako kôň núti \n čeľaď hrýzť hŕbu jabĺk.")/*, 16, 53, GFXFF*/);
  //tft.drawString(UTFUppercase(" čeľaď hrýzť hŕbu jabĺk."), 16, 66, GFXFF);
  //tft.drawString(UTFUppercase("Stróż pchnął kość w quiz gędźb vel fax myjń."), 16, 79, GFXFF);
  //tft.drawString(UTFUppercase("Jörg bäckt quasi zwei Haxenfüße vom Wildpony."), 16, 92, GFXFF);
  //tft.drawString(UTFUppercase("Pál fogyó IQ-jú kun exvő, ím dühös a WC bűzért."), 16, 105, GFXFF);

  prepareNTP();
  //Serial.println(ESP.getFreeHeap()); //see how much space we have available in the ~400 KB RAM
  

    //client.setCACert(root_ca);  
  client.setInsecure(); //accept any certificate: saves space in flash but enables MITM (idc)
  client.stop();
  Serial.println("\nStarting connection to server..."); tft.println("Starting connection to server...");
  const char* marqueeTextCharx;
  String devMessage;
  
  if (!client.connect(apiServer, 443))
    { Serial.println("Connection failed!"); tft.println("Connection failed!"); }
  else {
    Serial.println("Sending request..."); tft.println("Sending request...");
    setClockText("Request..."); printClock(false);
    sendRequest(tempStation, tempAD);
    Serial.println("Request sent."); tft.println("Request sent.");
    setClockText("Waiting..."); printClock(false);
    delay(500);   //probably should wait more, lol
    if(client.connected()) {Serial.println("Still connected, receiving data:");}
    int length =0;
    char c = 0;
    /*while (client.connected()) {
      if (client.read() == 'H') break;
      delay(1);
    }
    client.find("\r\n\r\n"); //skip headers Benoit's way
    Serial.print(length);
    Serial.println(" empty characters");
    */
    if (headerSkim()) {tft.print("HTTP error: ");
    tft.println("HTTP " + *httpStatus);}
    /*char receivedData[10000];
    length = 0;
    int receivedLength = 0;
    char prev_c = 0;*/
    
    //setMarqueeText("(MARQUEE TEST)" LOKOMOTIVA);

    /*
    while (client.connected()) {
      c = client.read();
      if (!c) break;
      if (c==255) {break;}
      Serial.print(c);
    }
    */

    
    do {
    DeserializationError err = deserializeJson(doc, client);
      if (err) {
        Serial.print("deserializeJson() failed with code "); Serial.println(err.c_str());
        tft.print("deserializeJson() failed with code "); tft.println(err.c_str());
        prepareClockFor(C_STATUS, C_SMALL, TFT_RED);
        setClockText(err.c_str()); printClock(false);
        client.stop();
        return;
      }
    } while (client.findUntil(",", "]"));

    const char* snameCharx = doc[0]["head"]["value"];
    Serial.print("Stanice ");
    Serial.println(UTFUppercase(snameCharx));
    tft.print("Stanice ");
    tft.println(snameCharx);
    //marqueeTextCharx = doc[0]["design"][0]["text"][0];
    marqueeText = LOKOMOTIVA;
    marqueeText += marqueeTextCharx; 
    
   // marqueeText += '\0';
//  Serial.println(marqueeText);
//  Serial.print(receivedLength);
//  Serial.println(" bytes received.");

    // http.end(); //Free resources
    //JsonObject obj = doc.as<JsonObject>();
    //prepareMarquee();
    serializeJsonPretty(doc, Serial);
    //setMarqueeText(LOKOMOTIVA + *marqueeTextCharx);
    //tft.drawString("Stanice " + UTFUppercase(snameCharx, SHORTEN), 16, 0, GFXFF);
  }
  client.stop();
  setClockText("Time..."); printClock(false);
  gimmeLocalTime();
  setClockText("Msg..."); printClock(false);
  if (!client.connect(msgServer, 443))
    Serial.println("Connection failed!");
  else {
    sendMsgRequest();
    delay(500);
    //headerSkim();
    client.find("\"d\":\"");
    /*while (client.connected())
    { 
      char c = client.read();
      if (c == 255) break;
      Serial.print(c);

      if (!c) delay(1);
    }*/
    devMessage = client.readStringUntil('"');
    Serial.println(devMessage);
  }
  client.stop();
  //prepareNTP;
  setClockText("Syncing..."); printClock(false);
  gimmeLocalTime(3);
  if (clockSync) prepareClockFor(C_TIME);
  else prepareClockFor(C_STATUS, clockSize, TFT_BLACK);
  jsonToTable();
  prepareMarquee();
  setMarqueeText(marqueeText);
}

int framesAfterSecond = 0;
int lastMillis = 999;

void loop()
{
  delay(27); //norm4
  // Serial.println(WiFi.localIP());
  framesAfterSecond++;
  int nowMillis = millis() % 1000;
  if (nowMillis < lastMillis) framesAfterSecond = 0;
  lastMillis = nowMillis; 
  if(framesAfterSecond == 0)
  {
    gimmeLocalTime(1, 0);
    prepareClockFor(C_TIME);
    setClockText(timeStringForClock(true));
    printClock(true);
  }
  if(framesAfterSecond == 3) printClock(false);
  if(framesAfterSecond == 15)
  {
    setClockText(timeStringForClock(false));
    printClock(false);
  }
  if(framesAfterSecond == 16 && timeinfo.tm_sec == 16)
  {
    prepareClockFor(C_STATUS, C_SMALL, TFT_RED);
    setClockText("SYNCING...");
    printClock(true);
  }
    if(framesAfterSecond == 19 && timeinfo.tm_sec == 16)
  {
    prepareClockFor(C_STATUS, C_SMALL, TFT_RED);
    printClock(false);
  }

  //setMarqueeText(timeString, false);
  //tft.drawString(timeString, 16, 162, GFXFF);
  updateRollingSum();
  delay(1);
  updateRollingSum();
  delay(2);
  updateRollingSum();
  delay(3);
  light = updateRollingSum();
  setBrightness(linearize(light));
 //Serial.println(bright);


  //Serial.print(light);
  //Serial.print("\t->\t");
  //Serial.println(bright);
  shiftMarquee();
  //tft.fillRect(0,140,8,14,0x0000);
  //tft.fillRect(312,140,8,14,0x0000);
}
