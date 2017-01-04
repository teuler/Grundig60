//--------------------------------------------------------------------------------
// project : monkeyDAB
// module  : monkeyDAB
//           DAB radio based on monkey DAB board
// author  : Thomas Euler
// history : 2016-10-08, created
//
//--------------------------------------------------------------------------------
#define  USE_HSV
#include <WS2812.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "monkeyRadio.h"

//--------------------------------------------------------------------------------
// Global definitions
//
#define          SER_HOST           (*SerHostStream)
#define          SER_DEBUG_OUT      (*SerDebugStream)
#define          SER_MONKEY_RADIO   Serial1
#define          SER_DEBUG          Serial
#define          GERMAN_TEXT

//                                  0   -> TX0
//                                  1   -> RX0
#define          PIN_LED            2
#define          PIN_BUTTON_SPRACHE 3
//                                  4   -> DAB SHNT
//                                  5   -> DAB SHDN
#define          PIN_DISPLAY_RST    6
#define          PIN_DIAL_VC        7
//                                  8
//                                  10
#define          PIN_RELAY          11
#define          PIN_AMP_POWER      12
#define          PIN_CAP_IN         A0
#define          PIN_CAP_OUT        A1
//                                  A2
#define          PIN_DIAL_EXTEND    A3
#define          PIN_DIAL_TON       A4
#define          PIN_BUTTON_MW      A5
#define          PIN_BUTTON_UKW     A6
#define          PIN_BUTTON_TA      A7

#define          ID_LED_INFO        0
#define          ID_LED_INFO2       1
#define          LED_COUNT          2
#define          ID_COLOR_PLAYING   0
#define          ID_COLOR_TUNING    1
#define          ID_COLOR_STARTING  2
#define          ID_COLOR_ERROR     3
#define          ID_COLOR_AUTOSEEK  4
#define          ID_COLOR_AUX       5
#define          COLOR_COUNT        6
#define          LED_INFO_MIN       200
#define          LED_INFO_MAX       255
#define          LED_FLICKER_MIN    180
#define          LED_FLICKER_MAX    220
#define          LED_FLICKER_DI     10
#define          LED_FLICKER_DT     40

#define          RELAY_RADIO        LOW
#define          RELAY_AUX          HIGH

#define          EEPROM_START       0
#define          EEPROM_TO_DEFAULT  false

#define          CONTROL_BUTTON     0
#define          CONTROL_DIAL_R     1
#define          CONTROL_DIAL_C     2

#define          BUTTON_SPRACHE     0
#define          BUTTON_TA          1
#define          BUTTON_UKW         2
#define          BUTTON_MW          3
#define          DIAL_PROGRAM       4
#define          DIAL_TON           5
#define          CONTROL_COUNT      6

#define          BUTTON_DEBOUNCE_MS 100L
#define          DIAL_C_DENOISE_MS  5L
#define          DIAL_R_DENOISE_MS  5L
#define          DIAL_R_DIV         10

#define          CAP_IN_TO_GND      24.48
#define          CAP_R_PULLUP       34.8
#define          MAX_ADC_VALUE      1023

#define          RADIO_UPDATE_MS    1000
#define          LOOP_DELAY_MS      10

#define          USE_DISPLAY
#define          DISPLAY_TIME_WSEC  false

//--------------------------------------------------------------------------------
// Finite stae machine definitions
//
#define          FSM_UNDEFINED              0
#define          FSM_OFF                    1
#define          FSM_AUX                    2
#define          FSM_DAB_PLAY               3
#define          FSM_DAB_AUTOSEARCH_START   4
#define          FSM_DAB_AUTOSEARCH_RUNNING 5
#define          FSM_DAB_AUTOSEARCH_DONE    6
#define          FSM_NOT_IMPLEMENTED        7
#define          LAST_FSM_STATE             7

char* FSMStr[LAST_FSM_STATE +1] = {
  #ifdef GERMAN_TEXT
  "Nicht definiert", "Aus",
  "AUX", "DAB",
  "Sendersuchlauf-Start", "Sendersuchlauf ...", "Sendersuchlauf fertig",
  "Nicht implementiert"
  #else
  "undefined", "off",
  "playing AUX", "playing DAB",
  "autosearch start", "autosearch running", "autosearch done",
  "not implemented"
  #endif
  };

//--------------------------------------------------------------------------------
// Global variables
//
HardwareSerial*  SerHostStream;     // port for monkey DAB board
HardwareSerial*  SerDebugStream;    // port for debug messages and key-base UI
WS2812           LEDs(LED_COUNT);   // LED(s)
#ifdef USE_DISPLAY
Adafruit_SSD1306 Display(PIN_DISPLAY_RST);
#endif

