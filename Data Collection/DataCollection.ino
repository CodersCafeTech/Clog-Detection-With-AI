#include <Wire.h>


#include "TFT_eSPI.h"                                 // Comes with Wio Terminal package

// Settings
#define BTN_START           0                         // 1: press button to start, 0: loop
#define BTN_PIN             WIO_KEY_C                 // Pin that connects to the button
#define SAMPLING_FREQ_HZ    4                         // Sampling frequency (Hz)
#define SAMPLING_PERIOD_MS  1000 / SAMPLING_FREQ_HZ   // Sampling period (ms)
#define NUM_SAMPLES         8                         // 8 samples at 4 Hz is 2 seconds
#define DEBOUNCE_DELAY      30                        // Delay for debounce (ms)
#define SAMPLE_DELAY        500                       // Delay between samples (ms)


// Used to remember what mode we're in
#define NUM_MODES           2
#define MODE_IDLE           0
#define MODE_RECORDING      1

TFT_eSPI tft;                         // Wio Terminal LCD

const int Output_Pin = D0;
volatile int  Pulse_Count;
unsigned int  Liter_per_hour;
unsigned long Current_Time, Loop_Time,Start_Time, timestamp;

void setup() {
  

  // Initialize button
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(Output_Pin, INPUT);
  // Start serial
  Serial.begin(115200);

  // Configure LCD
  tft.begin();
  tft.setRotation(3);
  tft.setFreeFont(&FreeSansBoldOblique18pt7b);
  tft.fillScreen(TFT_BLACK);
  tft.drawString("IDLE", 30, 100);

  attachInterrupt(0, Detect_Rising_Edge, RISING);

}

void loop() {
  
  static uint8_t mode = MODE_IDLE;
  static uint8_t btn_state;
  static uint8_t last_btn_state = HIGH;
  static unsigned long last_dbnc_time = 0;
  int btn_reading;
  float flowrate_v;

  static unsigned long sample_timestamp = millis();
  static unsigned long start_timestamp = millis();

  // Debounce button - see if button state has changed
  btn_reading = digitalRead(BTN_PIN);
  if (btn_reading != last_btn_state) {
    last_dbnc_time = millis();
  }

  // Debounce button - wait some time before checking the button again
  if ((millis() - last_dbnc_time) > DEBOUNCE_DELAY) {
    if (btn_reading != btn_state) {
      btn_state = btn_reading;
      
      // Only transition to new mode if button is still pressed
      if (btn_state == LOW) {
        mode = (mode + 1);
        if (mode >= NUM_MODES) {
          mode = MODE_IDLE;
        }

        // Update LCD
        tft.fillScreen(TFT_BLACK);
        switch (mode) {
          case MODE_IDLE:
            tft.drawString("IDLE", 30, 100);
            Serial.println();
            break;
          case MODE_RECORDING:
            tft.drawString("Recording", 30, 100);
            Serial.println("timestamp,flowrate");
            start_timestamp = millis();
            break;
          default:
            break;
        }
      }
    }
  }

  // Debounce button - seriously me, stop forgetting to put this in
  last_btn_state = btn_reading;

  // Only collect if not in idle
  if (mode > MODE_IDLE) {
    if ((millis() - sample_timestamp) >= SAMPLE_DELAY) {
      sample_timestamp = millis();
      Current_Time = millis();
     if(Current_Time >= (Loop_Time + 1000))
     {
        Loop_Time = Current_Time;
        Liter_per_hour = (Pulse_Count * 60 / 7.5);
        Pulse_Count = 0;
        Serial.print(sample_timestamp - start_timestamp);
        Serial.print(",");
        Serial.print(Liter_per_hour,DEC);
        Serial.println();
     }
    }
  }
}

void Detect_Rising_Edge ()
{ 
   Pulse_Count++;
} 
