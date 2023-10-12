// =======================================================
// Chess Solitaire Clock:  Copyright(c) 2023 - Jeff French
// =======================================================
//
// The Goal ... Create a device that simulates using a Chess Clock while playing on an electronic board ... 
// It shouldn't matter whether you are playing a live game online, or a bot or some local A.I. while sitting
// at the electronic board.
//
// The purpose is to simulate a full OTB Tournament Experience ... so if you have this device, an electronic 
// board and a scoresheet (paper or electronic ... where you record the moves of the game), you should effectively 
// be able fully simulate on OTB tournament experience.
// 
// Components Required:
//     Arduino Nano (or really any Arduino)
//       What will run the Clock Application
//     Display ... note:  I decided to go with a TM1638 module instead which has 8 7-segment alph-numeric outputs
//         Display the status of the clock:
//            Initially maybe just to recognize whose side it is to move
//            Later maybe a minute:second countdown (but it's really not necessary!!!)
//     Push Button (temporarily just use a button on the TM1638 ... it has 8!)
//            To Stop The Clock
//     PIR (Motion) Sensor Module ... NOTE: It takes about 30 seconds for the PIR Sensor to 'warm up' ... this is normal.
//            When the player using the e-board moves, the only thing needed is the push button to stop the clock 
//            however, we also need to know when your opponent moves and and take action to start the player's
//            clock back up.  The idea here is to automatically notice movement when the player makes the opponents move 
//            on the e-board so that it will automatically start your clock again. 
//

#include <TM1638.h>
#include <Event.h>
#include <MonkTimer.h>
#include <EEPROM.h>

// I wanted direct access to these, so I just copied from TM1638.h which are private in that class.  
// I could have made them public, but any updates would likely revert it back to private. 

uint8_t _digits[16]={
      0b00111111,0b00000110,0b01011011,0b01001111,
      0b01100110,0b01101101,0b01111101,0b00000111,
      0b01111111,0b01101111,0b01110111,0b01111100,
      0b00111001,0b01011110,0b01111001,0b01110001
    };

//define keypad buttons
#define btnWhite        0 // Start as White
#define btnReset        1 // Reset
#define btnStart        2 // Reset to Start Values (StartMinutes, Start Seconds and StartCheckDelay) ... after this, typically a Reset would occur and possibly a Save to EEPROM
#define btnDec          3 // Decrease Clock Time/Decrease Pir Check Delay (dependent on player color and which side's clock is running)
#define btnInc          4 // Increase Clock Time/Increase Pir Check Delay (dependent on player color and which side's clock is running)
#define btnPause        5 // Pause
#define btnSave         6 // Save to EEPROM
#define btnBlack        7 // Start as Black

#define btnNONE   -1

#define EVENT_OFF -1

#define pirPin        2
#define stopclockPin  3

//Variables 
int StartMinutes = 60;
int StartSeconds = 6;
int Minutes;
int Seconds;
bool blinkState = true;
bool firstMove = true;
bool isPaused = false;
int activeColor = 0;  // White = 1; Black = 2
int playerColor = 0;

int StartCheckDelay = 10; // How long to wait after a player's clock is stopped before check the pir sensor
int pirCheckDelay; 

//TM1638 setup
#define STB A0 // Strobe digital pin
#define CLK A1 // clock digital pin
#define DIO A2 // data digital pin

TM1638 tm(CLK,DIO,STB);

uint8_t text[8];
bool led[8];

MonkTimer t;
int tickEvent = EVENT_OFF;
int waitEvent = EVENT_OFF;

// Pir Sensor
int pirState = LOW;

// Stop Clock Button
int stopclockState = LOW;

//Arduino setup
void setup() 
{
  Serial.begin(9600);

  int eval;
  eval = EEPROM.read(0);
  if (eval != 255)
  {
    if (eval > 99)
      eval = 99;
    if (eval < 10)
      eval = 10;
    StartMinutes = eval;
  }
  eval = EEPROM.read(1);
  if (eval != 255)
  {
    eval = 6;
    StartSeconds = eval;
  }
  eval = EEPROM.read(2);
  if (eval != 255)
  {
    if (eval > 15)
      eval = 15;
    if (eval < 5)
      eval = 5;
    StartCheckDelay = eval;
  }

  //setup Pir Sensor
  pinMode(pirPin, INPUT);
  pinMode(stopclockPin, INPUT);

  performReset();
}

//Arduino loop
void loop() 
{
  checkButtons();
  clearButtons();

  clearStopClock();
  clearPir();
  
  if (activeColor == playerColor)
  {
    checkStopClock();
  }
  else
  {
    checkPir();
  }
    
  if (!isPaused)
    t.update();
}

// Reset to Startup Settings
void performReset()
{
  stopTickEvent();
  stopWaitEvent();

  blinkState = true;
  firstMove = true;
  activeColor = 0;
  playerColor = 0;

  Minutes = StartMinutes;
  Seconds = StartSeconds;

  pirCheckDelay = StartCheckDelay;
  pirState = LOW;

  stopclockState = LOW;

  for (int8_t i=0; i<8; i++)
  {
    text[i] = 0x00;
    led[i] = false;
  }

  tm.reset();

  startTickEvent();
}

