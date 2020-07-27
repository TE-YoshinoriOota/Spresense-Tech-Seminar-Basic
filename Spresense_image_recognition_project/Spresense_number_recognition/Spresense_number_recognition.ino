/*
 *  Spresense_number_recognition.ino - 5 digits recogntion application
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
#include <DNNRT.h>
#include <File.h>

#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#define TFT_RST 8
#define TFT_DC  9
#define TFT_CS  10

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS ,TFT_DC ,TFT_RST);

#define BAUDRATE    115200
#define TILS_SW1         0

/* defines for graphics of box guideline */
#define OFFSET_X        10
#define OFFSET_Y        64
#define BOX_WIDTH       56
#define BOX_STRIDE      60
#define BOX_HEIGHT     112
#define NUM_OF_BOX       5
#define GUIDE_LINE_U    35
#define GUIDE_LINE_L    91
#define DNN_IMG_WIDTH   28
#define DNN_IMG_HEIGHT  28
#define BOX_POS_X(i)   (OFFSET_X + BOX_STRIDE*i)

SDClass  theSD;
DNNRT dnnrt;
DNNVariable input(DNN_IMG_WIDTH*DNN_IMG_HEIGHT);

String gStrResult = "";

void draw_guide_and_text_on_tft () {
  /* draw boxes */ 
  for (int i = 0; i < NUM_OF_BOX; ++i) {
    tft.drawRect(BOX_POS_X(i) ,OFFSET_Y ,BOX_WIDTH ,BOX_HEIGHT ,tft.color565(255, 0, 0));
  }
  /* draw guide lines */
  int guide_line = OFFSET_Y + GUIDE_LINE_U;
  tft.drawLine(0 ,guide_line ,320 ,guide_line ,tft.color565(255, 0, 0));
  guide_line = OFFSET_Y + GUIDE_LINE_L;
  tft.drawLine(0 ,guide_line ,320 ,guide_line ,tft.color565(255, 0, 0));
  /* draw recognition results */
  if (gStrResult != "") {
    tft.setTextSize(3);   
    tft.setCursor(100, 20);
    tft.setTextColor(ILI9341_BLUE);
    tft.println(gStrResult);
  }
}

void CamCB(CamImage img) { 
  if (!img.isAvailable()) return;
  img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);
  tft.drawRGBBitmap(0, 0, (uint16_t *)img.getImgBuff(), 320, 240);
  draw_guide_and_text_on_tft();

  if (digitalRead(TILS_SW1) == HIGH) return;
  
  Serial.println("start clip and resize");
  gStrResult = "";
  int i ,j;
  for (i = 0; i < NUM_OF_BOX; ++i) {
    Serial.println("clip: " + String(i));
    CamImage small;
    CamErr camErr = img.clipAndResizeImageByHW(small
                ,BOX_POS_X(i) ,OFFSET_Y 
                ,BOX_POS_X(i)+BOX_WIDTH-1 ,OFFSET_Y+BOX_HEIGHT-1 /* note that the start pixel included the image */
                ,DNN_IMG_WIDTH ,DNN_IMG_HEIGHT);
    if (!small.isAvailable()) {
      Serial.println("Error Occured at making a target image");
      if (camErr) Serial.println("CamErr: " + String(camErr));
      return;
    }
    uint16_t* buf = (uint16_t*)small.getImgBuff();
    int index = 0;
    uint8_t label[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    float* input_buffer = input.data();
    float f_max = 0.0;
    for (j = 0; j < DNN_IMG_WIDTH * DNN_IMG_HEIGHT; ++j, ++buf) {
      input_buffer[j] = (float)(((*buf & 0x07E0) >> 5) << 2) ; // extract green
      if (input_buffer[j] > f_max) f_max = input_buffer[j];
    }
    /* normalization */
    for (j = 0; j < DNN_IMG_WIDTH * DNN_IMG_HEIGHT; ++j) {
      input_buffer[j] /= f_max;
    }

    Serial.println("DNN forward");
    dnnrt.inputVariable(input, 0);
    dnnrt.forward();
    DNNVariable output = dnnrt.outputVariable(0);
    float max_value = 0.0;
    for (j = 0; output.size() > j; ++j) {
      if (output[j] > max_value) {
        max_value = output[j];
        index = j;
      }
    }
    
    Serial.print("Result : ");
    Serial.println(label[index] + "  (" + String(output[index]) + ")");
    if (label[index] != 10) { 
      gStrResult += String(label[index]);
    } else {
      gStrResult += String(" ");
    }
  }
  Serial.println("Recognition Result: " + gStrResult);
}

void setup() {
  Serial.begin(BAUDRATE);
  pinMode(TILS_SW1,INPUT_PULLUP);
  
  Serial.println("Loading network model");
  File nnbfile = File("model.nnb", FILE_READ);
  if (!nnbfile) {
    Serial.println("nnb not found");
    while(1);
  }
  Serial.println("Initialize DNNRT");
  int ret = dnnrt.begin(nnbfile);
  if (ret < 0) {
    Serial.println("DNNRT initialize error.");
    while(1);
  } 

  tft.begin(); 
  tft.setRotation(3);  
  theCamera.begin();
  theCamera.startStreaming(true, CamCB);
}

void loop() {
  /* do nothing here */  
}
