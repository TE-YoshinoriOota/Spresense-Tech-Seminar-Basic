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
