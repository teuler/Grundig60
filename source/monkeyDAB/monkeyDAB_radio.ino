//--------------------------------------------------------------------------------
// project : monkeyDAB
// module  : monkeyDAB_radio
//           radio-related functions
// author  : Thomas Euler
// history : 2016-10-08, created
//
//--------------------------------------------------------------------------------
void updateStateInfo()
{
  int v, q;

  switch(FSM_state) {
    case FSM_AUX :
      // ...
      setLEDStatus_Info(ID_COLOR_AUX, -1, true);
      break;

    case FSM_DAB_PLAY:
      monkeyRadio.quiet(true);
      Radio.state = monkeyRadio.getPlayStatus();
      switch(Radio.state) {
        case PLAY_STATUS_PLAY:
          // Update all important radio parameters
          //
          Radio.progDAB        = monkeyRadio.getPlayIndex();
          Radio.signalStrength = monkeyRadio.getSignalStrength(&Radio.bitError);
          Radio.stereo         = monkeyRadio.getStereo();
          Radio.playMode       = monkeyRadio.getPlayMode();
          Radio.dataRate       = monkeyRadio.getDataRate();
          Radio.progDABType    = monkeyRadio.getProgramType(-1);
          if(!monkeyRadio.getProgramName(-1, 1, Radio.progName))
            Radio.progName[0]  = 0;
          if(!monkeyRadio.getProgramName(-1, 0, Radio.progAbbr))
            Radio.progAbbr[0]  = 0;
          Radio.qualityDAB     = monkeyRadio.getDABSignalQuality();
          Radio.DABService     = monkeyRadio.getServCompType(Radio.progDAB);

          // Update time
          //
          monkeyRadio.getRTC(&RTC_time);
          RTCTimeToTimeDateStr(DISPLAY_TIME_WSEC);

          // Update signal quality indicator (info LED)
          //
          Radio.lastQualityDAB = (Radio.lastQualityDAB +Radio.qualityDAB)/2;
          v = map(int(Radio.lastQualityDAB), 0, 100, LED_INFO_MIN, LED_INFO_MAX);
          setLEDStatus_Info(ID_COLOR_PLAYING, v, true);
          break;

        default:
          setLEDStatus_Info(ID_COLOR_ERROR, -1, true);
      }
      monkeyRadio.quiet(false);
      break;
  }
}

//--------------------------------------------------------------------------------
int  getAudioDABProgramCount()
{
  int i, nProg, nAudio, res;

  // Determine the number of DAB programs (DAB, DAB+) w/o the data channels
  //
  monkeyRadio.quiet(true);
  nProg  = monkeyRadio.getDABProgramCount();
  nAudio = 0;
  for(i=0; i<nProg; i+=1) {
    res  = monkeyRadio.getServCompType(i);
    if((res == DAB_SERVICE_DAB) || (res == DAB_SERVICE_DAB_PLUS))
      nAudio += 1;
  }
  monkeyRadio.quiet(false);

  SER_DEBUG.print(nAudio, DEC);
  SER_DEBUG.print(F(" of "));
  SER_DEBUG.print(nProg, DEC);
  SER_DEBUG.println(F(" DAB programs are audio"));

  return nAudio;
}

