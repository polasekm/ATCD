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
    uint8_t val;
    uint8_t state_p;

    char ns, ew;

    if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "$GPRMC,", strlen("$GPRMC,")) == 0)
    {
      ATCD_DBG_GPS_RMC
      //ATCD_DBG_GPS_GSA
      //ATCD_DBG_GPS_GGA

      if(sscanf(atcd.parser.buff + atcd.parser.line_pos, "%*,%s,%*,%f,%c,%f,%c,%f,%f,%s", atcd.gps.time, &atcd.gps.latitude, &ns, &atcd.gps.longitude, &ew, &atcd.gps.speed, &atcd.gps.course, atcd.gps.date) != 8)
      {
          ATCD_DBG_GPS_RMC_ERR
      }

      val = (uint8_t)atoi(atcd.parser.buff + atcd.parser.buff_pos - ATCD_RX_NL_LEN - 1);

      if(val >= 0 && val <= 10)
      {
        state_p = atcd.gsm.state;

        atcd.gsm.state = val;
        atcd.parser.buff_pos  = atcd.parser.line_pos;

        //Pokud probihal ATC, nemazat z odpovedi...

        if(state_p != val)
        {
          atcd_conn_reset_all();

        }
      }

      //if(atcd.gps.callback != NULL && (atcd.gps.cb_events & ATCD_GSM_EV_REG) != 0) atcd.gps.callback(ATCD_GSM_EV_REG);

      atcd.parser.buff_pos = atcd.parser.line_pos;

      return 1;
    }

    return 0;
}
//------------------------------------------------------------------------------