//read buttons
int readButtons()
{
  uint8_t buttons;
  if ((buttons = tm.getButtons()) == 0)
    return btnNONE;

  if (bitRead(buttons, 0))  return btnWhite;
  if (bitRead(buttons, 1))  return btnReset;
  if (bitRead(buttons, 2))  return btnStart;
  if (bitRead(buttons, 3))  return btnDec;
  if (bitRead(buttons, 4))  return btnInc;
  if (bitRead(buttons, 5))  return btnPause;
  if (bitRead(buttons, 6))  return btnSave;
  if (bitRead(buttons, 7))  return btnBlack;

  return btnNONE;  // we should never get here
}

// Clear StopClock (Touch Sensor)
void clearStopClock()
{
  if ((playerColor != 0) && (playerColor == activeColor))
    while (digitalRead(stopclockPin));
}

// Check StopClock (Touch Sensor)
void checkStopClock()
{
  if ((playerColor != 0) && (playerColor == activeColor) && (stopclockState != HIGH))
  {
    delay(300);
    int stopclockvalue = digitalRead(stopclockPin);
    if (stopclockvalue == HIGH)
    {
      firstMove = false;
      stopclockState = HIGH;
      ClearDelay((playerColor == 1) ? 6 : 2);
      
      // Wait defined seconds before the pir sensor will check for movement again
      startWaitEvent();
    }
  }
}

// Clear Pir Motion Sensor
void clearPir()
{
  if ((playerColor != 0) && (activeColor != playerColor))
    while (digitalRead(pirPin));
}

// Check Pir Motion Sensor
void checkPir()
{
  if ((playerColor != 0) && (activeColor != playerColor) && (pirState != HIGH) && (waitEvent == EVENT_OFF))
  {
    delay(300);
    int pirValue = digitalRead(pirPin);
    if (pirValue == HIGH)
    {
      firstMove = false;
      pirState = HIGH;
    }
  }
}

// Get rid of any extra button presses
void clearButtons()
{
  while (tm.getButtons());
}

// Check TM1638 buttons
void checkButtons() 
{
  switch (readButtons()) // depending on which button was pushed, we perform an action
  {
    case btnWhite:
    {
      if (playerColor == 0)
      {
        playerColor = 1;
        activeColor = 1;
        updateDisplay();
      }
      break;
    }
    case btnReset:
    {
      performReset();
      break;
    }
    case btnStart: 
    {
      StartMinutes = 60;
      StartSeconds = 6;
      StartCheckDelay = 10;
      break;
    }
    case btnDec:
    {
      if (playerColor == activeColor)
      {
        if (StartMinutes > 10)
          StartMinutes -= 5;
        Minutes = StartMinutes;
        Seconds = StartSeconds;
      }
      else
      {
        if (StartCheckDelay > 5)
          StartCheckDelay--;
        pirCheckDelay = StartCheckDelay;
        UpdateDelay((activeColor== 1) ? 2 : 6);
      }
      break;
    }
    case btnInc:
    {
      if (playerColor == activeColor)
      {
        if (StartMinutes < 95)
          StartMinutes += 5;
        Minutes = StartMinutes;
        Seconds = 5;
      }
      else
      {
        if (StartCheckDelay < 15)
          StartCheckDelay++;
        pirCheckDelay = StartCheckDelay;
        UpdateDelay((activeColor == 1) ? 2 : 6);
      }
      break;
    }
    case btnPause:
    {
      isPaused = (isPaused) ? false : true;
      break;
    }
    case btnSave:
    {
      EEPROM.write(0, StartMinutes);
      EEPROM.write(1, StartSeconds);
      EEPROM.write(2, StartCheckDelay);
      break;
    }
    case btnBlack:
    {
      if (playerColor == 0)
      {
        playerColor = 2; 
        activeColor = 1;
        startWaitEvent();
        
        Seconds = 1;
        updateDisplay();
      }
      break;
    }
    case btnNONE:
      break;
  }
}

// white timer 
void timerWhite() 
{
  //clear black timer
  if (activeColor != playerColor && firstMove == false)
  {
    if (text[5] <= 0x80)
      text[5] += 0x80;
  }
  else
  {
    for (int i = 4; i <= 7; i++)
    {
      if (text[i] >= 0x80)
        text[i] -= 0x80;
    }
  }

  for (int i = 4; i <= 7; i++)
    led[i] = false;
  
  if (waitEvent != EVENT_OFF)
  {
    led[0] = true;
  }
  else
  {
    for (int i = 0; i <= 3; i++)
      led[i] = true;
  }
  
  switch (blinkState)
  {
    case 1:
    {
      if (activeColor == playerColor)
      {
        if (text[1] >= 0x80)
          text[1] -= 0x80;
      }
      else
      {
        for (int i = 0; i <= 3; i++)
        {
          if (text[i] >= 0x80)
            text[i] -= 0x80;
        }
      }
      blinkState = 0;  
      break;
    }

    case 0:
    {
      if (activeColor == playerColor)
      {
        UpdateTime(0);
        if (text[1] < 0x80)
          text[1] += 0x80;
      }
      else
      {
        for (int i = 0; i <= 3; i++)
        {
          if (text[i] < 0x80)
            text[i] += 0x80;
        }
      }
      blinkState = 1;
      break;
    }
  }
}