typedef struct {
  char           progDAB, progDABType, DABService;
  char           progName[MAX_TEXT_BUFFER], progAbbr[MAX_TEXT_BUFFER];
  char           mode, state, stereo, playMode;
  bool           isProgChanged, isMute;
  int            bitError, dataRate, qualityDAB;
  float          lastQualityDAB;
  char           signalStrength;  // DAB signal strength
  char           volume;          // current volume
  String         RTC_timeDateStr; // current time and date as string
} radio_state_t;

typedef struct {
  bool           isNew;           // has EEPROM never been written
  char           progDAB;         // default DAB program index
  char           volume;          // default software volume
  int            minCap, maxCap;  // minimum and maximum C [pF] of program dial
  // ...
  byte           h[COLOR_COUNT], s[COLOR_COUNT], v[COLOR_COUNT];
} radio_config_t;

typedef struct {
  uint16_t       x, y, color, bgrColor, size;
  bool           doWrap;
  String         text;
} text_t;

// Represents an interface element, a button or different kinds of dials
//   val       := current value of the element, with 0 or 1 for buttons and
//                a dial-type specific value for dials
//   isChanged := if val has changed since the last check
//
typedef struct {
  uint8_t        pin, pin2, type;
  char           name[8];
  bool           rawState, state, lastState, readingState, highState;
  int            readingVal, lastVal, val;
  bool           isChanged;
  float          cap;
  unsigned long  tLast;
} control_t;


radio_state_t    Radio;
radio_config_t   Conf;
RTC_time_t       RTC_time;
char             ch;
int              res, nProg, iProg, iPrevProg, dirProg;
unsigned long    tLastUpdate;
bool             failed;
bool             isAmpPowerOn, relaySource;
BBE_EQ_t         BBE_EQ;

int              FSM_state, FSM_lastState;
int              nCapBins;
float            dCapBin;
bool             isControlChanged;

control_t        Controls[CONTROL_COUNT] = {
  {PIN_BUTTON_SPRACHE, 0,   CONTROL_BUTTON, "SPRACHE",
   LOW, LOW,  HIGH, LOW, LOW,  0,0,-1, false, 0.0, 0L},
  {PIN_BUTTON_TA, 0,        CONTROL_BUTTON, "TA",
   LOW, LOW,  HIGH, LOW, LOW,  0,0,-1, false, 0.0, 0L},
  {PIN_BUTTON_UKW, 0,       CONTROL_BUTTON, "UKW",
   LOW, HIGH, LOW,  LOW, HIGH, 0,0,-1, false, 0.0, 0L},
  {PIN_BUTTON_MW, 0,        CONTROL_BUTTON, "MW",
   LOW, LOW,  HIGH, LOW, LOW,  0,0,-1, false, 0.0, 0L},
  {PIN_CAP_IN, PIN_CAP_OUT, CONTROL_DIAL_C, "SENDER",
   LOW, LOW,  LOW,  LOW, LOW,  0,0,-1, false, 0.0, 0L},
  {PIN_DIAL_TON, 0,         CONTROL_DIAL_R, "TON",
   LOW, LOW,  LOW,  LOW, LOW,  0,0,-1, false, 0.0, 0L}};

//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
void initOtherHardware()
{
  pinMode(PIN_AMP_POWER, OUTPUT);
  switchAmpPower(true);
  pinMode(PIN_RELAY, OUTPUT);
  switchRelayToSource(RELAY_RADIO);
}

