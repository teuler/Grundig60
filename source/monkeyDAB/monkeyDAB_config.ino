//--------------------------------------------------------------------------------
// project : monkeyDAB
// module  : monkeyDAB_config
//           read and write configuration to EPROM
// author  : Thomas Euler
// history : 2016-10-08, created
//
//--------------------------------------------------------------------------------
void readConfig ()
{
  SER_DEBUG.print(F("Reading configuration from EEPROM ..."));
  
  EEPROM.get(EEPROM_START, Conf);
  if(Conf.isNew || EEPROM_TO_DEFAULT) {
    Conf.progDAB = 0;
    Conf.volume  = 10;
    Conf.isNew   = false;
    Conf.minCap  = 20;
    Conf.maxCap  = 260;
    // ...
    Conf.h[ID_COLOR_PLAYING]  = 50; 
    Conf.s[ID_COLOR_PLAYING]  = 178;
    Conf.v[ID_COLOR_PLAYING]  = 200;

    Conf.h[ID_COLOR_TUNING]   = 25; 
    Conf.s[ID_COLOR_TUNING]   = 178;
    Conf.v[ID_COLOR_TUNING]   = 200;

    Conf.h[ID_COLOR_STARTING] = 85;   //100
    Conf.s[ID_COLOR_STARTING] = 53;   //178
    Conf.v[ID_COLOR_STARTING] = 200;

    Conf.h[ID_COLOR_ERROR]    = 10; 
    Conf.s[ID_COLOR_ERROR]    = 178;
    Conf.v[ID_COLOR_ERROR]    = 200;

    Conf.h[ID_COLOR_AUTOSEEK] = 195;  //200 
    Conf.s[ID_COLOR_AUTOSEEK] = 98;   //255
    Conf.v[ID_COLOR_AUTOSEEK] = 220;  //250

    Conf.h[ID_COLOR_AUX]      = 85;  
    Conf.s[ID_COLOR_AUX]      = 178; 
    Conf.v[ID_COLOR_AUX]      = 240;
    // ...
    
    EEPROM.put(EEPROM_START, Conf);
    SER_DEBUG.print(F("  set to default and written to EEPROM"));
  }
  Radio.progDAB = Conf.progDAB;
  Radio.volume  = Conf.volume;
  // ...
  SER_DEBUG.println(F("  done"));
}

void writeConfig ()
{
  Conf.progDAB  = Radio.progDAB;
  Conf.volume   = Radio.volume;
  // ...
  EEPROM.put(EEPROM_START, Conf);
  Serial.println(F("Configuration in EEPROM updated"));
}
 
//--------------------------------------------------------------------------------

