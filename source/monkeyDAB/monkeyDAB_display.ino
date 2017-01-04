//--------------------------------------------------------------------------------
// project : monkeyDAB
// module  : monkeyDAB_display
//           display-related routines
// author  : Thomas Euler
// history : 2016-10-09, created
//           2016-12-24, changed to use OLED 128x32 I2C display
//
//--------------------------------------------------------------------------------
#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

//--------------------------------------------------------------------------------
void initDisplay()
{
  SER_DEBUG.print(F("Initializing display ..."));

  // Initialize with the I2C addr 0x3C (for the 128x32)
  // Initialize with the I2C addr 0x3D (for the 128x64)
  //
  Display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  Display.clearDisplay();
  Display.display();
  delay(200);

  SER_DEBUG.println(F("  done"));
}

//--------------------------------------------------------------------------------
void displayInfo()
{
  text_t   txt;
  uint16_t dh;
  int16_t  x1, y1, i, j;

  // Initialize
  //
  dh           = 9;
  txt.doWrap   = false;
  txt.color    = WHITE;
  txt.bgrColor = BLACK;
  txt.text     = "";
  txt.size     = 1;
  txt.x        = 0;
  txt.y        = 0;

  // Clear display buffer
  //
  Display.clearDisplay();

  // Display time
  //
  if(RTC_time.isOk) {
    txt.text = Radio.RTC_timeDateStr;
  }
  else {
    txt.text = F("Warte auf Uhrzeit ...");
  }
  displayTextXY(&txt);
  txt.y += dh;

  switch(FSM_state) {
    case FSM_AUX :
      txt.text  = F("AUX-Eingang aktiv ...");
      displayTextXY(&txt);
      txt.y     += dh;
      // ...
      break;

    case FSM_DAB_PLAY:
      // Display information about DAB radio channel
      //
      txt.text  = DABServiceTypeStr[Radio.DABService];
      txt.text += F("-Radio ...");
      displayTextXY(&txt);
      txt.y     += dh;

      switch(Radio.state) {
        case PLAY_STATUS_PLAY:
          if(Radio.mode == STREAM_MODE_DAB) {
            // Short name of program
            //
            txt.text   = Radio.progAbbr;
            txt.size   = 2;
            displayTextXY(&txt);
            txt.y     += dh *txt.size;
            txt.size   = 1;

            // Long program name
            //
            txt.text   = Radio.progName;
            txt.size   = 1;
            displayTextXY(&txt);
            txt.y     += dh;

            // Program type
            //
            if(Radio.progDABType < 0)
              txt.text = "nicht verfügbar";
            else {
              txt.text = ProgTypeStr[Radio.progDABType];
            }
            displayTextXY(&txt);
            txt.y     += dh;

            // Type of DAB service, program index and frequency
            //
            txt.text   = "#";
            txt.text  += int(Radio.progDAB);
            txt.text  += " ";
            txt.text  += DABFreqs[Radio.progDAB].freq;
            txt.text  += "MHz ";
          /*displayTextXY(&txt);
            txt.y     += dh;*/

            // Stereo/Mono and data rate
            //
          /*txt.text   = StereoTypeStr[Radio.stereo];
            txt.text  += ", ";
            txt.text  += int(Radio.dataRate);
            txt.text  += " kbps";
            displayTextXY(&txt);
            txt.y     += dh;*/

            // DAB signal strength
            //
            if(Radio.qualityDAB < 10) {
              txt.text += " ";
            }
            if(Radio.qualityDAB != 100) {
              txt.text += " ";
            }
            txt.text  += int(Radio.qualityDAB);
            txt.text  += "%";
            displayTextXY(&txt);
            txt.y     += dh;

          /*if(Radio.signalStrength > 0) {
              Serial.print("strength=");
              Serial.print(Radio.signalStrength, DEC);
              Serial.print(", bit error=");
              Serial.print(Radio.bitError, DEC);
            }
            else {
              Serial.print("strength n/a");
            }*/
          }
          break;
      }
      break;
  }
  // Display buffer
  //
  Display.display();
}

//--------------------------------------------------------------------------------
void displayMsg(String sHeader, String sMsg)
{
  text_t   txt;
  uint16_t dh;

  // Initialize
  //
  dh           = 9;
  txt.doWrap   = true;
  txt.color    = WHITE;
  txt.bgrColor = BLACK;
  txt.text     = "";
  txt.size     = 1;
  txt.x        = 0;
  txt.y        = 0;

  // Clear display buffer
  //
  Display.clearDisplay();

  // Skip line for time
  //
  txt.y += dh;

  txt.text  = sHeader;
  displayTextXY(&txt);
  txt.y     += dh;

  // ...

  // Display buffer
  //
  Display.display();
}

//--------------------------------------------------------------------------------
void displayClear()
{
  Display.clearDisplay();
}


void displayText(text_t* txt)
{
  _displayText(txt, false, false);
}

void displayTextLn(text_t* txt)
{
  _displayText(txt, false, true);
}

void displayTextXY(text_t* txt)
{
  _displayText(txt, true, false);
}

void displayTextXYLn(text_t* txt)
{
  _displayText(txt, true, true);
}

void _displayText(text_t* txt, bool doSetCursor, bool doLF)
{
  Display.setTextColor((*txt).color);
  Display.setTextSize((*txt).size);
  Display.setTextWrap((*txt).doWrap);
  if(doSetCursor)
    Display.setCursor((*txt).x, (*txt).y);
  if(doLF)
    Display.println((*txt).text);
  else
    Display.print((*txt).text);
}

//--------------------------------------------------------------------------------