//--------------------------------------------------------------------------------
void setup()
{
  // Initialize
  //
  Radio.progDAB        = 10;
  Radio.progName[0]    = 0;
  Radio.progAbbr[0]    = 0;
  Radio.state          = PLAY_STATUS_INVALID;
  Radio.mode           = STREAM_MODE_DAB;
  Radio.stereo         = -1;
  Radio.playMode       = -1;
  Radio.isProgChanged  = false;
  Radio.bitError       = 0;
  Radio.dataRate       = 0;
  Radio.signalStrength = 0;
  Radio.volume         = 10;
  Radio.qualityDAB     = 0;
  Radio.lastQualityDAB = 50;
  Radio.DABService     = DAB_SERVICE_UNKNOWN;
  Radio.isMute         = false;

  tLastUpdate          = millis();
  dirProg              = 1;
  iPrevProg            = 0;
  FSM_state            = FSM_UNDEFINED;
  FSM_lastState        = FSM_state;
  isControlChanged     = false;

  BBE_EQ.BBEOn         = 2;
  BBE_EQ.EQMode        = EQ_MODE_JAZZ;
  BBE_EQ.BBELo         = 5;  // 0..24, 0~12dB, 0.5dB/step
  BBE_EQ.BBEHi         = 0;  // 0..24, 0~12dB, 0.5dB/step
  BBE_EQ.BBECFreq      = 0;  // 0..1, 595Hz or 1000Hz
  BBE_EQ.BBEMachFreq   = 60; // 60, 90, 120, 150Hz
  BBE_EQ.BBEMachGain   = 0;  // 0, 4, 8, 12dB
  BBE_EQ.BBEMachQ      = 1;  // 1 or 3
  BBE_EQ.BBESurr       = 0;  // 0~10dB
  BBE_EQ.BBEMp         = 0;  // 0~10dB
  BBE_EQ.BBEHpF        = 0;  // 0 or 20~250Hz(10Hz/step), 0=off
  BBE_EQ.BBEHiMode     = 0;  // 0~12dB

  SER_DEBUG.begin(baudSerMonkeyRadio);
  delay(50);

  // Initialize modules
  //
  readConfig();
  initOtherHardware();
  initControls();
  initLEDs();
  #ifdef USE_DISPLAY
  initDisplay();
  #endif
  initControls();
  // ...

  // Open COM link to radio
  //
  SER_MONKEY_RADIO.begin(baudSerMonkeyRadio);
  SER_MONKEY_RADIO.setTimeout(tOutSerMonkeyRadio_ms);

  SerHostStream  = &SER_MONKEY_RADIO;
  SerDebugStream = &SER_DEBUG;
  monkeyRadio.setStream(&SER_HOST, &SER_DEBUG_OUT);
  delay(100);

  if(!monkeyRadio.openRadioPort(false)) {
    SER_DEBUG.println(F("ERROR: failed to open radio"));
  }
  else {
    // Link to radio established
    //
    monkeyRadio.getTotalDABProgramsFromDB();
    if(monkeyRadio.getDABProgramCount() == 0) {
      // No DAB programs in database, start search ...
      //
      monkeyRadio.DABAutoSearch(DAB_MIN_PROG, DAB_MAX_PROG, (displayFunc)displayMsg);
      monkeyRadio.getTotalDABProgramsFromDB();
    }
    monkeyRadio.setVolume(Radio.volume);
    monkeyRadio.setBBEEQ(BBE_EQ);
    monkeyRadio.syncRTC(true);

    if(monkeyRadio.getDABProgramCount() > 0) {
      // DAB programs are available
      //
      updateProgramDialParams();
      Radio.isProgChanged = true;
      // ...
    //FSM_state = FSM_DAB_PLAY;
    }
  }
  handleKeyInput('?');
}

//--------------------------------------------------------------------------------
void loop() {
  // Check if a command arrived via the terminal
  //
  if(Serial.available()) {
    ch = Serial.read();
    if(!handleKeyInput(ch)) {
      SER_DEBUG.println(F("ERROR: unknown key"));
      // ...
    }
  }
  // Update representation of interface hardware and finite state machine
  //
  updateControlStates();
  updateFSM();

  // If program has changed, switch station ...
  //
  if(Radio.isProgChanged) {
    monkeyRadio.quiet(true);

    // Check if selected index is an audio stream, if not, go to the next
    // or stay if at the end of the range
    //
    nProg  = monkeyRadio.getDABProgramCount();
    iProg  = Radio.progDAB;
    res    = monkeyRadio.getServCompType(iProg);
    failed = false;
    while(!failed && (res != DAB_SERVICE_DAB) && (res != DAB_SERVICE_DAB_PLUS)) {
      if(dirProg > 0) {
        if(iProg < nProg-1)
          iProg++;
        else
          failed = true;
      }
      else {
       if(iProg > 0)
          iProg--;
        else
          failed = true;
      }
      res = monkeyRadio.getServCompType(iProg);
    }
    if(!failed)
      Radio.progDAB = iProg;
    else {
      Radio.progDAB = iPrevProg;
    }
    // Switch to new program
    // (or stay with previous one, if there is no next one)
    //
    monkeyRadio.playStream(STREAM_MODE_DAB, Radio.progDAB);
    Radio.state = monkeyRadio.getPlayStatus();
    flickerLED_Info(ID_COLOR_TUNING, LED_FLICKER_MIN, LED_FLICKER_MAX, LED_FLICKER_DI, LED_FLICKER_DT, true);
    while(Radio.state != PLAY_STATUS_PLAY) {
      flickerLED_Info(ID_COLOR_TUNING, 0, 0, 0, 0, false);
      Radio.state = monkeyRadio.getPlayStatus();
    }
    monkeyRadio.quiet(false);
    tLastUpdate = 0;
    #ifdef USE_DISPLAY
    displayClear();
    #endif
    Radio.isProgChanged = false;
  }

  // Update radio info at a defined interval
  //
  if(millis() > (tLastUpdate +RADIO_UPDATE_MS)) {
    updateStateInfo();
    #ifdef USE_DISPLAY
    displayInfo();
    #endif
    tLastUpdate = millis();
  }
  // Delay main loop to allow hardware to keep up
  //
  if(LOOP_DELAY_MS > 0)
    delay(LOOP_DELAY_MS);
}
//--------------------------------------------------------------------------------
