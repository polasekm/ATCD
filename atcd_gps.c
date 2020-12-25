/*
 * usart.c
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin
 */
//------------------------------------------------------------------------------
#include "atcd_gps.h"
#include "atcd.h"

extern atcd_t atcd;

//------------------------------------------------------------------------------
void atcd_gps_init()
{

}
//------------------------------------------------------------------------------
void atcd_gps_reset()
{

}
//------------------------------------------------------------------------------
void atcd_gps_proc()
{

}
//------------------------------------------------------------------------------    
void atcd_gps_put_nmea(char *str)
{

}
//------------------------------------------------------------------------------
uint8_t atcd_gps_asc_msg()
{
    char *p, *np, *endl;

    if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "$GPRMC,", strlen("$GPRMC,")) == 0 ||
       strncmp(atcd.parser.buff + atcd.parser.line_pos, "$GNRMC,", strlen("$GNRMC,")) == 0 ||
       strncmp(atcd.parser.buff + atcd.parser.line_pos, "$GLRMC,", strlen("$GLRMC,")) == 0)
    {
      ATCD_DBG_GPS_RMC

      p    = atcd.parser.buff + atcd.parser.line_pos + strlen("$GPRMC,");
      endl = atcd.parser.buff + atcd.parser.buff_pos;

      //Time
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc;
      memcpy(atcd.gps.time, p, np - p);
      atcd.gps.time[np - p] = 0;
      p = np + 1;

      //Status
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc;
      p = np + 1;

      //Latitude
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc;
      atcd.gps.latitude = atof(p);
      p = np + 1;

      //NS
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc;
      if(*p == 'S') atcd.gps.latitude = -atcd.gps.latitude;
      p = np + 1;

      //Longitude
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc;
      atcd.gps.longitude = atof(p);
      p = np + 1;

      //EW
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc;
      if(*p == 'E') atcd.gps.longitude = -atcd.gps.longitude;
      p = np + 1;

      //Speed
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc;
      atcd.gps.speed = atof(p);
      p = np + 1;

      //Course
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc;
      atcd.gps.course = atof(p);
      p = np + 1;

      //Date
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc;
      memcpy(atcd.gps.date, p, np - p);
      atcd.gps.date[np - p] = 0;
      p = np + 1;

      //if(atcd.gps.callback != NULL && (atcd.gps.cb_events & ATCD_GSM_EV_REG) != 0) atcd.gps.callback(ATCD_GSM_EV_REG);

      atcd.parser.buff_pos = atcd.parser.line_pos;
      return 1;

    skip_proc:
      ATCD_DBG_GPS_RMC_ERR
      atcd.parser.buff_pos = atcd.parser.line_pos;
      return 1;
    }

    if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "$GPGSA,", strlen("$GPGSA,")) == 0 ||
       strncmp(atcd.parser.buff + atcd.parser.line_pos, "$GNGSA,", strlen("$GNGSA,")) == 0 ||
       strncmp(atcd.parser.buff + atcd.parser.line_pos, "$GLGSA,", strlen("$GLGSA,")) == 0)
    {
      ATCD_DBG_GPS_GSA

      p    = atcd.parser.buff + atcd.parser.line_pos + strlen("$GPGSA,");
      endl = atcd.parser.buff + atcd.parser.buff_pos;

      //if(atcd.gps.callback != NULL && (atcd.gps.cb_events & ATCD_GSM_EV_REG) != 0) atcd.gps.callback(ATCD_GSM_EV_REG);

      atcd.parser.buff_pos = atcd.parser.line_pos;
      return 1;

    skip_proc2:
      ATCD_DBG_GPS_GSA_ERR
      atcd.parser.buff_pos = atcd.parser.line_pos;
      return 1;
    }

    if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "$GPGGA,", strlen("$GPGGA,")) == 0 ||
       strncmp(atcd.parser.buff + atcd.parser.line_pos, "$GNGGA,", strlen("$GNGGA,")) == 0 ||
       strncmp(atcd.parser.buff + atcd.parser.line_pos, "$GLGGA,", strlen("$GLGGA,")) == 0)
    {
      ATCD_DBG_GPS_GGA

      p    = atcd.parser.buff + atcd.parser.line_pos + strlen("$GPGGA,");
      endl = atcd.parser.buff + atcd.parser.buff_pos;

      //if(atcd.gps.callback != NULL && (atcd.gps.cb_events & ATCD_GSM_EV_REG) != 0) atcd.gps.callback(ATCD_GSM_EV_REG);

      atcd.parser.buff_pos = atcd.parser.line_pos;
      return 1;

    skip_proc3:
      ATCD_DBG_GPS_GGA_ERR
      atcd.parser.buff_pos = atcd.parser.line_pos;
      return 1;
    }

    if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "$GPGSV,", strlen("$GPGSV,")) == 0 ||
       strncmp(atcd.parser.buff + atcd.parser.line_pos, "$GNGSV,", strlen("$GNGSV,")) == 0 ||
       strncmp(atcd.parser.buff + atcd.parser.line_pos, "$GLGSV,", strlen("$GLGSV,")) == 0)
    {
      ATCD_DBG_GPS_GSV

      p    = atcd.parser.buff + atcd.parser.line_pos + strlen("$GPGSV,");
      endl = atcd.parser.buff + atcd.parser.buff_pos;

      //if(atcd.gps.callback != NULL && (atcd.gps.cb_events & ATCD_GSM_EV_REG) != 0) atcd.gps.callback(ATCD_GSM_EV_REG);

      atcd.parser.buff_pos = atcd.parser.line_pos;
      return 1;

    skip_proc4:
      ATCD_DBG_GPS_GSV_ERR
      atcd.parser.buff_pos = atcd.parser.line_pos;
      return 1;
    }

    if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "$GPACCURACY,", strlen("$GPACCURACY,")) == 0)
    {
      ATCD_DBG_GPS_ACC

      p    = atcd.parser.buff + atcd.parser.line_pos + strlen("$GPACCURACY,");
      endl = atcd.parser.buff + atcd.parser.buff_pos;

      //if(atcd.gps.callback != NULL && (atcd.gps.cb_events & ATCD_GSM_EV_REG) != 0) atcd.gps.callback(ATCD_GSM_EV_REG);

      atcd.parser.buff_pos = atcd.parser.line_pos;
      return 1;

    skip_proc5:
      ATCD_DBG_GPS_ACC_ERR
      atcd.parser.buff_pos = atcd.parser.line_pos;
      return 1;
    }

    return 0;
}
//------------------------------------------------------------------------------
