//--------------------------------------------------------------------------------
// project : monkeyDAB
// module  : monkeyDAB_interface
//           hardware interface, such as buttons and dials
// author  : Thomas Euler
// history : 2016-11-03, created
//
//--------------------------------------------------------------------------------
void initControls()
{
  SER_DEBUG.print(F("Initializing controls ..."));

  // Set all analog inputs by default as floating to prevent them to interfer
  // with the capacitance measurements
  //
  for(byte j=A0; j<A7; j++) {
    pinMode(j, INPUT);
  }
  pinMode(PIN_DIAL_VC, OUTPUT);
  digitalWrite(PIN_DIAL_VC, LOW);

  // Initialize all controls and get a first readout
  //
  for(byte j=0; j<CONTROL_COUNT; j++) {
    switch(Controls[j].type) {
      case CONTROL_BUTTON :
        pinMode(Controls[j].pin, INPUT_PULLUP);
        delay(10);
        Controls[j].rawState   = digitalRead(Controls[j].pin);
        Controls[j].lastState  = Controls[j].rawState;
        Controls[j].state      = (Controls[j].rawState == Controls[j].highState);
        Controls[j].val        = int(Controls[j].state);
        Controls[j].tLast      = millis();
        Controls[j].isChanged  = true;
        break;

      case CONTROL_DIAL_C :
        pinMode(Controls[j].pin2, OUTPUT);
        pinMode(Controls[j].pin, OUTPUT);
        delay(10);
        Controls[j].cap        = readDialCapacitance(Controls[j].pin, Controls[j].pin2);
        Controls[j].readingVal = int((Controls[j].cap -(float)Conf.minCap) /dCapBin);
        Controls[j].readingVal = constrain(Controls[j].readingVal, 0, nCapBins-1);
        Controls[j].val        = Controls[j].readingVal;
        Controls[j].tLast      = millis();
        Controls[j].isChanged  = true;
        break;

      case CONTROL_DIAL_R :
        Controls[j].readingVal = readDialVoltage(Controls[j].pin);
        Controls[j].val        = Controls[j].readingVal;
        Controls[j].tLast      = millis();
        Controls[j].isChanged  = true;
        break;
    }
  }
  // Handle interactions between hardware controls
  //
  correctControlStates();

  SER_DEBUG.println(F("  done"));
}

//--------------------------------------------------------------------------------
void updateControlStates()
{
  // Check for robust changes of the controls' state
  // (debounce buttons and denoise dials)
  //
  for(int j=0; j<CONTROL_COUNT; j++) {
    Controls[j].isChanged = false;

    switch(Controls[j].type) {
      case CONTROL_BUTTON :
        // A button
        //
        Controls[j].readingState = digitalRead(Controls[j].pin);
        if(Controls[j].readingState != Controls[j].lastState) {
          Controls[j].tLast = millis();
        }
        if((millis() -Controls[j].tLast) > BUTTON_DEBOUNCE_MS) {
          Controls[j].isChanged  = (Controls[j].readingState != Controls[j].rawState);
          if(Controls[j].isChanged) {
            Controls[j].rawState = Controls[j].readingState;
            Controls[j].state    = (Controls[j].rawState == Controls[j].highState);
            Controls[j].val      = int(Controls[j].state);
          }
        }
        Controls[j].lastState = Controls[j].readingState;
        break;

      case CONTROL_DIAL_C :
        // A dial that uses a tunable capacitance
        //
        Controls[j].cap          = readDialCapacitance(Controls[j].pin, Controls[j].pin2);
      //Serial.println(Controls[j].cap);
        Controls[j].readingVal   = (Controls[j].cap -(float)Conf.minCap) /dCapBin;
        Controls[j].readingVal   = int(constrain(Controls[j].readingVal, 0, nCapBins-1));
        if(Controls[j].readingVal != Controls[j].lastVal) {
          Controls[j].tLast = millis();
        }
        if((millis() -Controls[j].tLast) > DIAL_C_DENOISE_MS) {
          Controls[j].isChanged = (Controls[j].readingVal != Controls[j].val);
          if(Controls[j].isChanged) {
            Controls[j].val = Controls[j].readingVal;
          }
        }
        Controls[j].lastVal = Controls[j].readingVal;
        break;

      case CONTROL_DIAL_R :
        // A dial that uses a tunable resistor (poti)
        //
        Controls[j].readingVal   = readDialVoltage(Controls[j].pin) /DIAL_R_DIV;
        if(Controls[j].readingVal != Controls[j].lastVal) {
          Controls[j].tLast = millis();
        }
        if((millis() -Controls[j].tLast) > DIAL_R_DENOISE_MS) {
          Controls[j].isChanged = (Controls[j].readingVal != Controls[j].val);
          if(Controls[j].isChanged) {
            Controls[j].val = Controls[j].readingVal;
          }
        }
        Controls[j].lastVal = Controls[j].readingVal;
        break;
    }
  }
  // Handle interactions between hardware controls
  //
  correctControlStates();

  // Determine if at least one control has changed and set global variable
  //
  isControlChanged = false;
  for(int j=0; j<CONTROL_COUNT; j++) {
    if(Controls[j].isChanged)
      isControlChanged = true;
  }
  if(isControlChanged) {
    for(int j=0; j<CONTROL_COUNT; j++) {
      SER_DEBUG.print(Controls[j].name);
      SER_DEBUG.print("=");
      SER_DEBUG.print(Controls[j].val, DEC);
      SER_DEBUG.print(" ");
    }
    SER_DEBUG.println();
  }
}

