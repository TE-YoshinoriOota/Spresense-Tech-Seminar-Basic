/*
 *  Spresense_audio_mp3_recorder.ino - Sound recorder example application
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


#include <SDHCI.h>
#include <Audio.h>

SDClass theSD;
AudioClass *theAudio = AudioClass::getInstance();

// static const int32_t recoding_frames = 400;
bool bRecording = false;
bool bStart = false;
int gCounter = 0;
File myFile;

void changeState() {    // interrupt handler
  bStart = bStart ? false : true;
  // bStart = ~bStart;
}

void recorder_end() {
  if (bRecording) {
    bRecording = false;
    theAudio->stopRecorder();
    theAudio->closeOutputFile(myFile);
    myFile.close();
    theAudio->setReadyMode();
    theAudio->end();
  }
  Serial.println("End recording");
}

void recorder_begin() {
  int err;
  theAudio->begin();
  theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC);  
  err = theAudio->initRecorder(AS_CODECTYPE_MP3 ,"/mnt/sd0/BIN" 
                              ,AS_SAMPLINGRATE_48000 ,AS_CHANNEL_MONO);
  if (err != AUDIOLIB_ECODE_OK) {
    Serial.println("Recorder initialize error");
    while(1);
  }
  Serial.println("Recording initialized");
  char filename[16] = {0};
  sprintf(filename, "REC%03d.MP3", gCounter);
  myFile = theSD.open(filename ,FILE_WRITE);
  if (!myFile) {
    Serial.println("File open error\n");
    while(1);
  }
  theAudio->startRecorder();
  bRecording = true;
  ++gCounter;
  Serial.println("Start recording");
}

int intPin = 4;
void setup() {
  Serial.begin(115200);
  pinMode(intPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(intPin) ,changeState ,FALLING);
}

void loop() {
  int err;
  if (!bStart && !bRecording) {
    return;
  } else if (bStart && !bRecording) {
    recorder_begin(); return;
  }else if (!bStart && bRecording) {
    recorder_end(); return;
  }
  
  err = theAudio->readFrames(myFile);
  if (err != AUDIOLIB_ECODE_OK) {
      Serial.println("File End! = " + String(err));
      recorder_end();
  }

  usleep(40000);
  return;
}
