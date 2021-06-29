/*
 *  Spresense_gnss_simple.ino - Simplified gnss example application
 *  Copyright 2019-2021 Sony Semiconductor Solutions Corporation
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
static const char* const labels[5] = {"no audio", "1000Hz", "300Hz", "3000Hz", "500Hz"};

/* Text position (rotated coordinate) */
#define APP_TITLE "FFT Spectrum Analyzer"

/* Graph area */
#define GRAPH_WIDTH  (200) // Spectrum value
#define GRAPH_HEIGHT (320) // Sample step

/* Graph position */
#define GX (40)
#define GY (0)

void setupLcd() {
  tft.begin();
  tft.setRotation(2);
  tft.fillScreen(ILI9341_BLACK);
}

void putResult(int index, float value) {
  String gStrResult = String(labels[index]) + String(" : ") + String(value);
  tft.setRotation(3);
  tft.fillRect(0, 200, 320, 40, ILI9341_BLACK);
  tft.setCursor(40, 210);
  tft.setTextColor(ILI9341_DARKGREY);
  tft.setTextSize(2);
  tft.println(gStrResult);
  tft.setRotation(2);
}

void showSpectrum(float *data, int index, float value) {
  static uint16_t frameBuf[GRAPH_HEIGHT][GRAPH_WIDTH];
  int val;
  for (int i = 0; i < GRAPH_HEIGHT; ++i) {
    val = data[i]*GRAPH_WIDTH;
    val = (val > GRAPH_WIDTH) ? GRAPH_WIDTH: val;
    for (int j = 0; j < GRAPH_WIDTH; ++j) {
      frameBuf[i][j] = (j > val) ? ILI9341_DARKGREY : ILI9341_WHITE;
    }
  }

  tft.drawRGBBitmap(GX, GY, (uint16_t*)frameBuf, GRAPH_WIDTH, GRAPH_HEIGHT);
  putResult(index, value);
  return;
}
