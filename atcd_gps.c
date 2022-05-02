/*
 * usart.c
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin
 */
//------------------------------------------------------------------------------
#include "atcd_gps.h"
#include "atcd.h"

#include <stdlib.h>

extern atcd_t atcd;

//------------------------------------------------------------------------------
uint8_t atcd_gps_checksum(const char *s);
//------------------------------------------------------------------------------
void atcd_gps_init()
{
  atcd_gps_reset();

  atcd.gps.cb_events = ATCD_GPS_EV_NONE;
  atcd.gps.callback = NULL;

  atcd.gps.stat.run_time_acc = 0;
  atcd.gps.stat.cs_err = 0;
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

  atcd.gps.last_fix = 0;
  atcd.gps.last_nmea_time = 0;
  atcd.gps.stat.time_to_fix = 0;
  atcd.gps.stat.first_search = 0;
  atcd.gps.stat.start_time = 0;
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
  uint8_t cs, scs;
  char *str;

  // u moemu A7 je prvni veta uvozena sekvenci nize...
  // asi osetrit co kdyby pak nebyla NMEA veta...

  str = atcd.parser.buff + atcd.parser.line_pos;

  if(strncmp(str, "+GPSRD:", strlen("+GPSRD:")) == 0)
  {
    str += strlen("+GPSRD:");
  }

  if(strncmp(str, "$GP", 3) == 0 || //GPS
     strncmp(str, "$GN", 3) == 0 || //"nejlepsi" z GP, GL, GA, BD
     strncmp(str, "$GA", 3) == 0 || //Galileo
     strncmp(str, "$GL", 3) == 0 || //Glonass
     strncmp(str, "$BD", 3) == 0)   //Beidou
  {
    ATCD_DBG_GPS_SENTECE

    atcd.gps.last_nmea_time = atcd_get_ms();

    if(atcd.gps.state == ATCD_GPS_STATE_OFF || atcd.gps.state == ATCD_GPS_STATE_W_OFF)
    {
      ATCD_DBG_GPS_SENTECE_OFF
      atcd.gps.state = ATCD_GPS_STATE_W_OFF;
      atcd.parser.buff_pos = atcd.parser.line_pos;
      return 1;
    }

    p    = str + strlen("$GP");
    endl = atcd.parser.buff + atcd.parser.buff_pos;

    cs = atcd_gps_checksum(str + 1);
    scs = strtoul(endl - 4, NULL, 16);

    if(cs != scs)
    { //$GPGSA,A,1,,,,,,,,,,,,,,*1E   <-- nejaky problem s UARTem
      //ATCD: GPS: Chybny checksum vety
      //$GPGSA,A,1,,,,,,,,,,,,,,,*1E
      //je v pohode
      ATCD_DBG_GPS_CS_ERR
      atcd.gps.stat.cs_err++;
      atcd.parser.buff_pos = atcd.parser.line_pos;
      return 1;
    }

    if(strncmp(p, "RMC,", strlen("RMC,")) == 0)
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
      { //$GNRMC,171340.000,A,5003.392352,N,01432.710519,E,0.00,90.64,161121,,,A*4A
        //aktualizace casu poslednich platnych dat
        strcpy(atcd.gps.time_fix, atcd.gps.time);
        atcd.gps.last_fix = atcd_get_ms();

        if(atcd.gps.state == ATCD_GPS_STATE_SEARCHING && atcd.gps.stat.first_search != 0)
        {
          atcd.gps.stat.time_to_fix = atcd.gps.last_fix - atcd.gps.stat.start_time;  //aktualizace doby trvani prvniho fixnuti
          atcd.gps.stat.first_search = 0;
        }
        atcd.gps.state = ATCD_GPS_STATE_FIX;
      }
      else
      { //$GNRMC,181603.000,V,,,,,0.98,69.11,161121,,,N*64
        //vyhodit do fce
        //if(atcd.gps.state == ATCD_GPS_STATE_FIX) atcd.gps.start_time = atcd_get_ms();
        atcd.gps.state = ATCD_GPS_STATE_SEARCHING;
      }

      p = np + 1;

      //Latitude
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc;
      if(atcd.gps.state == ATCD_GPS_STATE_FIX)
      { //5003.395958 -> 50+3.395958/60
        atcd.gps.latitude = atof(p + 2) / (double)60.f;
        *(p + 2) = 0;
        atcd.gps.latitude += atof(p);
      }
      p = np + 1;

      //NS
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc;
      if(atcd.gps.state == ATCD_GPS_STATE_FIX && *p == 'S') atcd.gps.latitude = -atcd.gps.latitude;
      p = np + 1;

      //Longitude
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc;
      if(atcd.gps.state == ATCD_GPS_STATE_FIX)
      { //01432.702996 -> 14+32.702996/60
        atcd.gps.longitude = atof(p + 3) / (double)60.f;
        *(p + 3) = 0;
        atcd.gps.longitude += atof(p);
      }
      p = np + 1;

      //EW
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc;
      if(atcd.gps.state == ATCD_GPS_STATE_FIX && *p == 'W') atcd.gps.longitude = -atcd.gps.longitude;
      p = np + 1;

      //Speed - je v uzlech - nutno nasobit
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc;
      if(atcd.gps.state == ATCD_GPS_STATE_FIX) atcd.gps.speed = atof(p) * 1.852;
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

      if(atcd.gps.callback != NULL && (atcd.gps.cb_events & ATCD_GPS_EV_UPDATE) != 0) atcd.gps.callback(ATCD_GPS_EV_UPDATE, &atcd.gps);

      atcd.parser.buff_pos = atcd.parser.line_pos;
      return 1;

    skip_proc:
      ATCD_DBG_GPS_RMC_ERR
      atcd.parser.buff_pos = atcd.parser.line_pos;
      return 1;
    }

    if(strncmp(p, "GSA,", strlen("GSA,")) == 0)
    {
      //GSA, aktivní satelity a DOP (Dilution Of Precision)
      ATCD_DBG_GPS_GSA

      p += strlen("GSA,");

      //mode MA
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc2;
      // tohle A je automatic!!! M je manual!!! neni stav dat!!!!
      //if(*p == 'A') atcd.gps.state = ATCD_GPS_STATE_FIX;
      //else atcd.gps.state = ATCD_GPS_STATE_SEARCHING;

      // nemohou se nekde parsovat nexistuji data pokud je ocekava za nejakou carkou?

      p = np + 1;

      //Fix mode
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc2;
      // TODO: aktulizovat jen pokud jsou platna data (nekonzistence pri prvni vete?)
      //potrebuji vedet jestli 2D nebo 3D hned v prvni vete; na ATCD_GPS_FIX_M_NO nemam pevny nazor
      if(*p == '2') atcd.gps.fix_mode = ATCD_GPS_FIX_M_2D;
      else if(*p == '3') atcd.gps.fix_mode = ATCD_GPS_FIX_M_3D;
      else if(atcd.gps.state == ATCD_GPS_STATE_FIX)
      {
        atcd.gps.fix_mode = ATCD_GPS_FIX_M_NO;
      }
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
      if (np==NULL)
        np = (char*)memchr(p, '*', endl - p);
      //chodi jen $GPGSA,A,1", ',' <repeats 15 times>, "*1E\r\n coz je o polozku mene
      //$GPGSA,A,1,,,,,,,,,,,,,,,*1E
      //$GPGSA,A,1,s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,pdop,hdop,*1E  tak nekdo urcite nechodi
      //chodi $GPGSA,A,3,27,16,08,26,10,,,,,,,,4.44,2.92,3.35*05
      //chodi $GLGSA,A,3,,,,,,,,,,,,,4.44,2.92,3.35*16
      //$GPGSA,A,3,27,16,08,26,10,  ,  ,  ,  ,   ,   ,   ,4.44,2.92,3.35*05
      //$GPGSA,A,1,s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,pdop,hdop,vdop*1E jsou tu vsichni a stejne error

      //$GPGSA,A,3,08,23,10,27,21,,,,,,,,1.57,1.26,0.94*07
      //ATCD: GPS: Chyba parsovani vety GSA
      //$GLGSA,A,3,76,68,,,,,,,,,,,1.57,1.26,0.94*1A
      //ATCD: GPS: Chyba parsovani vety GSA

      if(np == NULL) goto skip_proc2;
      if(atcd.gps.state == ATCD_GPS_STATE_FIX) atcd.gps.vdop = atof(p);
      p = np + 1;

      if(atcd.gps.callback != NULL && (atcd.gps.cb_events & ATCD_GPS_EV_UPDATE) != 0) atcd.gps.callback(ATCD_GPS_EV_UPDATE, &atcd.gps);

      atcd.parser.buff_pos = atcd.parser.line_pos;
      return 1;

    skip_proc2:
      ATCD_DBG_GPS_GSA_ERR
      atcd.parser.buff_pos = atcd.parser.line_pos;
      return 1;
    }

    if(strncmp(p, "GGA,", strlen("GGA,")) == 0)
    {
      uint8_t ggavalid=0;
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
      /*if(atcd.gps.state == ATCD_GPS_STATE_FIX)
      {
        atcd.gps.latitude = atof(p + 2) / (double)60.f;
        *(p + 2) = 0;
        atcd.gps.latitude += atof(p);
      }*/
      p = np + 1;

      //NS
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc3;
      //if(atcd.gps.state == ATCD_GPS_STATE_FIX && *p == 'S') atcd.gps.latitude = -atcd.gps.latitude;
      p = np + 1;

      //Longitude
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc3;
      /*if(atcd.gps.state == ATCD_GPS_STATE_FIX)
      {
        atcd.gps.longitude = atof(p + 3) / (double)60.f;
        *(p + 3) = 0;
        atcd.gps.longitude += atof(p);
      }*/
      p = np + 1;

      //EW
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc3;
      //if(atcd.gps.state == ATCD_GPS_STATE_FIX && *p == 'W') atcd.gps.longitude = -atcd.gps.longitude;
      p = np + 1;

      //Status - rezim urceni polohy - Fix quality
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc3;
      //TODO: cele opravit

      if(*p != '0')
        ggavalid=1; //potrebuji chytit hned prvni pocet satelitu a altitude protoze az v RMC prijde fix tak uz posilam SMSku s polohou
      /*if(*p == '0')
      {
        //vyhodit do fce
        //if(atcd.gps.state == ATCD_GPS_STATE_FIX) atcd.gps.start_time = atcd_get_ms();
        atcd.gps.state = ATCD_GPS_STATE_SEARCHING;
      }
      else
      {
        atcd.gps.last_fix = atcd_get_ms();

        if(atcd.gps.state == ATCD_GPS_STATE_SEARCHING && atcd.gps.start_time != 0)
        {
          atcd.gps.time_to_fix = atcd.gps.last_fix - atcd.gps.start_time;  //aktualizace doby trvani prvniho fixnuti
          atcd.gps.start_time = 0;
        }
        atcd.gps.state = ATCD_GPS_STATE_FIX;
      }*/
      p = np + 1;

      //Počet pouzitych satelitů - Number of satellites being tracked - Satellites used
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc3;

      //TODO: prejmeovat
      if((atcd.gps.state == ATCD_GPS_STATE_FIX) || ggavalid) atcd.gps.sats = atoi(p);
      p = np + 1;

      //HDOP
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc3;
      if((atcd.gps.state == ATCD_GPS_STATE_FIX) || ggavalid) atcd.gps.hdop = atof(p);
      p = np + 1;

      //Altitude
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc3;
      if((atcd.gps.state == ATCD_GPS_STATE_FIX) || ggavalid) atcd.gps.altitude = atof(p);
      p = np + 1;

      //Jednotka pro předchozí údaj (č.9) (M=metr)
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc3;
      //if(*p == 'E') atcd.gps.longitude = -atcd.gps.longitude;
      p = np + 1;

      //Undulation
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc3;
      if((atcd.gps.state == ATCD_GPS_STATE_FIX) || ggavalid) atcd.gps.undulation = atof(p);
      p = np + 1;

      //Jednotka vzdálenosti pro předchozí položku (č.11) (M=metr)
      np = (char*)memchr(p, ',', endl - p);
      if(np == NULL) goto skip_proc3;
      //if(*p == 'E') atcd.gps.longitude = -atcd.gps.longitude;
      p = np + 1;

      if(atcd.gps.callback != NULL && (atcd.gps.cb_events & ATCD_GPS_EV_UPDATE) != 0) atcd.gps.callback(ATCD_GPS_EV_UPDATE, &atcd.gps);

      atcd.parser.buff_pos = atcd.parser.line_pos;
      return 1;

    skip_proc3:
      ATCD_DBG_GPS_GGA_ERR
      atcd.parser.buff_pos = atcd.parser.line_pos;
      return 1;
    }

    if(strncmp(p, "GSV,", strlen("GSV,")) == 0)
    {
      //GSV (Satellites in View)
      // Informace o družicích
      ATCD_DBG_GPS_GSV

      p += strlen("GSV,");
      //gsv total
      //gsv index
      //satellites total
      //[sat id, elev, azim, SNR] x1..4
      //$GPGSV,3,1,12, 27,69,300,,    23,55,083,,    10,55,150,,     16,52,211,26      *78
      //$GPGSV,3,2,12, 08,35,298,23,  18,33,066,,    26,25,186,21,   15,14,048,        *7C
      //$GPGSV,3,3,12, 07,10,310,,    21,07,254,,    13,06,021,,     30,03,341,        *78
      //$GLGSV,3,1,09, 88,69,017,,    65,38,117,,    79,38,293,,     72,37,055,        *64
      //$GLGSV,3,2,09, 81,36,289,,    87,35,072,,    78,27,230,,     80,09,345,        *66
      //$GLGSV,3,3,09, 71,08,014,                                                      *57

      //myslim, ze by se hodilo vedet, jestli aspon 1 satelit ma SNR -> je platny cas

      atcd.parser.buff_pos = atcd.parser.line_pos;
      return 1;

    /*skip_proc4:
      ATCD_DBG_GPS_GSV_ERR
      atcd.parser.buff_pos = atcd.parser.line_pos;
      return 1;*/
    }

    if(strncmp(p, "VTG,", strlen("VTG,")) == 0)
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

    if(strncmp(p, "ACCURACY,", strlen("ACCURACY,")) == 0)
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

    if(strncmp(p, "HWBIAS,", strlen("HWBIAS,")) == 0)
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
uint8_t atcd_gps_checksum(const char *s)
{
  uint8_t c = 0;

  while(*s != '*' && *s != '\n') c ^= *s++;

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
atcd_gps_state_t atcd_gps_state()
{
  return atcd.gps.state;
}
//------------------------------------------------------------------------------
uint32_t atcd_gps_last_fix()
{
  return atcd.gps.last_fix;
}
//------------------------------------------------------------------------------
static uint8_t classify_gps_state_power[]={0, 0, 1, 1, 1};

uint32_t atcd_gps_runtime()
{
  uint32_t run_time;
  static_assert(sizeof(classify_gps_state_power)==ATCD_GPS_STATE__COUNT);

  run_time = atcd.gps.stat.run_time_acc;
  if(classify_gps_state_power[atcd.gps.state]) run_time += atcd_get_ms() - atcd.gps.stat.start_time;

  return run_time;
}
//-----------------------------------------------------------------------------
void atcd_gps_set_callback(uint8_t events, void (*gps_callback)(uint8_t event, const atcd_gps_t *gps))
{
  atcd.gps.cb_events = events;
  atcd.gps.callback = gps_callback;
}
//-----------------------------------------------------------------------------
//zjisteni stazenych almanachu
//at+cgnscmd=0,"$PMTK661,01*1E"
//$PMTK001,661,3,f7ffffff*4C jsou asi vsechny
//$PMTK001,661,3,7fc003f*7A  neco jo neco ne

//zjisteni stazenych efemerid
//at+cgnscmd=0,"$PMTK660,01*1F"
//$PMTK001,660,3,4800*10  .. znamena ?
//$PMTK001,660,3,20080010*17 = 20 080010
//                             satelity 71-68 67-64 protoze 0x20 je urcite sat 69   ale i 72
//                                24-21 20-17 16-13 12-9 8-5 4-1 protoze 080010 je urcite 20 a 5
//$PMTK001,660,3,20084000*12  15,19,24,72
//$PMTK001,660,3,20484000*16  15,24,30,5  72,65
//$PMTK001,660,3,81010*24    nemel by mit spis nic

//$PMTK001,660,3,804000*10 ma jen par
//$GPGSA,A,1,,,,,,,,,,,,,,,*1E    na GP mame 2 satelity se sig 17 a 18
//$GLGSA,A,1,,,,,,,,,,,,,,,*02    na GL zadny
//$GPGSV,4,1,13,24,81,288,,19,48,084,,12,44,240,17,17,34,053,*7F  = {24,81,288,},{19,48,084,},{12,44,240,17},{17,34,053,}
//$GPGSV,4,2,13,15,32,190,,13,17,161,,10,15,291,18,25,11,244,*71  = {15,32,190,},{13,17,161,},{10,15,291,18},{25,11,244,}
//$GPGSV,4,3,13,23,09,258,,32,09,323,,14,07,062,,01,05,016,*74    = {23,09,258,},{32,09,323,},{14,07,062,},{01,05,016,}
//$GPGSV,4,4,13,06,03,110,*4E                                     = {06,03,110,}
//$GLGSV,3,1,10,77,69,331,,67,42,242,,76,40,066,,68,36,312,*65    = {77,69,331,},{67,42,242,},{76,40,066,},{68,36,312,}
//$GLGSV,3,2,10,86,36,074,,78,22,275,,85,19,022,,87,14,139,*66    = {86,36,074,},{78,22,275,},{85,19,022,},{87,14,139,}
//$GLGSV,3,3,10,66,09,197,,69,02,349,*61                          = {66,09,197,},{69,02,349,}


//$GPGSA,A,1,,,,,,,,,,,,,,,*1E
//$GLGSA,A,1,,,,,,,,,,,,,,,*02
//$GPGSV,4,1,13,24,81,288,,19,48,084,17,12,44,240,19,17,34,053,17*71   -,17,19,17  pro 24,19,12,17
//$GPGSV,4,2,13,15,32,190,21,13,17,161,,10,15,291,18,25,11,244,*72     21,-,18,-   pro 15,10
//$GPGSV,4,3,13,23,09,258,,32,09,323,,14,07,062,,01,05,016,*74         -,-,-,-
//$GPGSV,4,4,13,06,03,110,*4E                                          -
//$GLGSV,3,1,10,77,70,334,,67,41,241,,76,39,067,,68,37,311,*65         -,-,-,-
//$GLGSV,3,2,10,86,37,072,,78,23,276,,85,18,022,,87,15,139,*63         -,-,-,-
//$GLGSV,3,3,10,66,08,197,,69,03,349,*61                               -,-
//$GNRMC,165648.336,V,,,,,0.44,0.00,251121,,,N*59

//$GPGSA,A,2,15,10,12,17,19,,,,,,,,1.55,1.21,0.96*07                              a tady mame 19,12,17,15,10
//$GLGSA,A,2,,,,,,,,,,,,,1.55,1.21,0.96*13
//$GPGSV,4,1,13,24,81,288,,19,48,084,17,12,44,240,19,17,34,053,16*70
//$GPGSV,4,2,13,15,32,190,21,13,17,161,,10,15,291,18,25,11,244,*72
//$GPGSV,4,3,13,23,09,258,,32,09,323,,14,07,062,,01,05,016,*74
//$GPGSV,4,4,13,06,03,110,*4E
//$GLGSV,3,1,10,77,70,334,,67,41,241,,76,39,067,,68,37,311,*65
//$GLGSV,3,2,10,86,37,072,,78,23,276,,85,18,022,,87,15,139,*63
//$GLGSV,3,3,10,66,08,197,,69,03,349,*61
//$GNRMC,165649.336,A,5003.392244,N,01432.693248,E,0.49,0.00,251121,,,A*7C
