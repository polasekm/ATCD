/*
 * atcd_setup.c
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin
 */
//------------------------------------------------------------------------------
#include "atcd_wifi.h"
#include "atcd.h"

extern atcd_t atcd;

//------------------------------------------------------------------------------
void atcd_setup_init()
{
  atcd.setup.clean=1;
  atcd.setup.clvl=60;
  atcd.setup.echo.np=96;
  atcd.setup.echo.ae=253;
  atcd.setup.echo.nr=16388;
  atcd.setup.echo.ns=20488;
  atcd.setup.echo.can=1;
  atcd.setup.echo.mirror_can=1;
  atcd.setup.cmic=12;
  atcd.setup.cagcset=1;
  atcd.setup.crsl=0;
  atcd.setup.sidet0=0;
}
//------------------------------------------------------------------------------
void atcd_setup_reset()
{
  atcd.setup.clean=0;
  atcd.setup.echo.mirror_can=1;
}
//------------------------------------------------------------------------------
/*void atcd_setup_proc()
{

}
//------------------------------------------------------------------------------
uint8_t atcd_setup_asc_msg()
{
  return 0;
}*/
//------------------------------------------------------------------------------
void atcd_setup_clvl(uint8_t clvl)
{
  atcd.setup.clvl=clvl;
  atcd.setup.clean=0;
}

void atcd_setup_echo(uint16_t np, uint16_t ae, uint16_t nr, uint16_t ns, uint8_t can)
{
  atcd.setup.echo.np=np;
  atcd.setup.echo.ae=ae;
  atcd.setup.echo.nr=nr;
  atcd.setup.echo.ns=ns;
  atcd.setup.echo.can=can;
  atcd.setup.clean=0;
}

void atcd_setup_cmic(uint8_t cmic)
{
  atcd.setup.cmic=cmic;
  atcd.setup.clean=0;
}

void atcd_setup_cagcset(uint8_t mic_agc_en)
{
  atcd.setup.cagcset=mic_agc_en;
  atcd.setup.clean=0;
}

void atcd_setup_crsl(uint8_t crsl)
{
  atcd.setup.crsl=crsl;
  atcd.setup.clean=0;
}

void atcd_setup_sidet(uint8_t sidet0)
{
  atcd.setup.sidet0=sidet0;
  atcd.setup.clean=0;
}
