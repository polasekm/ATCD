/*
 * usart.c
 *
 *  Created on: Apr 9, 2011
 *      Author: Martin
 */
//------------------------------------------------------------------------------
#include "atcd_phone.h"
#include "atcd.h"

extern atcd_t atcd;
//------------------------------------------------------------------------------
void atcd_phone_init()   //inializace telefonu
{
  atcd_phone_reset();
  //atcd.phone.pin = NULL;

  atcd.phone.cb_events = ATCD_PHONE_EV_ALL;
  atcd.phone.callback = NULL;

  atcd.phone.sms.cb_events = ATCD_SMS_EV_ALL;
  atcd.phone.sms.callback = NULL;

  atcd.phone.sms_tx.cb_events = ATCD_SMS_EV_ALL;
  atcd.phone.sms_tx.callback = NULL;
}
//------------------------------------------------------------------------------
void atcd_phone_reset()                   //phone state reset
{
  atcd.phone.state = ATCD_PHONE_STATE_IDLE;
  atcd.phone.number[0] = 0;
  atcd.phone.numbertype = -1;
  atcd.phone.ring_cnt = 0;
  atcd.phone.miss_call_cnt = 0;

  atcd.phone.dtmf_rx_tone = 0;
  atcd.phone.dtmf_tx_tone = 0;
  atcd.phone.dtmf_tx_dur = 1;

  //----------------------------
  // RX SMS
  atcd.phone.sms.sender = atcd.phone.sms_sender_buff;
  atcd.phone.sms.sender[0] = 0;
  atcd.phone.sms.datetime = atcd.phone.sms_datetime_buff;
  atcd.phone.sms.datetime[0] = 0;
  atcd.phone.sms.message = atcd.phone.sms_message_buff;
  atcd.phone.sms.message[0] = 0;

  atcd.phone.sms.len = 0;
  atcd.phone.sms.index = 0;
  atcd.phone.sms.state = ATCD_PHONE_SMS_STATE_IDLE;
  atcd.phone.sms.result = 0;

  atcd.phone.sms.timeout = 0;
  atcd.phone.sms.next = NULL;

  //----------------------------
  // TX SMS
  atcd.phone.sms_tx.sender = NULL;
  atcd.phone.sms_tx.datetime = NULL;
  atcd.phone.sms_tx.message = NULL;

  atcd.phone.sms_tx.len = 0;
  atcd.phone.sms_tx.index = 0;
  atcd.phone.sms_tx.state = ATCD_PHONE_SMS_STATE_IDLE;
  atcd.phone.sms_tx.result = 0;

  atcd.phone.sms_tx.timeout = 0;
  atcd.phone.sms_tx.next = NULL;
}
//------------------------------------------------------------------------------
void atcd_phone_set_callback(uint8_t enable_events, void (*callback)(uint8_t, char const *))
{
	atcd.phone.cb_events = enable_events;
	atcd.phone.callback = callback;
}
//------------------------------------------------------------------------------
void atcd_sms_set_callback(uint8_t doesNotUnderstand, void (*sms_callback)(uint8_t event, const atcd_sms_t *sms))
{
	atcd.phone.sms.cb_events = doesNotUnderstand;
	atcd.phone.sms.callback = sms_callback;
}
//------------------------------------------------------------------------------
void atcd_smstx_set_callback(uint8_t doesNotUnderstand, void (*sms_callback)(uint8_t event, const atcd_sms_t *sms))
{
  atcd.phone.sms_tx.cb_events = doesNotUnderstand;
  atcd.phone.sms_tx.callback = sms_callback;
}
//------------------------------------------------------------------------------
static void call_finished(char const *info)
{
  //uint8_t notify=(atcd.phone.state!=ATCD_PHONE_STATE_IDLE || atcd.phone.state_call_in!=0);
  atcd.phone.state = ATCD_PHONE_STATE_IDLE;
  atcd.phone.ring_cnt = 0;
  atcd.phone.number[0] = 0;
  atcd.phone.numbertype = -1;

  atcd.phone.state_call_in=0; //TODO: to by se melo udelat poradne ale jaxi se opet specha
  //radsi vic nez min; ted kdyz +CLCC: ...6 dela jen CALL tak by stejne duplicity nemely byt   if (notify)
  if(atcd.phone.callback != NULL && (atcd.phone.cb_events & ATCD_PHONE_EV_CALL_END) != 0) atcd.phone.callback(ATCD_PHONE_EV_CALL_END, info);
}
//------------------------------------------------------------------------------
void atcd_phone_proc()                    //phone processing
{

}
//------------------------------------------------------------------------------
uint8_t atcd_phone_asc_msg()
{
  uint8_t val;
  char *p, *np, *endl;

  if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "RING\r\n", strlen("RING\r\n")) == 0)
  {
    ATCD_DBG_PHONE_RING_DET

    if(atcd.phone.state != ATCD_PHONE_STATE_RING)
    {
      if(atcd.phone.state != ATCD_PHONE_STATE_RING_WA && atcd.phone.state != ATCD_PHONE_STATE_HANG_W)
      {
        atcd.phone.state = ATCD_PHONE_STATE_RING;
        atcd.phone.ring_cnt = 0;
        atcd.phone.number[0] = '\0';
        atcd.phone.numbertype = -1;
      }
    }

    atcd.phone.ring_cnt++;

    atcd.parser.buff_pos = atcd.parser.line_pos;
    if(atcd.phone.callback != NULL && (atcd.phone.cb_events & ATCD_PHONE_EV_RING) != 0) atcd.phone.callback(ATCD_PHONE_EV_RING, NULL);
    return 1;
  }
  
  if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "+CMT:", strlen("+CMT:")) == 0)
  {
    ATCD_DBG_PHONE_SMS_DET
    // Bude nasledovat SMS - pocet znaku je uveden na konci...

    p    = atcd.parser.buff + atcd.parser.line_pos + strlen("+CMT:");
    endl = atcd.parser.buff + atcd.parser.buff_pos;

    //Sender phone number str start
    np = (char*)memchr(p, '"', endl - p);
    if(np == NULL) goto skip_proc;
    p = np + 1;

    //Sender phone number str end
    np = (char*)memchr(p, '"', endl - p);
    if(np == NULL) goto skip_proc;
    memcpy(atcd.phone.sms.sender, p, np - p);
    atcd.phone.sms.sender[np - p] = 0;
    p = np + 1;

    //Sender phone end
    np = (char*)memchr(p, ',', endl - p);
    if(np == NULL) goto skip_proc;
    p = np + 1;

    //? end
    np = (char*)memchr(p, ',', endl - p);
    if(np == NULL) goto skip_proc;
    p = np + 1;

    //Date time str start
    np = (char*)memchr(p, '"', endl - p);
    if(np == NULL) goto skip_proc;
    p = np + 1;

    //Date time str end
    np = (char*)memchr(p, '"', endl - p);
    if(np == NULL) goto skip_proc;
    memcpy(atcd.phone.sms.datetime, p, np - p);
    atcd.phone.sms.datetime[np - p] = 0;
    p = np + 1;

    //Date time end
    np = (char*)memchr(p, ',', endl - p);
    if(np == NULL) goto skip_proc;
    p = np + 1;

    //? end
    np = (char*)memchr(p, ',', endl - p);
    if(np == NULL) goto skip_proc;
    p = np + 1;

    //? end
    np = (char*)memchr(p, ',', endl - p);
    if(np == NULL) goto skip_proc;
    p = np + 1;

    //? end
    np = (char*)memchr(p, ',', endl - p);
    if(np == NULL) goto skip_proc;
    p = np + 1;

    //?end
    np = (char*)memchr(p, ',', endl - p);
    if(np == NULL) goto skip_proc;
    p = np + 1;

    //DST number end
    np = (char*)memchr(p, ',', endl - p);
    if(np == NULL) goto skip_proc;
    p = np + 1;

    //? end
    np = (char*)memchr(p, ',', endl - p);
    if(np == NULL) goto skip_proc;
    p = np + 1;

    //length

    atcd.phone.sms.len = atoi(p);

    atcd.parser.mode = ATCD_P_MODE_SMS;
    atcd.parser.timer = atcd_get_ms();

    //atcd.phone.state = ATCD_PHONE_STATE_REG_ROAM;
    atcd.parser.buff_pos = atcd.parser.line_pos;
    if(atcd.phone.callback != NULL && (atcd.phone.cb_events & ATCD_PHONE_EV_SMS_IN) != 0) atcd.phone.callback(ATCD_PHONE_EV_SMS_IN, NULL);
    return 1;

  skip_proc:
    ATCD_DBG_PHONE_SMS_DET_ERR
    atcd.parser.buff_pos = atcd.parser.line_pos;
    return 1;
  }

  if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "+CIEV: \"CALL\",", strlen("+CIEV: \"CALL\",")) == 0)
  {
    ATCD_DBG_PHONE_CALL_DET
    // stav je uveden na konci

    val = (uint8_t)atoi(atcd.parser.buff + atcd.parser.buff_pos - ATCD_RX_NL_LEN - 1);
    atcd.parser.buff_pos = atcd.parser.line_pos;

    if(val == 1)
    {
      atcd.phone.state = ATCD_PHONE_STATE_CALL;
      if(atcd.phone.callback != NULL && (atcd.phone.cb_events & ATCD_PHONE_EV_CALL) != 0) atcd.phone.callback(ATCD_PHONE_EV_CALL, atcd.parser.buff + atcd.parser.line_pos);
    }
    else
    {
      call_finished(atcd.parser.buff + atcd.parser.line_pos);
    }
    return 1;
  }

  if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "+CLCC: ", 7/*strlen("+CLCC: ")*/) == 0)
  {
    ATCD_DBG_PHONE_CALL_DET
    //<id>,<dir>,<stat>,<mode>,<mpty>[,<number>,<type>,<alphaID>]<CR><LF>
    //id 1..7 takze jen 1 znak
    //dir 0..mobile orig, 1..mobile term
    //stat 0..active 1..held 2..dialing (MO) 3..alerting (MO) 4..incoming (MT) 5..waiting (MT) 6..disconnect
    //mode 0..voice 1..data 2..fax
    int id=atcd.parser.buff[atcd.parser.line_pos+7]-'0';
    if ((id>=1) && (id<=7))
    {
      uint8_t prevcallout=atcd.phone.state_call_out;
      uint8_t prevcallin=atcd.phone.state_call_in;
      int dir=atcd.parser.buff[atcd.parser.line_pos+9]-'0';
      int stat=atcd.parser.buff[atcd.parser.line_pos+11]-'0';
      if ((dir==0))
      {
        if (stat==0)
          atcd.phone.state_call_out|= 1<<(id-1);
        else
          atcd.phone.state_call_out&=~ (1<<(id-1));
      }
      else
      { //incoming +CLCC: 1,1,6,0,0,"+420777262425",145,""
        if (stat==6) //hanged up
        {
          atcd.phone.state_call_in&=~ (1<<(id-1));
          //if caller cancelled -> will be no "NO CARRIER"
          if (atcd.phone.state==ATCD_PHONE_STATE_RING || atcd.phone.state==ATCD_PHONE_STATE_RING_WA)
          {
            //ideally this would be done after callback(ATCD_PHONE_EV_CALL)
            call_finished(atcd.parser.buff + atcd.parser.line_pos);
          };
          //else notify using CALL and wait for NO CARRIER
        }
        else if (stat==4 || stat==5 || stat==0)
        {
          atcd.phone.state_call_in|= (1<<(id-1));
        }
      }
      if (prevcallout!=atcd.phone.state_call_out)
      {
        char tmps[30];
        snprintf(tmps, sizeof(tmps), "%02x->%02x", prevcallout, atcd.phone.state_call_out);
        atcd_dbg_inf2("call_out", tmps);
        //TODO: nejlepsi by bylo if (atcd.phone.state_call_out) atcd.phone.state = ATCD_PHONE_STATE_CALL;
        //ale muzu si tim rozbit inccall a nemam cas to prozkouset
        //takze necham at se to samo opravi v +CPAS: 4
      };
      if (prevcallin!=atcd.phone.state_call_in)
      {
        char tmps[30];
        snprintf(tmps, sizeof(tmps), "%02x->%02x", prevcallin, atcd.phone.state_call_in);
        atcd_dbg_inf2("call_in", tmps);
        if (atcd.phone.state_call_in&~prevcallin) //nektery bit 0->1
        {
          if (atcd.phone.state==ATCD_PHONE_STATE_IDLE || atcd.phone.state==ATCD_PHONE_STATE_RING)
            atcd.phone.state = ATCD_PHONE_STATE_RING;
          else
            atcd_dbg_err("@+CLCC", "state!=idle");
        }
      };
      if(atcd.phone.callback != NULL && (atcd.phone.cb_events & ATCD_PHONE_EV_CALL) != 0) atcd.phone.callback(ATCD_PHONE_EV_CALL, atcd.parser.buff + atcd.parser.line_pos);
    };

    atcd.parser.buff_pos = atcd.parser.line_pos; //proc asi vracim 1
    return 1;
  }

  if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "+CLIP:", strlen("+CLIP:")) == 0)
  { //+CLIP: "+420777262425",145,"",0,"",0
    ATCD_DBG_PHONE_CALL_N_DET

    p    = atcd.parser.buff + atcd.parser.line_pos + strlen("+CLIP:");
    endl = atcd.parser.buff + atcd.parser.buff_pos;

    //Phone number str start
    np = (char*)memchr(p, '"', endl - p);
    if(np == NULL) goto skip_proc2;
    p = np + 1;

    //Phone number str end
    np = (char*)memchr(p, '"', endl - p);
    if(np == NULL) goto skip_proc2;
    memcpy(atcd.phone.number, p, np - p);
    atcd.phone.number[np - p] = 0;
    p = np + 1;

    if (*p==',')
      atcd.phone.numbertype=atoi(p+1);

    atcd.parser.buff_pos = atcd.parser.line_pos;
    return 1;

  skip_proc2:
    //odpoved na AT+CLIP? je +CLIP: 1,1 coz neobsahuje uvozovky takze to jde sem, pustim to dal
    if (*p==' ') p++;
    if (atcd.parser.buff + atcd.parser.buff_pos - p==5 &&
        (p[0]=='0' || p[0]=='1') &&
        p[1]==',' &&
        (p[2]=='0' || p[2]=='1' || p[2]=='2'))
      return 0;
    ATCD_DBG_PHONE_CALL_N_DET_E
    atcd.parser.buff_pos = atcd.parser.line_pos;
    return 1;
  }

  if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "BUSY\r\n", strlen("BUSY\r\n")) == 0)
  {
    ATCD_DBG_PHONE_BUSY_DET

    //neulozit nekam zmeskane cislo?
    //neinkrementovat pocet zmeskanych hovoru?

    atcd.parser.buff_pos = atcd.parser.line_pos;
    call_finished(atcd.parser.buff + atcd.parser.line_pos);
    return 1;
  }

  if(strncmp(atcd.parser.buff + atcd.parser.line_pos, "NO CARRIER\r\n", strlen("NO CARRIER\r\n")) == 0)
  {
    ATCD_DBG_PHONE_NO_CAR_DET

    //neindikovat odmitnuty hovor?
    atcd.parser.buff_pos = atcd.parser.line_pos;
    call_finished(atcd.parser.buff + atcd.parser.line_pos);
    return 1;
  }

  return 0;
}
//------------------------------------------------------------------------------
void atcd_phone_call_answer()
{
  if(atcd.phone.state == ATCD_PHONE_STATE_RING) atcd.phone.state = ATCD_PHONE_STATE_RING_WA;
}
//------------------------------------------------------------------------------
void atcd_phone_call(const char *number)
{
  if(atcd.phone.state == ATCD_PHONE_STATE_IDLE)
  {
    strncpy(atcd.phone.number, number, sizeof(atcd.phone.number));
    atcd.phone.number[sizeof(atcd.phone.number)-1] = 0;
    atcd.phone.state = ATCD_PHONE_STATE_DIAL_W;
  }
}
//------------------------------------------------------------------------------
void atcd_phone_call_hang_up()
{
  if(atcd.phone.state != ATCD_PHONE_STATE_IDLE) atcd.phone.state = ATCD_PHONE_STATE_HANG_W;
  else if(atcd.phone.state != ATCD_PHONE_STATE_DIAL_W) atcd.phone.state = ATCD_PHONE_STATE_IDLE; //ale tady uz vzdycky IDLE je
}
//------------------------------------------------------------------------------
void atcd_phone_send_sms(char *number, char *msg)  //poslat SMS
{
  atcd.phone.sms_tx.sender = number;

  atcd.phone.sms_tx.message = msg;
  atcd.phone.sms_tx.len = strlen(msg);

//#pragma GCC diagnostic push
//#pragma GCC diagnostic warning "-Wenum-conversion" //compiler neumi
//#pragma GCC diagnostic warning "-Wextra" //nefunguje
  atcd.phone.sms_tx.state = ATCD_PHONE_SMS_STATE_SEND_W;
//#pragma GCC diagnostic pop

  atcd.phone.sms_tx.result = 0;
}
//------------------------------------------------------------------------------
uint8_t atcd_phone_sms_proc(char ch)
{
  // If parser in SMS receiving mode
  if(atcd.parser.mode == ATCD_P_MODE_SMS)
  {
    // Pokud je v bufferu misto
    if(atcd.parser.buff_pos - atcd.parser.line_pos < sizeof(atcd.phone.sms_message_buff)-1) //max [159]
    {
      // Zapise prijaty byte do bufferu
      atcd.phone.sms.message[atcd.parser.buff_pos - atcd.parser.line_pos] = ch;
    }
    else
    {
      ATCD_DBG_PHONE_SMS_BUFF_E
      //conn->cb_events |=  ATCD_CONN_EV_OVERRUN;
    }

    atcd.parser.buff_pos++;

    // Pokud jsme dosahli konce bloku s textem SMS
    //if(atcd.buff_pos >= atcd.parser.ipd_len)
    if(atcd.parser.buff_pos - atcd.parser.line_pos >= atcd.phone.sms.len)
    {
      ATCD_DBG_PHONE_SMS_END

      atcd.parser.mode = ATCD_P_MODE_IDLE;
      atcd.parser.timer = atcd_get_ms();

      atcd.parser.buff_pos = 0;
      atcd.parser.line_pos = 0;

      atcd.phone.sms.message[atcd.phone.sms.len] = 0;

      atcd.phone.sms.cb_events |= ATCD_SMS_EV_SMS_IN;
      if(atcd.phone.sms.callback != NULL) atcd.phone.sms.callback(ATCD_SMS_EV_SMS_IN, &atcd.phone.sms);
    }

    return 1;
  }

  return 0;
}
//------------------------------------------------------------------------------
atcd_phone_state_t atcd_phone_state()
{
  return atcd.phone.state;
}
//------------------------------------------------------------------------------
atcd_phone_t *atcd_phone_fullstate()
{
  return &atcd.phone;
}
//------------------------------------------------------------------------------
uint16_t atcd_phone_ring_cnt()
{
  return atcd.phone.ring_cnt;
}
//------------------------------------------------------------------------------
const char *atcd_phone_ring_number(int *ntyp) //nikdy nevraci NULL ale muze ""
{
  if (ntyp)
    *ntyp=atcd.phone.numbertype;
  return atcd.phone.number;
}
//------------------------------------------------------------------------------
uint8_t atcd_phone_are_phones_equal(const char *p1, int t1, const char *p2, int t2)
{
  //00421X,129 == +421X,145 == 0X,129 = X,129    typ==0 <- nevim
  /*bool allgood=true;
  static const char *ph1[4]={"00421777777448", "+421777777448", "0777777448", "777777448"}
  static const int pt1[4]=  {129,              145,             129,          129};
  for (int i1=0; i1<4; i1++)
    for (int i2=0; i2<4; i2++)
      if (!atcd_phone_are_phones_equal(ph1[i1], pt1[i1]129, ph1[i2], 0))
        allgood=false;
  assert(allgood);*/

  const char *p1a, *p2a;
  int l1, l2, ptr12, l12;

  //sice to dopisuji az 120830 ale vypada to ze je to presne to co chci
  if (*p2==0)
    return 0;; //jinak se kazde cislo shoduje s ujcem ktery ma prazdny phone a to nechci

  p1a=p1; p2a=p2;
  if ((p1a[0]=='0') && (p1a[1]=='0'))
   {
    p1a+=2;
    if ((t1==129) || (t2==0))
      t1=145;
    else
      p1a=p1;//error:='p1=00xxx,!129';
   }
  else if (t1==145)                                   //+420
   {
    if (p1a[0]=='+')
      p1a++; //"420xxx" i "+420xxx"
   }
  else
  {
    if ((t1==0) && (p1a[0]=='+'))
    {
      p1a++;
      t1=145;
    }//"420xxx" i "+420xxx"
    else if (p1a[0]=='0') //ve Svedsku chodi cisla (prozvoneni) jako 0734275077,129 a ne +46734275077,145
      p1a++;
  }
  if ((p2a[0]=='0') && (p2a[1]=='0'))
   {
    p2a+=2;
    if ((t2==129) || (t2==0))
      t2=145;
    else
      p2a=p2;//  error:='p2=00xxx,!129';
   }
  else if (t2==145)
   {
    if (p2a[0]=='+') //"420xxx" i "+420xxx"
      p2a++;
   }
  else
  {
    if ((t2==0) && (p2a[0]=='+'))
    {
      p2a++;
      t2=145;
    }//"420xxx" i "+420xxx"
    else if (p2a[0]=='0') //ve Svedsku chodi cisla (prozvoneni) jako 0734275077,129 a ne +46734275077,145
      p2a++;
  }
//co udela p1=0734275077 p2=+46734275077   at+test=34,"0734275077",129,"+46734275077",145
  l1=(int)strlen(p1);                  //13        10
  l2=(int)strlen(p2);                  //0         12
  ptr12=(int)(p1a-p1)-(int)(p2a-p2);      //1-0=1     1-1
  l12=l1-l2;                              //13        -2

  if ((t1==145) && (t2!=145))                       //f && f
   {
    if (ptr12-l12>0)                      //1-13
      return 0;;
   }
  else if ((t1!=145) && (t2==145))                  //t && t
   {
    if (ptr12-l12<0)                                //0--2<0 = false
      return 0;;
   }
  else
   {
    if (l12!=ptr12) //zbyva stejne
      return 0;;
   }
  if (l12<0)
   {
    p2a+=(ptr12-l12);                               //+=2
   }
  else
   {
    p1a+=l12-ptr12;
   }
  return (strcmp(p1a, p2a)==0);
}
