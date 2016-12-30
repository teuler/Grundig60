//--------------------------------------------------------------------------------
// project : monkeyDAB
// module  : monkeyDAB_support
//           supporting routines
// author  : Thomas Euler
// history : 2016-10-08, created
//
//--------------------------------------------------------------------------------
const char*      ShrtDayOfWeekStr[]= {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};

//--------------------------------------------------------------------------------
void RTCTimeToTimeDateStr(bool withSecs) 
{
  int  d;

  Radio.RTC_timeDateStr = "";  

  if(RTC_time.isOk) {
    // Convert data to string
    //
    d = findDayOfWeek(RTC_time.day, RTC_time.month, RTC_time.year +2000);
    if((d < 0) || (d > 6))
      Radio.RTC_timeDateStr += "n/a";
    else
      Radio.RTC_timeDateStr  = ShrtDayOfWeekStr[d];
    Radio.RTC_timeDateStr   += " ";  
    Radio.RTC_timeDateStr   += int(RTC_time.day);
    Radio.RTC_timeDateStr   += ".";
    Radio.RTC_timeDateStr   += int(RTC_time.month);  
    Radio.RTC_timeDateStr   += ".";
    Radio.RTC_timeDateStr   += RTC_time.year; // +2000;    
    Radio.RTC_timeDateStr   += " ";
    
    // Add time to string
    //
    if(RTC_time.hour < 10) {
      Radio.RTC_timeDateStr += "0";
    }  
    Radio.RTC_timeDateStr   += int(RTC_time.hour);      
    if(RTC_time.minute < 10) {
      Radio.RTC_timeDateStr += ":0";
    }  
    else {
     Radio.RTC_timeDateStr  += ":";    
    }  
    Radio.RTC_timeDateStr   += int(RTC_time.minute);      

    if(withSecs) {
      if(RTC_time.second < 10) {
        Radio.RTC_timeDateStr += ":0";
      }  
      else {
        Radio.RTC_timeDateStr += ":";    
      }  
      Radio.RTC_timeDateStr   += int(RTC_time.second);      
    }
  }  
}  

//--------------------------------------------------------------------------------
int findDayOfWeek(long d, int m, int y)
{
  long month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  long i; 

  if(((y % 4) == 0) && (((y % 100) != 0) || ((y % 400) == 0)))
    month[1] = 29;

  if((y < 1900) || (m < 1) || (m > 12) || (d < 1) || (d > month[m - 1])) {
    return -1;
  }
  for(i=1900; i<y; i++)
    if(((i % 4) == 0) && (((i % 100) != 0) || ((i % 400) == 0)))
      d += 366;
    else {
      d += 365;
    }
  for(i=0; i<m - 1; i++) {
    d += month[i];
  }
  return (d % 7);
}

//--------------------------------------------------------------------------------
void switchAmpPower(bool isOn) 
{
  isAmpPowerOn = isOn;
  digitalWrite(PIN_AMP_POWER, !(isAmpPowerOn));

  SER_DEBUG.print(F("Amplifier power was switched " ));
  SER_DEBUG.println(isAmpPowerOn ? ("ON"):("OFF"));
}

//--------------------------------------------------------------------------------
void switchRelayToSource(bool source) 
{
  if(source != relaySource) {
    relaySource = source;

    SER_DEBUG.print(F("Audio source was switched to " ));
    SER_DEBUG.println(relaySource ? ("AUX"):("radio"));
  }
  digitalWrite(PIN_RELAY, relaySource);
}

//--------------------------------------------------------------------------------
float readDialCapacitance(byte pin_in, byte pin_out)
{
  float         cap;
  int           val, digVal, t_dc;
  unsigned long u1, u2, t;

  pinMode(pin_in, INPUT);
  digitalWrite(pin_out, HIGH);
  val = analogRead(pin_in);
  digitalWrite(pin_out, LOW);

  if(val < 1000) {
    pinMode(pin_in, OUTPUT);
    cap = (float)val *CAP_IN_TO_GND /(float)(MAX_ADC_VALUE -val);
    return cap; // in pF
  }
  else {
    pinMode(pin_in, OUTPUT);
    delay(1);
    pinMode(pin_out, INPUT_PULLUP);
    u1 = micros();
    do {
      digVal = digitalRead(pin_out);
      u2     = micros();
      if(u2 > u1) 
        t    = u2 -u1;
      else   
        t    = u1 -u2;
    }
    while((digVal < 1) && (t < 400000L));

    pinMode(pin_out, INPUT);  
    val  = analogRead(pin_out);
    digitalWrite(pin_in, HIGH);
    t_dc = (int)(t /1000L) *5;
    delay(t_dc);   
    pinMode(pin_out, OUTPUT);  
    digitalWrite(pin_out, LOW);
    digitalWrite(pin_in, LOW);
    cap  = -(float)t / CAP_R_PULLUP / log(1.0 -(float)val /(float)MAX_ADC_VALUE);
    return cap *1000.0; // pF
  }
}
//--------------------------------------------------------------------------------
float readDialCapacitanceAvg(byte pin_in, byte pin_out, int n)
{
  float C = 0.0;
  for(int j=0; j<n; j++) {
    C += readDialCapacitance(pin_in, pin_out);
    delay(50);
  }
  return C/n;
}

//--------------------------------------------------------------------------------
int readDialVoltage(byte pin)
{
  int val;

  // To prevent a voltage at an analog-in pin from interfering with the 
  // capacitance measurement, 1) the voltage for the circuit is only switched on
  // when needed (PIN_DIAL_VC), and 2) the analog-in pin is set as floating input
  // after reading 
  //
  digitalWrite(PIN_DIAL_VC, HIGH);
  delay(2);
  val = analogRead(pin);
  pinMode(pin, INPUT);
  digitalWrite(PIN_DIAL_VC, LOW);
  delay(2);
  return val;
}

//--------------------------------------------------------------------------------