// black timer
void timerBlack() 
{
  if (activeColor != playerColor)
  {
    if (text[1] <= 0x80)
      text[1] += 0x80;
  }
  else
  {
    for (int i = 0; i <= 3; i++)
    {
      if (text[i] >= 0x80)
       text[i] -= 0x80;
    }
  }

  for (int i = 0; i <= 3; i++)
    led[i] = false;

  if (waitEvent != EVENT_OFF)
  {
    led[7] = true;
  }
  else
  {
    for (int i = 4; i <= 7; i++)
      led[i] = true;
  }
  
  switch (blinkState)
  {
    case 1:
    {
      if (activeColor == playerColor)
      {
        if (text[5] >= 0x80)
          text[5] -= 0x80;
      }
      else
      {
        for (int i = 4; i <= 7; i++)
        {
          if (text[i] >= 0x80)
            text[i] -= 0x80;
        }
      }
      blinkState = 0;  
      break;
    }

    case 0:
    {
      if (activeColor == playerColor)
      {
        UpdateTime(4);
        if (text[5] < 0x80)
          text[5] += 0x80;
      }
      else
      {
        for (int i = 4; i <= 7; i++)
        {
          if (text[i] < 0x80)
            text[i] += 0x80;
        }
      }
      blinkState = 1;
      break;
    }
  }
}

void UpdateTime(int index)
{
  if (Seconds == 0)
  {
    if (Minutes > 0)
    {
      Minutes--;
      Seconds = 59;
    }
  }
  else
  {
    if (Seconds > 0)
      Seconds--;
  }
  
  String MinutesString = String(Minutes);
  text[index] = ((Minutes < 10) ? _digits[0] : _digits[(MinutesString.charAt(0) - '0')]);
  text[index + 1] = ((Minutes < 10) ? _digits[(MinutesString.charAt(0) - '0')] : _digits[(MinutesString.charAt(1) - '0')]);

  String SecondsString = String(Seconds);
  text[index + 2] = ((Seconds < 10) ? _digits[0] : _digits[(SecondsString.charAt(0) - '0')]);
  text[index + 3] = ((Seconds < 10) ? _digits[(SecondsString.charAt(0) - '0')] : _digits[(SecondsString.charAt(1) - '0')]);
} 

void UpdateDelay(int index)
{
  String DelayString = String(pirCheckDelay);
  text[index] = ((pirCheckDelay < 10) ? _digits[0] : _digits[(DelayString.charAt(0) - '0')]);
  text[index + 1] = ((pirCheckDelay < 10) ? _digits[(DelayString.charAt(0) - '0')] : _digits[(DelayString.charAt(1) - '0')]);
}

void ClearDelay(int index)
{
  text[index] = 0x00;
  text[index + 1] = 0x00;
}

// Tick Update
void updatetick()
{
  if (activeColor == playerColor && stopclockState == HIGH)
  {
    if (activeColor == 1)
      activeColor = 2;
    else
      activeColor = 1;
  }

  stopclockState = LOW;

  switch (activeColor)
  {
    case 1: 
    {
      timerWhite();
      break;
    }

    case 2: 
    {
      timerBlack();
      break;
    }

    default:
      break;
  }

  if (pirState == HIGH)
  {
    if (activeColor != playerColor)
    {
      if (playerColor == 1)
        activeColor = 1;
      else
        activeColor = 2;
    }
  }

  pirState = LOW;

  updateDisplay();
}

// Pir Wait Update
void updatewait()
{
  // We just need to turn off the Event if it's active. 
  stopWaitEvent();
}

void startWaitEvent()
{
  stopWaitEvent();
  waitEvent = t.every((pirCheckDelay * 1000), updatewait);
}

void stopWaitEvent()
{
  if (waitEvent != EVENT_OFF)
  {
    t.stop(waitEvent);
    waitEvent = EVENT_OFF;
  }
}

void startTickEvent()
{
  stopTickEvent();
  tickEvent = t.every(500, updatetick);
}

void stopTickEvent()
{
  if (tickEvent != EVENT_OFF)
  {
    t.stop(tickEvent);
    tickEvent = EVENT_OFF;
  }
}

// Update TM1638 7-Segment Displays and LEDs
void updateDisplay()
{
  for (int8_t i=0; i<8; i++)
  {
    tm.displayDig(7-i, text[i]);
    tm.writeLed(i+1, led[i]);
  }
}


