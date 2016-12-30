//--------------------------------------------------------------------------------
// project : monkeyDAB
// module  : monkeyDAB_UI_key
//           User interface via serial port (key-based)
// author  : Thomas Euler
// history : 2016-10-08, created
//
//--------------------------------------------------------------------------------
bool handleKeyInput(char ch)
{
  int   iLED, val;
  float C;

  switch(ch) {
    case '?':
      Serial.println(F("---"));
      Serial.println(F("? help"));
      Serial.println(F("n  next program"));      
      Serial.println(F("p  previous program"));      
      Serial.println(F("+  increase volume"));
      Serial.println(F("-  decrease volume"));      
      Serial.println(F("m  toggle mute/unmute"));    
      Serial.println(F("i  update program info"));  
      Serial.println(F("t  toggle amplifier power"));  
      Serial.println(F("r  switch to radio (DAB)"));
      Serial.println(F("x  switch to AUX"));
      Serial.println(F("e  change equalizer mode"));
      Serial.println();
      Serial.println(F("a  start autosearch for DAB programs"));            
      Serial.println(F("c  enable RTC clock sync"));  
      Serial.println(F("0  adjust info LED's hue and saturation"));  
      Serial.println(F("d  calibrate program dial"));
      Serial.println();
      Serial.println(F("!  write current configuration to EEPROM"));  
      Serial.println(F("---"));
      break;
  
    case 'n':
      if(Radio.progDAB < monkeyRadio.getDABProgramCount()-1) {
        iPrevProg = Radio.progDAB;
        dirProg   = +1;
        Radio.progDAB++;
        Radio.isProgChanged = true;
      }  
      break;

    case 'p':
      if(Radio.progDAB > 0) {
        iPrevProg = Radio.progDAB;
        dirProg   = -1;
        Radio.progDAB--;
        Radio.isProgChanged = true;
      }  
      break;

    case 'm':
      if(Radio.isMute) {
        monkeyRadio.hardUnMute();
        Radio.isMute = false;
      }
      else {
        monkeyRadio.hardMute();
        Radio.isMute = true;
      }
      break;

    case '+':
      monkeyRadio.volumePlus();
      Radio.volume = monkeyRadio.getVolume();
      break;  

    case '-':
      monkeyRadio.volumeMinus();
      Radio.volume = monkeyRadio.getVolume();
      break;  

    case 'a':
      setLEDStatus_Info(ID_COLOR_AUTOSEEK, -1, true);
      monkeyRadio.DABAutoSearch(DAB_MIN_PROG, DAB_MAX_PROG);
      if(monkeyRadio.getDABProgramCount() > 0) {
        Radio.isProgChanged = true;
        Radio.progDAB       = 0;
        Radio.volume        = Conf.volume;
        monkeyRadio.setVolume(Radio.volume);
      }
      break;

    case 'i':
      updateStateInfo();
      printInfo();
      break;

    case '!':
      writeConfig();
      break;  

    case 'c':
      monkeyRadio.syncRTC(true);
      break;  

    case 't':
      if(isAmpPowerOn)
        switchAmpPower(false);
      else  
        switchAmpPower(true);
      break;

    case 'r':
      switchRelayToSource(RELAY_RADIO);
    //FSM_state = FSM_DAB_PLAY;
      break;

    case 'x':
      switchRelayToSource(RELAY_AUX);
    //FSM_state = FSM_AUX;
      break;

    case 'e':
      if(BBE_EQ.EQMode < LAST_EQ_MODE)
        BBE_EQ.EQMode += 1;
      else {
        BBE_EQ.EQMode  = 0;
      }
      monkeyRadio.setBBEEQ(BBE_EQ);
      Serial.print("EQ mode is ");
      Serial.println(EQModeStr[BBE_EQ.EQMode]);
      break;

    case 'd':
      Serial.println(F("Move program dial to the very LEFT and press a key ..."));
      while(!Serial.available())
        delay(20);
      Serial.read();  
      Conf.minCap = readDialCapacitanceAvg(Controls[DIAL_PROGRAM].pin, Controls[DIAL_PROGRAM].pin2, 10);

      Serial.println(F("Move program dial to the very RIGHT and press a key ..."));
      while(!Serial.available())
        delay(20);
      Serial.read();
      Conf.maxCap = readDialCapacitanceAvg(Controls[DIAL_PROGRAM].pin, Controls[DIAL_PROGRAM].pin2, 10);

      Serial.print(F("Capacitance = "));
      Serial.print(Conf.minCap, 1);
      Serial.print(F(" .. "));
      Serial.print(Conf.maxCap, 1);

      C = ((float)Conf.maxCap -(float)Conf.minCap) /50.0;
      Conf.maxCap -= C;
      Serial.print(F(", with safety = "));
      Serial.print(Conf.minCap, 1);
      Serial.print(F(" .. "));
      Serial.print(Conf.maxCap, 1);
      Serial.println();
      
      updateProgramDialParams();
      writeConfig();
      break;

    case '0':
      iLED = int(ch) -48;
      Serial.print("Adjust LED #");
      Serial.print(iLED, DEC);
      Serial.println(", exit by pressing 'x'");
      
      while(ch != 'x') {
        if(Serial.available()) {
          ch = Serial.read();
          switch(ch) {
            case 'q':
              if(Conf.h[iLED] <= 250)
                Conf.h[iLED] += 5;
              break;
            case 'a':
              if(Conf.h[iLED] >= 5)
                Conf.h[iLED] -= 5;
              break;
            case 'w':
              if(Conf.s[iLED] <= 250)
                Conf.s[iLED] += 5;
              break;
            case 's':
              if(Conf.s[iLED] >= 5)
                Conf.s[iLED] -= 5;
              break;
          }
          Serial.print("h=");
          Serial.print(Conf.h[iLED], DEC);
          Serial.print(", s=");
          Serial.println(Conf.s[iLED], DEC);

          _setLEDColorHSV(iLED, Conf.h[iLED], Conf.s[iLED], Conf.v[iLED], true);
        }
      }
      Serial.println("... done");
      break;
      
    default:
      return false;
  }
  return true;
}

