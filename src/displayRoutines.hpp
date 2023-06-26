#include <Arduino.h>
#include "szdc_api_config.h"
//#ifndef PROJECT_VERSION
//  #define PROJECT_VERSION "Compilation error"
//#endif
#include <TFT_eSPI.h>       // Hardware-specific library
TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

#define GFXFF 1
#include <../lib/TFT_eSPI/Fonts/Custom/Elektrocas_9U.h>
#include <../lib/TFT_eSPI/Fonts/Custom/Pragotron_Infoline_8U.h>
#include <../lib/TFT_eSPI/Fonts/Custom/GeaThinHalf_9.h>
#define ELEKTROCAS9 &Elektrocas_9U
#define ELEKTROCAS8 &Pragotron_Infoline_8U
#define GEASCRIPT9 &ElektrocasGeaThinHalf_9 
#define LOKOMOTIVA "\xC2\x80\xC2\x81\xC2\x82"
#define EC_LOGO "\x9C "
#define IC_LOGO "\x9D "
#define SC_LOGO "\x9E "

String marqueeText = "DEFAULT MARQUEE TEXT";
TFT_eSprite marqueeBitmap = TFT_eSprite(&tft);
TFT_eSprite clockSprite = TFT_eSprite(&tft);

int marqueeSpaceWidth = 304;
int marqueeLength = 0;
int marqueePhase = 0;
int clockX = 254;
int clockY = 23;
uint8_t clockSize = 0;  //0 = small, 1 = large (not implemented), 2 = Tahoma (not implemented)

void setupDisplay()
{
  tft.begin();
  tft.setRotation(1);	// landscape
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  tft.setFreeFont(ELEKTROCAS9);
  tft.setTextColor(TFT_WHITE, STN_BLUE);
}

void printHeader()
{
  tft.setTextColor(STN_BLUE);
  tft.print("\n  github.com/ChaoticNeutralCzech/ESP32_Odjezdova_tabule\n");
  tft.print("                       version "); tft.println(PROJECT_VERSION);
  tft.println(API_COPYRIGHT);
  tft.setTextColor(TFT_LIGHTGREY);
}

void prepareMarquee()
{
  marqueeBitmap.setColorDepth(1);
  marqueeBitmap.createSprite(marqueeSpaceWidth, 13);
  marqueeBitmap.setBitmapColor(lcdBG, lcdFG);
                              //▲C1▲, ▲C0▲
  marqueeBitmap.setFreeFont(ELEKTROCAS8);
  marqueeBitmap.setTextColor(1, 0);
  marqueeBitmap.fillSprite(0);
}
#define C_TIME 0
#define C_STATUS 1
#define C_SMALL 0
#define C_BIG 1
bool clockDisp = C_TIME;



void prepareClockFor(bool disp, bool size = clockSize, uint16_t bgColor = lcdBG) //C_TIME, C_SMALL
{
  clockSprite.setColorDepth(1);
  clockSprite.createSprite(43, 11);
  clockSprite.setTextColor(0, 1, true);

  if (disp) //displaying status
  {
    clockSprite.setFreeFont(ELEKTROCAS9); 
    clockSprite.setBitmapColor(bgColor, lcdFG);
    clockSprite.fillSprite(1);
  } 
  else //displaying time 
  {
    clockSprite.setFreeFont(GEASCRIPT9); 
    clockSprite.setBitmapColor(lcdBG, lcdFG);
                              //▲C1▲, ▲C0▲  
    clockSprite.fillSprite(1);  //clear sprite
  }                            
  

  clockDisp = disp;
  clockSize = size;
}

void setClockText(String clkText)
{
  if (clockDisp) //set to display status
  {
    clockSprite.fillSprite(1); //clear
    int tw = clockSprite.textWidth(clkText);
    int sw = clockSprite.width();
    int leftOffset = (sw-tw)/2;
    if (leftOffset < 0) leftOffset = 0;
    clockSprite.setTextColor(0/*, 1, true*/);
    clockSprite.drawString(clkText, leftOffset, -2, GFXFF);
  } 
  else 
  {
    clockSprite.drawString(clkText, 2, 1, GFXFF);
  }
  
}

void printClock(bool effect = false)
{
  if (effect) clockSprite.pushSprite(clockX, clockY, 0); //prints only blue, white is transparent
  else clockSprite.pushSprite(clockX, clockY);        //prints white and blue
}

void setMarqueeText(String newMarqueeText, bool recalculateLength = true)
{
  if (recalculateLength) marqueeLength=marqueeBitmap.textWidth(newMarqueeText);
  marqueeText = newMarqueeText;
}


void shiftMarquee()
{
  marqueePhase++;
  marqueeBitmap.fillSprite(0);
  marqueeBitmap.setBitmapColor(lcdBG, lcdFG);
  if (marqueePhase == marqueeSpaceWidth + marqueeLength) marqueePhase = 0;
  marqueeBitmap.drawString(marqueeText, marqueeSpaceWidth-marqueePhase, 0);
  marqueeBitmap.pushSprite(8,140);
}