//--------------------------------------------------------------------------------
void updateProgramDialParams()
{
  // Calculate width of capacitance bins (of the program dial) for the
  // available number of programs
  //
  nCapBins = getAudioDABProgramCount();
  dCapBin  = (float)(Conf.maxCap -Conf.minCap)/nCapBins;

  SER_DEBUG.print(F("Program dial capacitance = "));
  SER_DEBUG.print(Conf.minCap, DEC);
  SER_DEBUG.print(F(" .. "));
  SER_DEBUG.print(Conf.maxCap, DEC);
  SER_DEBUG.print(F(" -> "));
  SER_DEBUG.print(dCapBin, 3);
  SER_DEBUG.print(F(" wide program bins"));
  SER_DEBUG.println();
}

//--------------------------------------------------------------------------------
void correctControlStates()
{
  // Handle interactions between hardware controls, i.e. the UKW button state
  // cannot be determined independet of the state of the other buttons
  //
  bool state;

  if(Controls[BUTTON_TA].isChanged || Controls[BUTTON_MW].isChanged) {
    state = (!Controls[BUTTON_TA].state && !Controls[BUTTON_MW].state);
    Controls[BUTTON_UKW].state     = state;
    Controls[BUTTON_UKW].lastState = state;
    Controls[BUTTON_UKW].val       = int(state);
    Controls[BUTTON_UKW].isChanged = true;
  }
}

//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
void updateFSM()
{
  int j;

  FSM_lastState = FSM_state;
  FSM_state     = FSM_UNDEFINED;

  if(Controls[BUTTON_TA].state) {
    // Playing from AUX
    //
    FSM_state = FSM_AUX;
    switchRelayToSource(RELAY_AUX);
  }
  else
  if(Controls[BUTTON_MW].state) {
    if(!Controls[BUTTON_SPRACHE].state) {
      // Play current DAB program
      //
      FSM_state = FSM_DAB_PLAY;
      switchRelayToSource(RELAY_RADIO);
    }
    else {
      // Initiate autosearch ...
      //
      FSM_state = FSM_DAB_AUTOSEARCH;
      if(runDABAutoSearch())
        FSM_state = FSM_DAB_PLAY;
      else {
        FSM_state = FSM_DAB_AUTOSEARCH_FAILED;
        // ...
      }
    }
  }
  else {
    // Not yet implemented
    //
    FSM_state = FSM_NOT_IMPLEMENTED;
  }

  // Check if program dial has changed and alter program
  // accordingly when in DAB play mode
  //
  if(Controls[DIAL_PROGRAM].isChanged) {
//if((FSM_state == FSM_DAB_PLAY) && Controls[DIAL_PROGRAM].isChanged) {
    iPrevProg = Radio.progDAB;
    Radio.progDAB = Controls[DIAL_PROGRAM].val;
    Radio.isProgChanged = true;
  }

  // If changed, print state name to log
  //
  if(FSM_lastState != FSM_state) {
    SER_DEBUG.println(FSMStr[FSM_state]);
  }
}

//--------------------------------------------------------------------------------
