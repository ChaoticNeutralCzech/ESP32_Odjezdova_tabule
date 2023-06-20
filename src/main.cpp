#include <Arduino.h>
#include "defaultSettings.hpp"
#include "brightnessRoutines.hpp"
#include "displayRoutines.hpp"
#include "networkRoutines.hpp"
//#include "../lib/ArduinoJson-v6.21.2.hpp"
#include <ArduinoJson.h>

const char* downloadedPage[16384];
char tempStation[10] = "5453613"; //temporary: the actual station code will be settable in the browser GUI
char tempAD[10] = DEP;   //temporary: the actual arrival control will be part of the settings class

void setup()
{
  setupPins();
  Serial.begin(115200);
  connectToWifi();

  setupDisplay();
  tft.drawString("Stanice Kamenný Újezd u Českých Budějovic zastávka", 16, 0, GFXFF);
  tft.drawString("Příliš žluťoučký kůň úpěl ďábelské ódy.", 16, 40, GFXFF);
  tft.drawString("Vypätá dcéra grófa Maxwella s IQ nižším ako kôň núti čeľaď hrýzť hŕbu jabĺk.", 16, 53, GFXFF);
  tft.drawString("Stróż pchnął kość w quiz gędźb vel fax myjń.", 16, 66, GFXFF);
  tft.drawString("Jörg bäckt quasi zwei Haxenfüße vom Wildpony.", 16, 79, GFXFF);
  tft.drawString("Pál fogyó IQ-jú kun exvő, ím dühös a WC bűzért.", 16, 92, GFXFF);
  ledcSetup(PWMchannel,freq,resolution);
  ledcAttachPin(BL_pin, PWMchannel);
  ledcWrite(PWMchannel, 0);
  prepareNTP();
  Serial.println(ESP.getFreeHeap()); //see how much space we have available in the ~400 KB RAM

  //client.setCACert(root_ca);  
  client.setInsecure(); //worth trying to save space in flash?
  client.stop();
  Serial.println("\nStarting connection to server...");
  
  if (!client.connect(serverName, 443))
    Serial.println("Connection failed!");
  else {
    Serial.println("Connected to server!");
    sendRequest(tempStation, tempAD);
    Serial.println("Request sent.");
    
    delay(500);   //probably should wait more, lol
    if(client.connected()) {Serial.println("Still connected, receiving data:");}
    int length =0;
    char c = 0;
    while (client.connected() && length < 10000) {
      c = client.read();
      if (c == 'H') break;
      if (length == 0) Serial.print("Skipping ");
      length++;
      delay(1);
    }
    Serial.print(length);
    Serial.println(" empty characters");

    char receivedData[10000];
    length = 0;
    int receivedLength = 0;
    char prev_c = 0;
    setMarqueeText("(MARQUEE TEST)" LOKOMOTIVA);
    bool receivingMarquee = false;
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r")
      {
          Serial.println("headers received");
          break;
      }
    }
    length = 0;
    while (client.connected()) {
      c = client.read();
      receivedData[length] = c;
      if (!c) break;
      if (c==255) {receivedData[length] = 0; break;}
      Serial.print(c);
    }
   /*for (int i=0; i<16384; i++)
    {
      if (!receivedData[i]) break;
      Serial.print(receivedData[i]);
    }*/
    //DynamicJsonDocument doc(16384);
    //DeserializationError err = deserializeJson(doc, *receivedData);


    //deserializ(JsonBuffer, client);

    // err = deserializeJson();

   // marqueeText += '\0';
    Serial.println(marqueeText);
    Serial.print(receivedLength);
    Serial.println(" bytes received.");
    //gimmeLocalTime();
    //tft.drawString(timeString, 16, 162, GFXFF);

    // http.end(); //Free resources
    
    prepareMarquee();

  }
  
  gimmeLocalTime();
  setMarqueeText(timeString);
}

void loop()
{
  delay(27); //norm4
  // Serial.println(WiFi.localIP());
  gimmeLocalTime();
  setMarqueeText(timeString, false);
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


