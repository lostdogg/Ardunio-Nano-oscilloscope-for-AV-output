#include <TVout.h>
#include <fontALL.h>

TVout TV;

// Pin Definitions
const int signalInput = A0;
const int vDivPot = A1;
const int tDivPot = A2;
const int trigPot = A3;
const int hPosPot = A4;
const int modePot = A5; 
const int autoNormalSwitch = 2; // Ground for Normal Mode, Open for Auto

// Display constraints
const int screenW = 120;
const int screenH = 96;
const int centerY = screenH / 2;

// Anti-Overscan: Adjusted maxY to 88 to leave room for the bottom UI text
const int minY = 4;
const int maxY = 88; 

uint8_t waveData[screenW];
uint8_t peakMax[screenW]; 

void setup() {
  TV.begin(NTSC, screenW, screenH);
  TV.select_font(font4x6);
  
  pinMode(signalInput, INPUT);
  pinMode(autoNormalSwitch, INPUT_PULLUP);
  
  // Safe ADC Overclock (Prescaler 16). Fast enough for audio, but stable.
  ADCSRA = (ADCSRA & 0xf8) | 0x04; 
}

void loop() {
  // --- 1. Read UI Controls ---
  // Read raw values first so we can use them for both scaling and the UI percentages
  int rawVDiv = analogRead(vDivPot);
  int rawTDiv = analogRead(tDivPot);
  int rawTrig = analogRead(trigPot);
  int rawHPos = analogRead(hPosPot);
  int rawMode = analogRead(modePot);

  int vScale = map(rawVDiv, 0, 1023, 1, 10);      
  int tDiv = map(rawTDiv, 0, 1023, 0, 100);       
  int trigLevel = rawTrig;                        
  int hPosDelay = map(rawHPos, 0, 1023, 0, 5000); 
  int modeValue = rawMode;                        
  bool isNormalMode = (digitalRead(autoNormalSwitch) == LOW); 
  
  int acqMode = 0;
  if (modeValue > 340 && modeValue <= 682) acqMode = 1;
  else if (modeValue > 682) acqMode = 2;

  // --- 2. Triggering ---
  long timeout = 0;
  // Normal mode waits longer to catch slow waves, Auto mode fires quickly
  long maxTimeout = isNormalMode ? 40000 : 5000; 
  
  // Improve Triggering: Rising Edge
  // Wait for signal to be BELOW trigger first
  while(analogRead(signalInput) > trigLevel && timeout < maxTimeout) { timeout++; }
  // Then wait for it to cross ABOVE trigger (Rising Edge)
  while(analogRead(signalInput) < trigLevel && timeout < maxTimeout) { timeout++; }
  
  if(hPosDelay > 0) delayMicroseconds(hPosDelay);

  // --- 3. Acquisition ---
  if (acqMode == 0) { 
    // SAMPLE MODE
    for (int i = 0; i < screenW; i++) {
      int val = analogRead(signalInput);
      int scaledVal = 512 + ((val - 512) * vScale); 
      if(scaledVal < 0) scaledVal = 0; if(scaledVal > 1023) scaledVal = 1023;
      waveData[i] = map(scaledVal, 0, 1023, maxY, minY); 
      delayMicroseconds(tDiv);
    }
  } 
  else if (acqMode == 1) { 
    // PEAK DETECT MODE
    for (int i = 0; i < screenW; i++) {
      int localMin = 1023;
      int localMax = 0;
      int subSamples = (tDiv / 2) + 1; 
      
      for(int j = 0; j < subSamples; j++) {
        int val = analogRead(signalInput);
        if(val < localMin) localMin = val;
        if(val > localMax) localMax = val;
      }
      
      int scaledMin = 512 + ((localMin - 512) * vScale);
      int scaledMax = 512 + ((localMax - 512) * vScale);
      if(scaledMin < 0) scaledMin = 0; if(scaledMin > 1023) scaledMin = 1023;
      if(scaledMax < 0) scaledMax = 0; if(scaledMax > 1023) scaledMax = 1023;

      waveData[i] = map(scaledMax, 0, 1023, maxY, minY); 
      peakMax[i] = map(scaledMin, 0, 1023, maxY, minY);  
    }
  } 
  else if (acqMode == 2) { 
    // AVERAGE MODE
    for (int i = 0; i < screenW; i++) {
      long sum = 0;
      for(int j = 0; j < 4; j++) { sum += analogRead(signalInput); }
      int scaledVal = 512 + (((sum / 4) - 512) * vScale);
      if(scaledVal < 0) scaledVal = 0; if(scaledVal > 1023) scaledVal = 1023;
      
      waveData[i] = map(scaledVal, 0, 1023, maxY, minY);
      delayMicroseconds(tDiv);
    }
  }

  // --- 4. Drawing ---
  TV.clear_screen();
  
  // Draw Grid
  for(int i = 0; i < screenW; i+=12) TV.set_pixel(i, centerY, 1);
  for(int i = 0; i < screenH; i+=16) TV.set_pixel(screenW/2, i, 1);

  // Draw Waveform
  if (acqMode == 1) {
    for (int i = 0; i < screenW; i++) {
      TV.draw_line(i, waveData[i], i, peakMax[i], 1);
    }
    TV.print(0, 0, "PEAK");
  } else {
    for (int i = 0; i < screenW - 1; i++) {
      TV.draw_line(i, waveData[i], i+1, waveData[i+1], 1);
    }
    if (acqMode == 0) TV.print(0, 0, "SMPL");
    if (acqMode == 2) TV.print(0, 0, "AVG");
  }

  // --- 5. User Interface (UI) ---
  
  // UI: Trigger Status
  if (timeout >= maxTimeout) {
    TV.print(90, 0, "WAIT");
  } else {
    TV.print(90, 0, "TRIG");
  }

  // UI: Visual Trigger Indicator (Tick mark on the left edge)
  // Scale the raw trigger level by the exact same vScale used for the waveform
  int scaledTrig = 512 + ((trigLevel - 512) * vScale);
  if (scaledTrig < 0) scaledTrig = 0; 
  if (scaledTrig > 1023) scaledTrig = 1023;
  
  int trigY = map(scaledTrig, 0, 1023, maxY, minY);
  TV.draw_line(0, trigY, 4, trigY, 1); // Draws a 5-pixel horizontal line on the left

  // UI: Potentiometer Percentages (0-99%)
  // Abbreviations: V=VDiv, T=TDiv, R=Trig, H=HPos, M=Mode
  TV.print(0,  90, "V:"); TV.print(map(rawVDiv, 0, 1023, 0, 99));
  TV.print(24, 90, "T:"); TV.print(map(rawTDiv, 0, 1023, 0, 99));
  TV.print(48, 90, "R:"); TV.print(map(rawTrig, 0, 1023, 0, 99));
  TV.print(72, 90, "H:"); TV.print(map(rawHPos, 0, 1023, 0, 99));
  TV.print(96, 90, "M:"); TV.print(map(rawMode, 0, 1023, 0, 99));
}