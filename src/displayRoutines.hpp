#include <Arduino.h>

#include <TFT_eSPI.h>       // Hardware-specific library
TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

#define GFXFF 1
#include <../lib/TFT_eSPI/Fonts/Custom/Elektrocas_9U.h>
#include <../lib/TFT_eSPI/Fonts/Custom/Pragotron_Infoline_8U.h>
#define ELEKTROCAS9 &Elektrocas_9U
#define ELEKTROCAS8 &Pragotron_Infoline_8U
#define LOKOMOTIVA "\xC2\x80\xC2\x81\xC2\x82"

String marqueeText = "DEFAULT MARQUEE TEXT";
TFT_eSprite marqueeBitmap = TFT_eSprite(&tft);
TFT_eSprite clockBitmap = TFT_eSprite(&tft);

int marqueeSpaceWidth = 304;
int marqueeLength = 0;
int marqueePhase = 0;

void setupDisplay()
{
  tft.begin();
  tft.setRotation(1);	// landscape
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  tft.setFreeFont(ELEKTROCAS9);
  tft.setTextColor(TFT_WHITE, STN_BLUE);
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

void setMarqueeText(String newMarqueeText, bool recalculateLength = true)
{
  if (recalculateLength) marqueeLength=marqueeBitmap.textWidth(newMarqueeText);
  marqueeText = newMarqueeText;
}


void shiftMarquee()
{
  marqueePhase++;
  marqueeBitmap.fillSprite(0);
  if (marqueePhase == marqueeSpaceWidth + marqueeLength) marqueePhase = 0;
  marqueeBitmap.drawString(marqueeText, marqueeSpaceWidth-marqueePhase, 0);
  marqueeBitmap.pushSprite(8,140);
}

void prepareClock()
{
  clockBitmap.setColorDepth(1);
  clockBitmap.createSprite(43, 11);
  clockBitmap.setBitmapColor(lcdFG, lcdBG);
                            //▲C1▲, ▲C0▲
  clockBitmap.setFreeFont(ELEKTROCAS8);
  clockBitmap.setTextColor(1, 0);
  clockBitmap.fillSprite(0);
}