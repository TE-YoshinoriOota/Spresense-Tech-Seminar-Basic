/*
 *  DisplayToLCD.ino - Sound recognition example 
 *  Copyright 2020 Sony Semiconductor Solutions Corporation
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define _cs  10
#define _dc   9   
#define _rst  8

Adafruit_ILI9341 tft = Adafruit_ILI9341(_cs, _dc, _rst);

/* Text position (rotated coordinate) */
#define TX 35
#define TY 210
/* Graph area */
#define GRAPH_WIDTH  (200) // Spectrum value
#define GRAPH_HEIGHT (320) // Sample step
/* Graph position */
#define GX ((240 - GRAPH_WIDTH) / 2) + 20
#define GY ((320 - GRAPH_HEIGHT) / 2)
/* REC Mark */
#define REC_X (GRAPH_WIDTH-30)
#define REC_Y (GRAPH_HEIGHT-30)
#define REC_W (20)


/* Scale */
int range = 40;

void setupLcd() {
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(TX, TY);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("FFT ");
  tft.setRotation(2);
}

static int preLavelId = -1;
unsigned long showSpectrum(float *pDst, int lavelId, float value) {
  static uint16_t frameBuf[GRAPH_HEIGHT][GRAPH_WIDTH];
  unsigned long calc_start;
  int val;
  int i, j;
  uint16_t colormap[] = {
    ILI9341_MAGENTA,
    ILI9341_BLUE,
    ILI9341_CYAN,
    ILI9341_GREEN,
    ILI9341_YELLOW,
    ILI9341_ORANGE,
    ILI9341_RED,
  };

  calc_start = micros();
  int maxvalue = GRAPH_WIDTH;
  int one_step = maxvalue / 7;
  int index;

  for (i = 0; i < GRAPH_HEIGHT; ++i) {
    int val = pDst[i]*GRAPH_WIDTH;
    val = (val > GRAPH_WIDTH) ? GRAPH_WIDTH: val;
    for (j = 0; j < GRAPH_WIDTH; ++j) {
      index = j / one_step;
      if (index > 6) index = 6;
      frameBuf[i][j] = (j > val) ? ILI9341_BLACK : colormap[index];
    }
  }


  tft.drawRGBBitmap(GX, GY, (uint16_t*)frameBuf, GRAPH_WIDTH, GRAPH_HEIGHT);
  tft.setRotation(3);
  if (lavelId != preLavelId) {
    tft.fillRect(TX+100, TY, TX+200, 240, ILI9341_BLACK);  
    tft.setCursor(TX+100, TY);
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(2);
    tft.println(labels[lavelId]);
  }
  tft.fillRect(TX+200, TY, 320, 240, ILI9341_BLACK);  
  tft.setCursor(TX+200, TY);
  tft.setTextColor(ILI9341_BLUE);
  tft.setTextSize(2);
  tft.println(value);
  tft.setRotation(2);  
  preLavelId = lavelId;
  return micros() - calc_start;
}