//--------------------------------------------------------------------------------
void printInfo() 
{
  switch(Radio.state) {
    case PLAY_STATUS_PLAY:
      Serial.print("Playing ");
      if(Radio.mode == STREAM_MODE_DAB) {
        Serial.print(F("DAB stream #"));
        Serial.print(Radio.progDAB, DEC);
        Serial.print(F(", "));
        Serial.print(DABFreqs[Radio.progDAB].freq, 3);
        Serial.print(F(" MHz, "));
        Serial.print(DABFreqs[Radio.progDAB].name);
        Serial.println();

        if(Radio.signalStrength > 0) {
          Serial.print(F("strength="));
          Serial.print(Radio.signalStrength, DEC);
          Serial.print(F(", bit error="));
          Serial.print(Radio.bitError, DEC);
        }
        else {
          Serial.print(F("strength n/a"));
        }
        Serial.print(F(", stereo="));
        Serial.print(Radio.stereo, DEC);
        Serial.print(F(", play mode="));
        Serial.print(Radio.playMode, DEC);
        Serial.print(F(", data rate="));
        Serial.print(Radio.dataRate, DEC);
        Serial.print(F(" kbps"));
        Serial.println();

        Serial.print(F("DAB signal quality="));
        Serial.print(Radio.qualityDAB, DEC);
        Serial.print(F(" (0..100)"));
        Serial.println();
        
        if(Radio.progDABType < 0) {
          Serial.print(F("program type n/a"));
        }
        else {
          Serial.print(F("program type="));
          Serial.print(Radio.progDABType, DEC);
          Serial.print(F(" ("));
          Serial.print(ProgTypeStr[Radio.progDABType]);
          Serial.print(F(")"));
        }
        if(strlen(Radio.progName) == 0) {
          Serial.print(F(", program name n/a"));
        }
        else {
          Serial.print(F(", program name="));
          Serial.print(Radio.progName);
        }
        Serial.println();
        Serial.print(F("DAB service type="));
        Serial.print(DABServiceTypeStr[Radio.DABService]);
        Serial.print(F("("));
        Serial.print(Radio.DABService, DEC);
        Serial.print(F(")"));
        Serial.println();

        if(RTC_time.isOk) {
          Serial.println(Radio.RTC_timeDateStr);
        }
      }
      break;
  }    
}

//--------------------------------------------------------------------------------
