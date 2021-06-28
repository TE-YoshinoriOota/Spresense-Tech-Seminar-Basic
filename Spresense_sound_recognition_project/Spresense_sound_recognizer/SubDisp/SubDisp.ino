#ifndef SUBCORE
#error "Core selection is wrong!!"
#endif

#include <MP.h>
#include <MPMutex.h>
MPMutex mutex(MP_MUTEX_ID0);

struct Spectrum {
  void* data;
  int index;
  float value;
};

void setup() {
  setupLcd();
  MP.begin();
}

void loop() {
  int ret;
  int8_t msgid;
  struct Spectrum* sp_data;

  ret = MP.Recv(&msgid, &sp_data);
  if (ret < 0)  return;

  do {
    ret = mutex.Trylock();
  } while (ret != 0);

  float* buff = (float*)(sp_data->data);
  int index = sp_data->index;
  float value = sp_data->value;
  showSpectrum(buff, index, value);
  mutex.Unlock();
}
