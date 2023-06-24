#include <Arduino.h>
#include "defaultSettings.hpp"
#include "brightnessRoutines.hpp"
#include "displayRoutines.hpp"
#include "networkRoutines.hpp"
#include "settingsRoutines.hpp"
#include "textparser.hpp"
//#include "../lib/ArduinoJson-v6.21.2.hpp"
#include <ArduinoJson.h>

const char* downloadedPage[16384];
char tempStation[10] = "5453613"; //"5453940"; //"5453120"; //temporary: the actual station code will be settable in the browser GUI //5453613
char tempAD[10] = DEP;   //temporary: the actual arrival control will be part of the settings class
String sname;
void setup()
{
  setupPins();
  Serial.begin(115200);
  connectToWifi();
  setupDisplay();
  tft.drawString(UTFUppercase("Příliš žluťoučký kůň úpěl ďábelské ódy."), 16, 40, GFXFF);
  tft.drawString(UTFUppercase("Vypätá dcéra grófa Maxwella s IQ nižším ako kôň núti "), 16, 53, GFXFF);
  tft.drawString(UTFUppercase(" čeľaď hrýzť hŕbu jabĺk."), 16, 66, GFXFF);
  tft.drawString(UTFUppercase("Stróż pchnął kość w quiz gędźb vel fax myjń."), 16, 79, GFXFF);
  tft.drawString(UTFUppercase("Jörg bäckt quasi zwei Haxenfüße vom Wildpony."), 16, 92, GFXFF);
  tft.drawString(UTFUppercase("Pál fogyó IQ-jú kun exvő, ím dühös a WC bűzért."), 16, 105, GFXFF);
  
  ledcSetup(PWMchannel,freq,resolution);
  ledcAttachPin(BL_pin, PWMchannel);
  ledcWrite(PWMchannel, 0);
  prepareNTP();
  Serial.println(ESP.getFreeHeap()); //see how much space we have available in the ~400 KB RAM

  //client.setCACert(root_ca);  
  client.setInsecure(); //accept any certificate: saves space in flash but enables MITM (idc)
  client.stop();
  Serial.println("\nStarting connection to server...");
  
  if (!client.connect(apiServer, 443))
    Serial.println("Connection failed!");
  else {
    Serial.println("Connected to server!");
    sendRequest(tempStation, tempAD);
    Serial.println("Request sent.");
    
    delay(500);   //probably should wait more, lol
    if(client.connected()) {Serial.println("Still connected, receiving data:");}
    int length =0;
    char c = 0;
    /*while (client.connected() && length < 10000) {
      c = client.read();
      if (c == 'H') break;
      if (length == 0) Serial.print("Skipping ");
      length++;
      delay(1);
    }
    Serial.print(length);
    Serial.println(" empty characters");
    */
    //if (headerSkim()) Serial.print ("HTTP error: ");
    //Serial.println(httpStatus);
    /*char receivedData[10000];
    length = 0;
    int receivedLength = 0;
    char prev_c = 0;*/
    client.find("\r\n\r\n"); //skip headers Benoit's way
    //setMarqueeText("(MARQUEE TEST)" LOKOMOTIVA);

   /* while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r")
      {
          Serial.println("headers received");
          break;
      }
    }
    */
    length = 0;
    /*
    while (client.connected()) {
      c = client.read();
      receivedData[length] = c;
      if (!c) break;
      if (c==255) {receivedData[length] = 0; break;}
      Serial.print(c);
    }*/

    DynamicJsonDocument doc(16384);
   // client.read(); //gets rid of leading [
    
  do {
    // Deserialize the next post
    DeserializationError err = deserializeJson(doc, client);
    if (err) {
      Serial.print("deserializeJson() failed with code ");
      Serial.println(err.c_str());
      client.stop();
      return;
    }

    //

    } while (client.findUntil(",", "]"));

    JsonObject table = doc[0];
    const char* snameCharx = table["head"]["value"];
    Serial.print("Stanice ");
    Serial.println(UTFUppercase(snameCharx));
    const char* marqueeTextCharx = table["design"][0]["text"][0];
   // marqueeText += '\0';
//  Serial.println(marqueeText);
//  Serial.print(receivedLength);
//  Serial.println(" bytes received.");
    //gimmeLocalTime();
    //tft.drawString(timeString, 16, 162, GFXFF);

    // http.end(); //Free resources
    JsonObject obj = doc.as<JsonObject>();
    prepareMarquee();
    serializeJsonPretty(doc, Serial);
    //marqueeText = marqueeTextCharx;
    setMarqueeText(LOKOMOTIVA + UTFUppercase(marqueeTextCharx, SHORTEN));
    //sname = snameCharx;  + UTFUppercase(snameCharx))
    tft.drawString("Stanice " + UTFUppercase(snameCharx, SHORTEN), 16, 0, GFXFF);
    //free(tempPrefix); //it was only used for concatenation, now let's save space in heap (?)
  }
  client.stop();
  if (!client.connect(msgServer, 443))
    Serial.println("Connection failed!");
  else {
    sendMsgRequest();
    delay(500);
    //headerSkim();
    client.find("\r\n\r\n");
    while (client.connected())
    { 
      char c = client.read();
      if (c == 255) break;
      Serial.print(c);

      if (!c) delay(1);
    }
  }
  gimmeLocalTime(3);
  //setMarqueeText(timeString);
}

void loop()
{
  delay(27); //norm4
  // Serial.println(WiFi.localIP());
  //gimmeLocalTime(3);
  //setMarqueeText(timeString, false);
  //tft.drawString(timeString, 16, 162, GFXFF);
  updateRollingSum();
  delay(1);
  updateRollingSum();
  delay(2);
  updateRollingSum();
  delay(3);
  light = updateRollingSum();
  int bright = (light*light/3340-27*light/10+6077);
  setBrightness(bright);
  //Serial.print(light);
  //Serial.print("\t->\t");
  //Serial.println(bright);
  shiftMarquee();
  //tft.fillRect(0,140,8,14,0x0000);
  //tft.fillRect(312,140,8,14,0x0000);
}