//--------------------------------------------------------------------------------
bool runDABAutoSearch()
{
  String txt;
  int    j;
  float  C;

  // Give the user the chance to abort the autosearch process by releasing the
  // Sprache key
  //
  flickerLED_Info(ID_COLOR_AUTOSEEK, LED_FLICKER_MIN, LED_FLICKER_MAX,
                  LED_FLICKER_DI, 1000/COUNTDOWN_DIV -50, true);
  for(j=COUNTDOWN_MAX_S*COUNTDOWN_DIV; j>0; j-=1) {
    txt  = TXT_START_IN;
    txt += round(float(j)/COUNTDOWN_DIV);
    txt += TXT_START_IN_PART2;
    displayMsg(TXT_AUTOSEARCH, txt, TXT_OR_REMOVE_SPRACHE, "");
    flickerLED_Info(ID_COLOR_AUTOSEEK, 0, 0, 0, 0, false);
    if(checkSpracheKey() == false) {
      // User aborted the autosearch procedure
      //
      displayMsg(TXT_AUTOSEARCH, TXT_ABORTED, "", "");
      delay(1000);
      return false;
    }
  }
  // Continue ...
  //
  setLEDStatus_Info(ID_COLOR_AUTOSEEK, -1, true);
  displayMsg(TXT_AUTOSEARCH, TXT_IS_STARTING, "", "");
  delay(1000);

  // Ask user to release SPRACHE key ...
  //
  flickerLED_Info(ID_COLOR_AUTOSEEK, LED_FLICKER_MIN, LED_FLICKER_MAX,
                  LED_FLICKER_DI, 1000/COUNTDOWN_DIV -50, true);
  displayMsg(TXT_AUTOSEARCH, TXT_RELEASE_SPRACHE1, TXT_RELEASE_SPRACHE2, "");
  while(checkSpracheKey()) {
    flickerLED_Info(ID_COLOR_AUTOSEEK, 0, 0, 0, 0, false);
  }
  setLEDStatus_Info(ID_COLOR_AUTOSEEK, -1, true);

  // Run autosearch ...
  //
  monkeyRadio.DABAutoSearch(DAB_MIN_PROG, DAB_MAX_PROG, (displayFunc)displayMsg);
  if(monkeyRadio.getDABProgramCount() > 0) {
    // Station(s) found ...
    //
    Radio.isProgChanged = true;
    Radio.progDAB       = 0;
    Radio.volume        = Conf.volume;
    monkeyRadio.setVolume(Radio.volume);

    // Ask user to turn SENSER dial all way to the left and then lock
    // SPRACHE key. Then measure capacitance ...
    //
    flickerLED_Info(ID_COLOR_AUTOSEEK, LED_FLICKER_MIN, LED_FLICKER_MAX,
                    LED_FLICKER_DI, 1000/COUNTDOWN_DIV -50, true);
    displayMsg(TXT_AUTOSEARCH, TXT_TURN_SENDER, TXT_TURN_SENDER_LEFT2,
               TXT_TURN_SENDER_LEFT3);
    while(!checkSpracheKey()) {
      flickerLED_Info(ID_COLOR_AUTOSEEK, 0, 0, 0, 0, false);
    }
    setLEDStatus_Info(ID_COLOR_AUTOSEEK, -1, true);
    Conf.minCap = readDialCapacitanceAvg(Controls[DIAL_PROGRAM].pin,
                                         Controls[DIAL_PROGRAM].pin2, 10);

    // Ask user to turn SENSER dial all way to the right and then release
    // SPRACHE key. Then measure capacitance ...
    //
    flickerLED_Info(ID_COLOR_AUTOSEEK, LED_FLICKER_MIN, LED_FLICKER_MAX,
                    LED_FLICKER_DI, 1000/COUNTDOWN_DIV -50, true);
    displayMsg(TXT_AUTOSEARCH, TXT_TURN_SENDER, TXT_TURN_SENDER_RIGHT2,
               TXT_TURN_SENDER_RIGHT3);
    while(checkSpracheKey()) {
      flickerLED_Info(ID_COLOR_AUTOSEEK, 0, 0, 0, 0, false);
    }
    setLEDStatus_Info(ID_COLOR_AUTOSEEK, -1, true);
    Conf.maxCap = readDialCapacitanceAvg(Controls[DIAL_PROGRAM].pin,
                                         Controls[DIAL_PROGRAM].pin2, 10);

    // Correct maximal capacitance (safety margin)
    //
    C = ((float)Conf.maxCap -(float)Conf.minCap) /50.0;
    Conf.maxCap -= C;

    // Update dial parameters and write new configuration to EEPROM
    //
    updateProgramDialParams();
    writeConfig();

    displayMsg(TXT_AUTOSEARCH, TXT_SEARCH_SUCCESS1, TXT_SEARCH_SUCCESS2, "");
    delay(1000);
    return true;
  }

  displayMsg(TXT_AUTOSEARCH, TXT_SEARCH_ERROR, "", "");
  delay(1000);
  return false;
}

//--------------------------------------------------------------------------------
