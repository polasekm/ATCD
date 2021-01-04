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
    atcd_gps_reset();

    atcd.gps.cb_events = ATCD_GPS_EV_NONE;
    atcd.gps.callback = NULL;
}
//------------------------------------------------------------------------------
void atcd_gps_reset()
{
    atcd.gps.state = ATCD_GPS_STATE_OFF;

    atcd.gps.date[0] = 0;
    atcd.gps.time[0] = 0;
    atcd.gps.time_fix[0] = 0;

    atcd.gps.sats = 0;

    atcd.gps.latitude = 0;
    atcd.gps.longitude = 0;
    atcd.gps.altitude = 0;
    atcd.gps.undulation = 0;
    atcd.gps.fix_mode = ATCD_GPS_FIX_M_NO;

    atcd.gps.speed = 0;
    atcd.gps.course = 0;

    atcd.gps.pdop = 0;
    atcd.gps.hdop = 0;
    atcd.gps.vdop = 0;
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

    // u moemu A7 je prvni veta uvozena sekvenci nize...
    // asi osetrit co kdyby pak nebyla NMEA veta...
    if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "+GPSRD:", strlen("+GPSRD:")) == 0)
    {
        atcd.parser.line_pos += strlen("+GPSRD:");
    }

    if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "$GP", strlen("$GP")) == 0 ||
       strncmp(atcd.parser.buff + atcd.parser.line_pos, "$GN", strlen("$GN")) == 0 ||
       strncmp(atcd.parser.buff + atcd.parser.line_pos, "$GL", strlen("$GL")) == 0)
    {
        ATCD_DBG_GPS_SENTECE

        if(atcd.gps.state == ATCD_GPS_STATE_OFF)
        {
            ATCD_DBG_GPS_SENTECE_OFF

            atcd.parser.buff_pos = atcd.parser.line_pos;
            return 1;
        }

        p    = atcd.parser.buff + atcd.parser.line_pos + strlen("$GP") ;
        endl = atcd.parser.buff + atcd.parser.buff_pos;

        if(strncmp(atcd.parser.buff + atcd.parser.line_pos + strlen("$GP"), "RMC,", strlen("RMC,")) == 0)
        {
            //RMC (Recommended Minimum Navigation Information)
            //Minimální doporučená informace pro navigac
            ATCD_DBG_GPS_RMC

            p += strlen("RMC,");

            //Time (UTC)
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc;
            memcpy(atcd.gps.time, p, np - p);
            atcd.gps.time[np - p] = 0;
            p = np + 1;

            //Status
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc;
            if(*p == 'A')
            {
                atcd.gps.state = ATCD_GPS_STATE_FIX;
                strcpy(atcd.gps.time_fix, atcd.gps.time);
            }
            else atcd.gps.state = ATCD_GPS_STATE_SEARCHING;
            p = np + 1;

            //Latitude
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc;
            if(atcd.gps.state == ATCD_GPS_STATE_FIX) atcd.gps.latitude = atof(p);
            p = np + 1;

            //NS
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc;
            if(atcd.gps.state == ATCD_GPS_STATE_FIX && *p == 'S') atcd.gps.latitude = -atcd.gps.latitude;
            p = np + 1;

            //Longitude
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc;
            if(atcd.gps.state == ATCD_GPS_STATE_FIX) atcd.gps.longitude = atof(p);
            p = np + 1;

            //EW
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc;
            if(atcd.gps.state == ATCD_GPS_STATE_FIX && *p == 'E') atcd.gps.longitude = -atcd.gps.longitude;
            p = np + 1;

            //Speed    --- neni to v uzlech?
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc;
            if(atcd.gps.state == ATCD_GPS_STATE_FIX) atcd.gps.speed = atof(p);
            p = np + 1;

            //Course
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc;
            if(atcd.gps.state == ATCD_GPS_STATE_FIX) atcd.gps.course = atof(p);
            p = np + 1;

            //Date
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc;
            memcpy(atcd.gps.date, p, np - p);
            atcd.gps.date[np - p] = 0;
            p = np + 1;

            if(atcd.gps.callback != NULL && (atcd.gps.cb_events & ATCD_GPS_EV_UPDATE) != 0) atcd.gps.callback(ATCD_GPS_EV_UPDATE);

            atcd.parser.buff_pos = atcd.parser.line_pos;
            return 1;

          skip_proc:
            ATCD_DBG_GPS_RMC_ERR
            atcd.parser.buff_pos = atcd.parser.line_pos;
            return 1;
        }
        else if(strncmp(atcd.parser.buff + atcd.parser.line_pos + strlen("$GP"), "GSA,", strlen("GSA,")) == 0)
        {
            //GSA, aktivní satelity a DOP (Dilution Of Precision)
            ATCD_DBG_GPS_GSA

            p += strlen("GSA,");

            //mode MA
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc2;
            //if(*p == 'A') atcd.gps.state = ATCD_GPS_STATE_FIX;
            //else atcd.gps.state = ATCD_GPS_STATE_SEARCHING;
            p = np + 1;

            //Fix mode
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc2;
            if(*p == '2') atcd.gps.fix_mode = ATCD_GPS_FIX_M_2D;
            else if(*p == '3') atcd.gps.fix_mode = ATCD_GPS_FIX_M_3D;
            else atcd.gps.fix_mode = ATCD_GPS_FIX_M_NO;
            p = np + 1;

            //PRN numbers of satellites used in solution (null for unused fields), total of 12 fields
            //4.
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc2;
            //atcd.gps.latitude = atof(p);
            p = np + 1;

            //5.
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc2;
            //atcd.gps.latitude = atof(p);
            p = np + 1;

            //6.
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc2;
            //atcd.gps.latitude = atof(p);
            p = np + 1;

            //7.
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc2;
            //atcd.gps.latitude = atof(p);
            p = np + 1;

            //8.
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc2;
            //atcd.gps.latitude = atof(p);
            p = np + 1;

            //9.
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc2;
            //atcd.gps.latitude = atof(p);
            p = np + 1;

            //10.
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc2;
            //atcd.gps.latitude = atof(p);
            p = np + 1;

            //11.
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc2;
            //atcd.gps.latitude = atof(p);
            p = np + 1;

            //12.
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc2;
            //atcd.gps.latitude = atof(p);
            p = np + 1;

            //13.
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc2;
            //atcd.gps.latitude = atof(p);
            p = np + 1;

            //14.
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc2;
            //atcd.gps.latitude = atof(p);
            p = np + 1;

            //15.
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc2;
            //atcd.gps.latitude = atof(p);
            p = np + 1;

            //PDOP
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc2;
            if(atcd.gps.state == ATCD_GPS_STATE_FIX) atcd.gps.pdop = atof(p);
            p = np + 1;

            //HDOP
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc2;
            if(atcd.gps.state == ATCD_GPS_STATE_FIX) atcd.gps.hdop = atof(p);
            p = np + 1;

            //VDOP
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc2;
            if(atcd.gps.state == ATCD_GPS_STATE_FIX) atcd.gps.vdop = atof(p);
            p = np + 1;

            if(atcd.gps.callback != NULL && (atcd.gps.cb_events & ATCD_GPS_EV_UPDATE) != 0) atcd.gps.callback(ATCD_GPS_EV_UPDATE);

            atcd.parser.buff_pos = atcd.parser.line_pos;
            return 1;

          skip_proc2:
            ATCD_DBG_GPS_GSA_ERR
            atcd.parser.buff_pos = atcd.parser.line_pos;
            return 1;
        }
        else if(strncmp(atcd.parser.buff + atcd.parser.line_pos + strlen("$GP"), "GGA,", strlen("GGA,")) == 0)
        {
            //GGA - zeměpisná délka a šířka, geodetická výška, čas určení souřadnic
            ATCD_DBG_GPS_GGA

            p += strlen("GGA,");

            //Čas (UTC), pro který platí údaje o vypočtené pozici
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc3;
            //memcpy(atcd.gps.time, p, np - p);
            //atcd.gps.time[np - p] = 0;
            p = np + 1;

            //Latitude
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc3;
            //if(atcd.gps.state == ATCD_GPS_STATE_FIX) atcd.gps.latitude = atof(p);
            p = np + 1;

            //NS
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc3;
            //if(atcd.gps.state == ATCD_GPS_STATE_FIX && *p == 'S') atcd.gps.latitude = -atcd.gps.latitude;
            p = np + 1;

            //Longitude
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc3;
            //if(atcd.gps.state == ATCD_GPS_STATE_FIX) atcd.gps.longitude = atof(p);
            p = np + 1;

            //EW
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc3;
            //if(atcd.gps.state == ATCD_GPS_STATE_FIX && *p == 'E') atcd.gps.longitude = -atcd.gps.longitude;
            p = np + 1;

            //Status - rezim urceni polohy - Fix quality
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc3;
            if(*p == '0') atcd.gps.state = ATCD_GPS_STATE_SEARCHING;
            else atcd.gps.state = ATCD_GPS_STATE_FIX;
            p = np + 1;

            //Počet viditelných satelitů - Number of satellites being tracked - Satellites used
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc3;
            atcd.gps.sats = atoi(p);
            p = np + 1;

            //HDOP
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc3;
            if(atcd.gps.state == ATCD_GPS_STATE_FIX) atcd.gps.hdop = atof(p);
            p = np + 1;

            //Altitude
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc3;
            if(atcd.gps.state == ATCD_GPS_STATE_FIX) atcd.gps.altitude = atof(p);
            p = np + 1;

            //Jednotka pro předchozí údaj (č.9) (M=metr)
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc3;
            //if(*p == 'E') atcd.gps.longitude = -atcd.gps.longitude;
            p = np + 1;

            //Undulation
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc3;
            if(atcd.gps.state == ATCD_GPS_STATE_FIX) atcd.gps.undulation = atof(p);
            p = np + 1;

            //Jednotka vzdálenosti pro předchozí položku (č.11) (M=metr)
            np = (char*)memchr(p, ',', endl - p);
            if(np == NULL) goto skip_proc3;
            //if(*p == 'E') atcd.gps.longitude = -atcd.gps.longitude;
            p = np + 1;

            if(atcd.gps.callback != NULL && (atcd.gps.cb_events & ATCD_GPS_EV_UPDATE) != 0) atcd.gps.callback(ATCD_GPS_EV_UPDATE);

            atcd.parser.buff_pos = atcd.parser.line_pos;
            return 1;

          skip_proc3:
            ATCD_DBG_GPS_GGA_ERR
            atcd.parser.buff_pos = atcd.parser.line_pos;
            return 1;
        }

        if(strncmp(atcd.parser.buff + atcd.parser.line_pos + strlen("$GP"), "GSV,", strlen("GSV,")) == 0)
        {
            //GSV (Satellites in View)
            // Informace o družicích
            ATCD_DBG_GPS_GSV

            p += strlen("GSV,");

            atcd.parser.buff_pos = atcd.parser.line_pos;
            return 1;

          /*skip_proc4:
            ATCD_DBG_GPS_GSV_ERR
            atcd.parser.buff_pos = atcd.parser.line_pos;
            return 1;*/
        }

        if(strncmp(atcd.parser.buff + atcd.parser.line_pos + strlen("$GP"), "VTG,", strlen("VTG,")) == 0)
        {
            ATCD_DBG_GPS_VTG

            p += strlen("VTG,");

            atcd.parser.buff_pos = atcd.parser.line_pos;
            return 1;

          /*skip_proc5:
            ATCD_DBG_GPS_VTG_ERR
            atcd.parser.buff_pos = atcd.parser.line_pos;
            return 1;*/
        }

        if(strncmp(atcd.parser.buff + atcd.parser.line_pos + strlen("$GP"), "ACCURACY,", strlen("ACCURACY,")) == 0)
        {
            ATCD_DBG_GPS_ACC

            p += strlen("ACCURACY,");

            atcd.parser.buff_pos = atcd.parser.line_pos;
            return 1;

          /*skip_proc6:
            ATCD_DBG_GPS_ACC_ERR
            atcd.parser.buff_pos = atcd.parser.line_pos;
            return 1;*/
        }

        if(strncmp(atcd.parser.buff + atcd.parser.line_pos + strlen("$GP"), "HWBIAS,", strlen("HWBIAS,")) == 0)
        {
            ATCD_DBG_GPS_BIAS

            p += strlen("HWBIAS,");

            atcd.parser.buff_pos = atcd.parser.line_pos;
            return 1;

          /*skip_proc7:
            ATCD_DBG_GPS_BIAS_ERR
            atcd.parser.buff_pos = atcd.parser.line_pos;
            return 1;*/
        }

        ATCD_DBG_GPS_USENTECE

        atcd.parser.buff_pos = atcd.parser.line_pos;
        return 1;
    }

    return 0;
}
//------------------------------------------------------------------------------
int atcd_gps_checksum(const char *s)
{
    int c = 0;

    while (*s)
        c ^= *s++;

    return c;
}
//------------------------------------------------------------------------------
void atcd_gps_enable()
{
    if(atcd.gps.state == ATCD_GPS_STATE_OFF || atcd.gps.state == ATCD_GPS_STATE_W_OFF) atcd.gps.state = ATCD_GPS_STATE_W_SEARCH;
}
//------------------------------------------------------------------------------
void atcd_gps_disable()
{
    if(atcd.gps.state != ATCD_GPS_STATE_OFF && atcd.gps.state != ATCD_GPS_STATE_W_OFF) atcd.gps.state = ATCD_GPS_STATE_W_OFF;
}
//------------------------------------------------------------------------------
