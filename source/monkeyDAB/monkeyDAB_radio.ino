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
