//--------------------------------------------------------------------------------
// project : monkeyDAB
// module  : monkeyDAB_LED
//           LED related functions
// author  : Thomas Euler
// history : 2016-10-08, created
//
//--------------------------------------------------------------------------------
void initLEDs()
{
  LEDs.setOutput(PIN_LED);
  _setLEDStatus(ID_LED_INFO,  ID_COLOR_STARTING, -1, true);
  _setLEDStatus(ID_LED_INFO2, ID_COLOR_STARTING, -1, true);
  // ...
}

//--------------------------------------------------------------------------------
void setLEDStatus_Info(byte state, int intens, bool doSync)
{
  _setLEDStatus(ID_LED_INFO,  state, intens, doSync);
  _setLEDStatus(ID_LED_INFO2, state, intens, doSync);
}

void flickerLED_Info(byte color, int i0, int i1, int di, int dt, bool doReset)
{
  _flickerLED(ID_LED_INFO,  color, i0, i1, di, dt, doReset);
  _flickerLED(ID_LED_INFO2, color, i0, i1, di, dt, doReset);
}

//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
void _setLEDStatus(byte index, byte state, int intens, bool doSync)
{
  byte v;

  if((index >= 0) && (index < LED_COUNT)) {
    if(intens < 0)
      v = Conf.v[state];
    else
      v = byte(constrain(intens, 0, 255));
    _setLEDColorHSV(index, Conf.h[state], Conf.s[state], v, doSync);
  }
}

//--------------------------------------------------------------------------------
void _flickerLED(byte index, byte color, int i0, int i1, int di, int dt, bool doReset)
{
  static int _i0, _i1, _di, _dt, _i;

  if(doReset) {
    _i0 = i0;
    _i1 = i1;
    _di = di;
    _dt = dt;
    _i  = (_i0 +_i1)/2;
  }
  else {
    _i += _di;
    if((_i > _i1) || (_i < _i0))
      _di = -_di;
  }
  _setLEDStatus(index, color, _i, true);
  if(!doReset)
    delay(_dt);
}

//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
void _setLEDColorHSV(byte index, byte hue, byte sat, byte val, bool doSync)
{
  cRGB rgb;
  rgb.SetHSV(hue, sat, val);
  LEDs.set_crgb_at(index, rgb);
  if(doSync)
    LEDs.sync();
}

//--------------------------------------------------------------------------------
