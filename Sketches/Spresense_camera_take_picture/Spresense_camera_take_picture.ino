/*
 *  Spresense_camera_take_picture.ino - Simple digital camera application
 *  Copyright 2019 Sony Semiconductor Solutions Corporation
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


#include <Camera.h>
#include <SPI.h>
#include <SDHCI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#define TFT_RST 8
#define TFT_DC  9
#define TFT_CS  10

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS ,TFT_DC ,TFT_RST);

SDClass theSD;
bool bButtonPressed = false;
int gCounter = 0;

void changeState () {
  bButtonPressed = true;
}

void CamCB(CamImage img){
  if (img.isAvailable()) {
    img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);
    tft.drawRGBBitmap(0, 0, (uint16_t *)img.getImgBuff(), 320, 240);
    if (bButtonPressed) {
      tft.setTextSize(2);
      tft.setCursor(100, 200);
      tft.setTextColor(ILI9341_RED);
      tft.println("Shooting");
    }
  }
}

int intPin = 0;
void setup() {
  pinMode(LED0, OUTPUT);
  pinMode(intPin, INPUT_PULLUP);

  theSD.begin();
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(3);
  theCamera.begin();
  theCamera.startStreaming(true, CamCB);

  theCamera.setStillPictureImageFormat(
    CAM_IMGSIZE_QUADVGA_H ,CAM_IMGSIZE_QUADVGA_V
   ,CAM_IMAGE_PIX_FMT_JPG);

  attachInterrupt(digitalPinToInterrupt(intPin) ,changeState ,FALLING);
}

void loop() {
  if (!bButtonPressed) return;
  Serial.println("button pressed");
  digitalWrite(LED0, HIGH);
  theCamera.startStreaming(false, CamCB);
  CamImage img = theCamera.takePicture();
  if (img.isAvailable()) {
      char filename[16] = {0};
      sprintf(filename, "PICT%03d.JPG", gCounter);
      File myFile = theSD.open(filename,FILE_WRITE);
      myFile.write(img.getImgBuff(), img.getImgSize());
      myFile.close();
      ++gCounter;
  }
  bButtonPressed = false;
  theCamera.startStreaming(true, CamCB);
  digitalWrite(LED0, LOW);
}
