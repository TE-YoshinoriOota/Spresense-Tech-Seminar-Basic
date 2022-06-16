#ifdef SUBCORE
#error "Core selection is wrong!!"
#endif

#include <Audio.h>
#include <FFT.h>
#include <MP.h>
#include <MPMutex.h>
#include <DNNRT.h>
#include <SDHCI.h>

#define FFT_LEN 1024

SDClass theSD;
DNNRT dnnrt;
DNNVariable input(FFT_LEN/2); 

MPMutex mutex(MP_MUTEX_ID0);

FFTClass<AS_CHANNEL_MONO, FFT_LEN> FFT;

AudioClass *theAudio = AudioClass::getInstance();

const int subcore = 1;
const float maxSpectrum = 1.5;

const char labels[5][10] = {"no audio", "1000Hz", "300Hz", "3000Hz", "500Hz"};

struct Spectrum {
  void* data;
  int index;
  float value;
};

void setup() {
  Serial.begin(115200);
  while (!theSD.begin() ) { Serial.println("Insert SD card"); };
  FFT.begin(WindowHamming, AS_CHANNEL_MONO, FFT_LEN/2);

  Serial.println("Subcore start");
  MP.begin(subcore);

  Serial.println("Init Audio Recorder");
  theAudio->begin();
  theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC);
  int err = theAudio->initRecorder(AS_CODECTYPE_PCM ,"/mnt/sd0/BIN" 
                           ,AS_SAMPLINGRATE_48000 ,AS_CHANNEL_MONO);                             
  if (err != AUDIOLIB_ECODE_OK) {
    Serial.println("Recorder initialize error");
    while(1);
  }

  Serial.println("Initialize DNNRT");
  File nnbfile("model.nnb");
  if (!nnbfile) {
    Serial.print("nnb not found");
    while(1);
  }

  int ret = dnnrt.begin(nnbfile);
  if (ret < 0) {
    Serial.print("DNN Runtime begin fail: " + String(ret));
    while(1);
  }

  Serial.println("Start Recorder");
  theAudio->startRecorder(); 
  usleep(1);
}


void loop(){
  static const uint32_t buffering_time = FFT_LEN*1000/AS_SAMPLINGRATE_48000;
  static const uint32_t buffer_size = FFT_LEN*sizeof(int16_t);
  static char buff[buffer_size];
  static float pDst[FFT_LEN];
  static uint32_t last_time = 0;
  uint32_t read_size; 
  
  
  int ret = theAudio->readFrames(buff, buffer_size, &read_size);
  if (ret != AUDIOLIB_ECODE_OK && ret != AUDIOLIB_ECODE_INSUFFICIENT_BUFFER_AREA) {
    Serial.println("Error err = " + String(ret));
    theAudio->stopRecorder();
    exit(1);
  }
  
  uint32_t current_time = millis();
  uint32_t duration = current_time - last_time;
  if (read_size < FFT_LEN || duration < buffering_time/2) {
    delay(buffering_time/2);
    return;
  } else {
    last_time = current_time;   
  }

  FFT.put((q15_t*)buff, FFT_LEN);
  FFT.get(pDst, 0);

  float *dnnbuf = input.data();
  for (int i = 0; i < FFT_LEN/2; ++i) {
    pDst[i] /= maxSpectrum;
    dnnbuf[i] = pDst[i];
  }

  dnnrt.inputVariable(input, 0);
  dnnrt.forward(); 
  DNNVariable output = dnnrt.outputVariable(0);
  int index = output.maxIndex();
  Serial.println("Frequency: " + String(labels[index]));
  Serial.println("Probability: " + String(output[index]));

  int8_t sndid;
  float data[FFT_LEN/2];
  if (mutex.Trylock() != 0) return;
  sndid = 100;
  memcpy(data, pDst, sizeof(float)*FFT_LEN/2);
  struct Spectrum sp_data;
  sp_data.data = (void*)data;
  sp_data.index = index;
  sp_data.value = output[index];
  ret = MP.Send(sndid, &sp_data, subcore);
  if (ret < 0) Serial.println("MP.Send Error");
  mutex.Unlock();
}
