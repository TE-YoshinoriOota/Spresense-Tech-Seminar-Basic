/*
 *  Spresense_audio_mp3_player.ino - Sound player example application
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

bool bPlaying = false;
bool bStart = false;
File myFile;

void changeState() {    // interrupt handler
  bStart = ~bStart;
}

void player_end() {
  if (bPlaying) 
    theAudio->stopPlayer(AudioClass::Player0 ,AS_STOPPLAYER_NORMAL);
  bPlaying = false;
  myFile.close();
  Serial.println("End play");
}

void player_start() {
  Serial.println("Start play");
  myFile = theSD.open("Sound.mp3");
  if (!myFile) {
    Serial.println("File open error\n");
    while(1);
  }
  int err = theAudio->writeFrames(AudioClass::Player0, myFile);
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
  pinMode(intPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(intPin) ,changeState ,FALLING);

  theAudio->begin();
  theAudio->setRenderingClockMode(AS_CLKMODE_NORMAL);
  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP ,AS_SP_DRV_MODE_LINEOUT);

  err = theAudio->initPlayer(AudioClass::Player0 ,AS_CODECTYPE_MP3 ,"/mnt/sd0/BIN"
                            ,AS_SAMPLINGRATE_AUTO ,AS_CHANNEL_STEREO);
  if (err != AUDIOLIB_ECODE_OK) {
    Serial.println("Player0 initialize error");
    while(1);
  }
  Serial.println("Player0 initialized");
}

void loop() {
  int err;
  if (!bStart && !bPlaying) {
    return;
  } else if (bStart && !bPlaying) {
    player_start(); return;
  }else if (!bStart && bPlaying) {
    player_end(); return;
  }
  
  err = theAudio->writeFrames(AudioClass::Player0, myFile);
  if (err == AUDIOLIB_ECODE_FILEEND) {
    Serial.println("Audio file ended");
    player_end();
  }

  usleep(40000);
  return;
}
