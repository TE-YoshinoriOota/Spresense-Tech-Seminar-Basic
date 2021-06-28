/*
 *  Spresense_audio_rec_and_play_button.ino - Sound recorder example application
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

bool bPlayStart = false;
bool bPlaying = false;
bool bRecStart = false;
bool bRecording = false;

static int gCounter = 0;
static char filename[16] = {0};
File myFile;

void changeState() {    // interrupt handler
  bRecStart = ~bRecStart;
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
  digitalWrite(LED0, LOW);
  Serial.println("End recording");
  delay(1000);
  // 
  bPlayStart = true;
}


void recorder_begin() {
  int err;
  theAudio->begin();
  theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC, 100);  // the gain up to 30db
  err = theAudio->initRecorder(AS_CODECTYPE_MP3 ,"/mnt/sd0/BIN" 
                              ,AS_SAMPLINGRATE_48000 ,AS_CHANNEL_MONO);
  if (err != AUDIOLIB_ECODE_OK) {
    Serial.println("Recorder initialize error");
    while(1);
  }
  Serial.println("Recording initialized");
  sprintf(filename, "REC%03d.mp3", gCounter);
  myFile = theSD.open(filename ,FILE_WRITE);
  if (!myFile) {
    Serial.println("File open error\n");
    while(1);
  }
  theAudio->startRecorder();
  bRecording = true;
  ++gCounter;
  digitalWrite(LED0, HIGH);
  Serial.println("Start recording");
}


void player_end() {
  if (bPlaying) 
    theAudio->stopPlayer(AudioClass::Player0);
  bPlaying = false;
  myFile.close();
  Serial.println("End play");
  theAudio->setReadyMode();
  theAudio->end();
  // theSD.remove(filename);
  // Serial.println(String(filename) + " removed");
  bPlayStart = false;
}


void player_begin() {
  int err;
  Serial.println("Start play");
  theAudio->begin();
  theAudio->setRenderingClockMode(AS_CLKMODE_NORMAL);
  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP ,AS_SP_DRV_MODE_LINEOUT);

  err = theAudio->initPlayer(AudioClass::Player0 ,AS_CODECTYPE_MP3 ,"/mnt/sd0/BIN"
                            ,AS_SAMPLINGRATE_48000 ,AS_CHANNEL_MONO);
  if (err != AUDIOLIB_ECODE_OK) {
    Serial.println("Player0 initialize error");
    while(1);
  }
  Serial.println("Player0 initialized");

  Serial.println("Open file: " + String(filename));
  myFile = theSD.open(filename);
  if (!myFile) {
    Serial.println("File open error\n");
    while(1);
  }
  err = theAudio->writeFrames(AudioClass::Player0, myFile);
  if ((err != AUDIOLIB_ECODE_OK) && (err != AUDIOLIB_ECODE_FILEEND)) {
    Serial.println("File Read Error! =" + String(err));
    player_end();
  }
  theAudio->setVolume(-160);
  theAudio->startPlayer(AudioClass::Player0);
  bPlaying = true;
}


int intPin = 0;
void setup() {
  int err;
  Serial.begin(115200);
  theSD.begin();
  pinMode(intPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(intPin) ,changeState ,FALLING);

}

void loop() {
  int err;
  if (bRecStart && !bRecording) {
    recorder_begin();
    usleep(40000);
    return;    
  } else if (!bRecStart && bRecording) {
    recorder_end();
    usleep(40000);
    return;    
  } else if (bRecStart && bRecording) {
    err = theAudio->readFrames(myFile);
    if (err != AUDIOLIB_ECODE_OK) {
        Serial.println("File End! = " + String(err));
        recorder_end();
    }
    usleep(40000);
    return;    
  }
  
  if (bPlayStart && !bPlaying) {
    player_begin();
    usleep(40000);
    return;    
  } else if (!bPlayStart && bPlaying) {
    player_end();
    usleep(40000);
    return;    
  } else if (bPlayStart && bPlaying) {
    err = theAudio->writeFrames(AudioClass::Player0, myFile);
    if (err == AUDIOLIB_ECODE_FILEEND) {
      Serial.println("Audio file ended");
      player_end();
    }
    usleep(40000);
    return;    
  }

  return;
}
