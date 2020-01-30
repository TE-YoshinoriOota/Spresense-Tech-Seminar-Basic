/*
 *  Spresense_gnss_simple.ino - Simplified gnss example application
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


#include <GNSS.h>
#define STRING_BUFFER_SIZE  128       /**< %Buffer size */
static SpGnss Gnss;                   /**< SpGnss object */

enum ParamSat {
  eSatGps,            /**< GPS                     World wide coverage  */
  eSatGlonass,        /**< GLONASS                 World wide coverage  */
  eSatGpsSbas,        /**< GPS+SBAS                North America        */
  eSatGpsGlonass,     /**< GPS+Glonass             World wide coverage  */
  eSatGpsQz1c,        /**< GPS+QZSS_L1CA           East Asia & Oceania  */
  eSatGpsGlonassQz1c, /**< GPS+Glonass+QZSS_L1CA   East Asia & Oceania  */
  eSatGpsQz1cQz1S,    /**< GPS+QZSS_L1CA+QZSS_L1S  Japan                */
};

static enum ParamSat satType =  eSatGps;

static void print_pos(SpNavData *pNavData) {
  char StringBuffer[STRING_BUFFER_SIZE];

  /* print time */
  snprintf(StringBuffer, STRING_BUFFER_SIZE, "%04d/%02d/%02d ", pNavData->time.year, pNavData->time.month, pNavData->time.day);
  Serial.print(StringBuffer);

  snprintf(StringBuffer, STRING_BUFFER_SIZE, "%02d:%02d:%02d.%06d, ", pNavData->time.hour, pNavData->time.minute, pNavData->time.sec, pNavData->time.usec);
  Serial.print(StringBuffer);

  /* print satellites count */
  snprintf(StringBuffer, STRING_BUFFER_SIZE, "numSat:%2d, ", pNavData->numSatellites);
  Serial.print(StringBuffer);

  /* print position data */
  if (pNavData->posFixMode == FixInvalid)  {
    Serial.print("No-Fix, ");
  } else {
    Serial.print("Fix, ");
  }
  if (pNavData->posDataExist == 0) {
    Serial.print("No Position");
  } else {
    Serial.print("Lat=");
    Serial.print(pNavData->latitude, 6);
    Serial.print(", Lon=");
    Serial.print(pNavData->longitude, 6);
  }
  Serial.println("");
}

void setup() {
  int error_flag = 0;
  Serial.begin(115200);
  sleep(3);  /* Wait HW initialization done. */

  Gnss.setDebugMode(PrintInfo);

  int result;
  result = Gnss.begin();
  if (result != 0)  {
    Serial.println("Gnss begin error!!"); while(1);
  }
  
  switch (satType) {
  case eSatGps:
    Gnss.select(GPS);
    break;
  case eSatGpsSbas:
    Gnss.select(GPS); Gnss.select(SBAS);
    break;
  case eSatGlonass:
    Gnss.select(GLONASS);
    break;
  case eSatGpsGlonass:
    Gnss.select(GPS); Gnss.select(GLONASS);
    break;
  case eSatGpsQz1c:
    Gnss.select(GPS); Gnss.select(QZ_L1CA);
    break;
  case eSatGpsQz1cQz1S:
    Gnss.select(GPS); Gnss.select(QZ_L1CA); Gnss.select(QZ_L1S);
    break;
  case eSatGpsGlonassQz1c:
  default:
    Gnss.select(GPS); Gnss.select(GLONASS); Gnss.select(QZ_L1CA);
    break;
  }
    
  result = Gnss.start(COLD_START);
  if (result != 0) {
    Serial.println("Gnss start error!!");  while(1);
  }
  Serial.println("Gnss setup OK");
}


void loop() {
  if (!Gnss.waitUpdate(-1))  {
    Serial.println("no update");
    return;
  }
  SpNavData NavData;
  Gnss.getNavData(&NavData);
  print_pos(&NavData);
}
