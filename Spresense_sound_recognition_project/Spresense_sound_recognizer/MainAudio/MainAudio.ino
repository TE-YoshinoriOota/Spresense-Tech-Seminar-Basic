/*
 *  MainAudio.ino - Sound recognition example
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

#ifdef SUBCORE
#error "Core selection is wrong!!"
#endif

#include <MP.h>
#include <Audio.h>
#include <float.h>


#define SND_AUD 100
#define REQ_FFT 119
#define ACK_FFT  99
#define BEG_REC   9
#define END_REC  19

#include <SDHCI.h>
SDClass theSD;

#define DNNENABLE
#ifdef DNNENABLE
#include <DNNRT.h>
DNNRT dnnrt;
#endif

#define HAVE_LCD
#ifdef HAVE_LCD

#endif

AudioClass *theAudio = AudioClass::getInstance();
static const int32_t buffer_sample = 1024;
static const int32_t buffer_size = buffer_sample * sizeof(int16_t);
static char buff[buffer_size];
uint32_t read_size;

const int subcore = 1;
static int g_loop = 0;

#define MAXLOOP 200

static const char* const labels[5] = {"no audio", "1000Hz", "300Hz", "3000Hz", "500Hz"};
 

void setup()
{
  Serial.begin(115200);
  theSD.begin();

#ifdef HAVE_LCD
  setupLcd();
#endif
 
  Serial.println("Init Audio Recorder");
  theAudio->begin();
  theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC, 200);
  int err = theAudio->initRecorder(AS_CODECTYPE_PCM ,"/mnt/sd0/BIN" 
                           ,AS_SAMPLINGRATE_48000 ,AS_CHANNEL_MONO);                             
  if (err != AUDIOLIB_ECODE_OK) {
    Serial.println("Recorder initialize error");
    while(1);
  }

  MP.begin(subcore);
  MP.RecvTimeout(MP_RECV_POLLING); 

#ifdef DNNENABLE
  File nnbfile("model.nnb");
  if (!nnbfile) {
    Serial.print("nnb not found");
    while(1);
  }

  int ret = dnnrt.begin(nnbfile);
  if (ret < 0) {
    Serial.print("DNN Runtime begin fail: " + String(ret));
    if (ret == -16) {
      Serial.println("Please update bootloader!");
    } else {
      Serial.println(ret);
    }
    while(1);
  }
#endif

  theAudio->startRecorder(); 
  Serial.println("Start Recording");
  task_create("audio recording", 200, 1024, audioReadFrames, NULL);
}

void audioReadFrames() {
  
  while (1) {
    int err = theAudio->readFrames(buff, buffer_size, &read_size);
    if (err != AUDIOLIB_ECODE_OK && err != AUDIOLIB_ECODE_INSUFFICIENT_BUFFER_AREA) {
      Serial.println("Error err = " + String(err));
      theAudio->stopRecorder();
      exit(1);
    }
    
    int8_t sndid = SND_AUD; /* user-defined msgid */
    if ((read_size != 0) && (read_size == buffer_size)) {
      MP.Send(sndid, &buff, subcore);
    } else {
      usleep(1);
    }
  }
  
}

void loop()
{
  static float* data;
  static bool bNG = false;
  int8_t rcvid;
  int ret = MP.Recv(&rcvid, &data, subcore);
  if (ret < 0) {
    return;
  }

#ifdef DNNENABLE
  DNNVariable input(buffer_sample/2);
  float *dnnbuf = input.data();
  if (rcvid == ACK_FFT) {
    // Serial.println("FFT Receive Success!");
    for (int i = 0; i < buffer_sample/2; ++i) {
      dnnbuf[i] = data[i];
    }
  }
#endif

#ifdef DNNENABLE
  dnnrt.inputVariable(input, 0);
  dnnrt.forward();
  DNNVariable output = dnnrt.outputVariable(0);

  int index = output.maxIndex();
  float value = output[index];
  //Serial.println("index: " + String(index));
  //Serial.println("Frequency: " + String(labels[index]));
  //Serial.println("Realiability: " + String(value));
#endif

#ifdef HAVE_LCD
  showSpectrum(data, index, value);
#endif

}
