/*
 *  SubFFT.ino - Sound recognition example
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

#ifndef SUBCORE
#error "Core selection is wrong!!"
#endif

#define SND_AUD 100
#define REQ_FFT 119
#define ACK_FFT  99
#define BEG_REC   9
#define END_REC  19

#include <MP.h>
#include <math.h>
#include <float.h>

/* Use CMSIS library */
#define ARM_MATH_CM4
#define __FPU_PRESENT 1U
#include <cmsis/arm_math.h>
#include "RingBuff.h"

/* Select FFT length */
#define FFTLEN 1024
#define SMA_WINDOW 4

/* Ring buffer */
#define INPUT_BUFFER (1024 * 2)
RingBuff ringbuf(INPUT_BUFFER);

/* Allocate the larger heap size than default */
USER_HEAP_SIZE(128 * 1024);

static float pSrc[FFTLEN];
static float pDst[FFTLEN];
static float pSMA[SMA_WINDOW][FFTLEN];

static int g_counter = 0;
static bool bRecFFT = false;

void setup() {
  MP.begin();
  MP.RecvTimeout(MP_RECV_POLLING);
}

void loop() {
  int ret;
  int8_t msgid;
  void *buff;

  ret = MP.Recv(&msgid, &buff);
  if (ret < 0)  return;

  ringbuf.put((q15_t*)buff, 1024);
  
  while (ringbuf.stored() >= FFTLEN) {
    ringbuf.get(pSrc, FFTLEN);
    fft(pSrc, pDst, FFTLEN);
    applySMA(pSMA, pDst); /* Simple Moving Average filter */

    int i;
    float dmax = 0.0;
    for (i = 0; i < FFTLEN / 2; ++i) {
      if (pDst[i] > dmax) dmax = pDst[i];
    }

    if (dmax < 20) dmax = 20;

    for (i = 0; i < FFTLEN; ++i) {
      pDst[i] = pDst[i] / dmax;
    }
    
    int8_t sndid = ACK_FFT;
    ret = MP.Send(sndid, &pDst);
    if (ret < 0) {
      MPLog("MP.Send(pDst) error");
    }    
  }
}

void applySMA(float gSMA[SMA_WINDOW][FFTLEN], float gDst[FFTLEN]) {
  int i, j;
  if (g_counter == SMA_WINDOW) g_counter = 0;
  for (i = 0; i < FFTLEN; ++i) {
    gSMA[g_counter][i] = gDst[i];
    float sum = 0;
    for (j = 0; j < SMA_WINDOW; ++j) {
      sum += gSMA[j][i];
    }
    gDst[i] = sum / SMA_WINDOW;
  }
  ++g_counter;
}

void fft(float *gSrc, float *gDst, int fftLen) {
  float tmpBuf[FFTLEN];
  arm_rfft_fast_instance_f32 S;
  arm_rfft_1024_fast_init_f32(&S);

  arm_rfft_fast_f32(&S, gSrc, tmpBuf, 0);
  arm_cmplx_mag_f32(&tmpBuf[2], &gDst[1], fftLen / 2 - 1);
  gDst[0] = tmpBuf[0];
  gDst[fftLen / 2] = tmpBuf[1]; 
}
