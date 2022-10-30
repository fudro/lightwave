  /*
   * This program uses an audio circuit and an FFT library to control the colors 
   * of an LED strip (NEOPIXEL or generic WS2812B).
   * The colors are determined by the frequency of the audio input signal.
   * 
   * The audio circuit is based on this Instructable:
   * https://www.instructables.com/id/Arduino-Audio-Input/
   * 
   * The FFT library information is here:
   *  Example of use of the FFT libray to compute FFT for a signal sampled through the ADC.
   *  Copyright (C) 2018 Enrique Condés and Ragnar Ranøyen Homb
   *  
   *  This program is free software: you can redistribute it and/or modify
   *  it under the terms of the GNU General Public License as published by
   *  the Free Software Foundation, either version 3 of the License, or
   *  (at your option) any later version.
   *  
   *  This program is distributed in the hope that it will be useful,
   *  but WITHOUT ANY WARRANTY; without even the implied warranty of
   *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   *  GNU General Public License for more details.
   *  
   *  You should have received a copy of the GNU General Public License
   *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
   *  
   *  The FastLED library used to control the LED strip can be found here:
   *  
   * 
   */

  #include "arduinoFFT.h"
  #include "FastLED.h"

  #define LED_PIN   6   //Data signal pin for controlling the LEDs
  #define NUM_LEDS  300 //total number of LEDs in the strip
  CRGB leds[NUM_LEDS];  //Create an array of LED objects
  arduinoFFT FFT = arduinoFFT();          //Create FFT object
  const uint16_t samples = 256;           //This value MUST ALWAYS be a power of 2 (Arduino Uno max value is 128. Best value for Arduino Mega is 256)
  const double samplingFrequency = 3520; /* Set samplingFrequency in Hz.
                                            As rule of thumb, set this value to be twice the maximum frequency you wish to detect.
                                            For Arduino Uno, set this value to 1760 for a max frequency of about 860Hz.
                                            For Arduino Mega, set this value to 3520 for a max frequency of about 1746Hz.
                                         */
  unsigned int sampling_period_us;        //The sampling period in microseconds
  unsigned long microseconds;
  
  /*
  These are the input and output vectors
  Input vectors receive computed results from FFT
  */
  double vReal[samples];
  double vImag[samples];
  
  /*
   Create arrays for the note names acccording to the corresponding note frequencies.
  */
//  char *noteNames_Uno[] =           {" ",   " ",    " ",    " ",    " ",    " ",    " ",    " ",  "C3",     "D3",     "E3",   "F3",   "G3",   " ",    "A3",   " ",      "B3",   "C4",   " ",      "D4",   "D4",   " ",      "E4",   " ",    "F4",   " ",      " ",    "G4",   " ",    " ",            "A4",   " ",  " ",  " ",  "B4",   " ",    "C5",   "C5",   " ",      " ",    " ",    "D5",   " ",    " ",      " ",    " ",    "E5",   " ",    " ",    "F5",   " ",    " ",    " ",      " ",    " ",    "G5",   "G5",   " ",    " ",      " ",    " ",          "A5"};
//  char *noteNames_Mega[] =          {" ",   " ",    " ",    " ",    "E",    "G",    "A2",   "B2",   "C3#",    "D3#",  "E3",   "F3",   "G3",   "G3#",  "A3",   "A3#",    "B3",   "C4",   "C4#",    "D",    "D4",   "D4#",    "E4",   "F",    "F4",   "F4#",    "G",    "G4",   "G#",   "G4#",    "A",  "A4",   "A4#",    "B4",   "B4",   " ",    "C5",   " ",    "C5#",    " ",    "D5",   "D",    " ",    "D5#",    " ",    " ",    "E5",   " ",    " ",    "F5",   " ",    " ",    "F5#",    " ",    " ",    "G5",   " ",    " ",    "G5#",    " ",    " ",    " ",  "A5", " ",  " ",  " ",  "A5#",  " ",  " ",  " ",  "B5", " ",  " ",  " ",  "C6", " ",  " ",  " ",  " ",  "C6#",  " ",  " ",  " ",  " ",  "D6", " ",  " ",  " ",  "D#",   "D6#",  " ",  " ",  " ",  "E6",   " ",  " ",  " ",  " ",  " ",  "F6",   "F",  " ",  " ",  " ",  " ",  " ",  "F6#",  " ",  " ",  " ",  " ",  " ",  " ",  "G6",   " ",  " ",  " ",  " ",  " ",  " ",  "G6#",  " ",  " ",  " ",  " ",  " ",  "A"};                                                                            
  const uint8_t noteColors_samples[] =        {0,     0,      0,      0,      8,      11,     1,      3,      5,        7,      8,      9,      11,     12,     1,      2,        3,      4,      5,        6,      6,      7,        8,      0,      9,      10,       0,      11,     12,     12,       1,    1,      2,        3,      3,      0,      4,      0,      5,        0,      6,      0,      0,      7,        0,      0,      8,      0,      0,      9,      0,      0,      10,       0,      0,      11,     0,      0,      12,       0,      0,      0,    1,    0,    0,    0,    2,      0,    0,    0,    3,    0,    0,    0,    1,    0,    0,        0,      2,      0,    0,    0,    0,    3,    0,    0,    0,    0,      4,      0,    0,    0,    5,      0,    0,    0,    0,    0,    6,      0,    0,    0,    0,    0,    0,    7,      0,    0,    0,    0,    0,    0,    8,      0,    0,    0,    0,    0,    0,    9,      0,    0,    0,    0,    0,    10};
  /*Corresponding Sample Frequency:                           82.5    96.25   110     123.75  137.5     151.25  165     178.75  192.5   206.25  220     233.75    247.5   261.25  275               302.5   316.25    330             357.5   371.25            398.75          426.25          453.75  467.5     481.25  495             522.5           550               577.5   591.25          618.75                    660                     701.25                  742.5                     783.75                  825                             880                     935                       990                     1045                          1113.75                         1182.5                   1237.5 1251.25                   1306.25                               1388.75 1402.5                              1471.25                                     1567.5                                      1663.75                               Cannot be detected - out of sample range (last detectable sample frequency is 1746.25)
   *Actual Note Frequency:                                    82.41   98      110     123.47  138.59    155.56  164.81  174.61  196     207.65  220     233.08    246.94  261.63  277.18    293.66          311.13    329.63  349.23          369.99    392             415.3             440           466.16    493.88                  523.25          554.37            587.33                  622.25                    659.25                  698.46                  739.99                    783.99                  830.61                          880                     932.33                    987.77                  1046.5                        1108.73                         1174.66                         1244.51                   1318.51                               1396.91                                     1479.98                                     1567.98                                     1661.22                               1760
   *NOTES:                                                    GUITAR  GUITAR
   */

  //Setup variables for controlling the strip
  int color_index = 0;
  double max_mag = 0;   //store the last maximum magnitude
  const uint8_t led_speed = 10;   //how fast the LEDs illuminate in sequence
                              //Store the desired hue values in an array 
  const uint8_t hueColors[] = { 0,    //0   Default Color (Red)
                                1,    //1   A   Fire Orange
                                10,   //2   A#  Pumpkin
                                27,   //3   B   Yellow
                                90,   //4   C   Green
                                80,   //5   C#  Lime
                                160,  //6   D   Blue
                                140,  //7   D#  Light Blue
                                120,  //8   E   Teal
                                250,  //9   F   Hot Pink
                                235,  //10  F#  Pink
                                185,  //11  G   Purple
                                200}; //12  G#  Violet 
                          
  
  void setup() { 
    /*
     * Set up strip.
     * FORMAT:
     * FastLED.addLeds<[CHIP_TYPE], [DATA_PIN], [COLOR_ORDER]>(leds, NUM_LEDS);  // For COLOR_ORDER, GRB ordering is typical and this parameter can be omitted.
     * 
     * EXAMPLES (both of the following statements will work with the strip I have):
     * FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed and the COLOR_ORDER parameter can be omitted.
     * FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);  // GRB ordering is typical
     */
//      FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
      FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  
      //clear ADCSRA and ADCSRB registers
      ADCSRA = 0;
      ADCSRB = 0;
      
      ADMUX |= (1 << REFS0); //set reference voltage
      ADMUX |= (1 << ADLAR); //left align the ADC value so we can read the highest 8 bits from ADCH register only
    
      ADCSRA |= (1 << ADPS2) | (1 << ADPS0); //set ADC clock with 32 prescaler 16mHz/32 = 500kHz
      ADCSRA |= (1 << ADATE); //enable auto trigger
      ADCSRA |= (1 << ADEN); //enable ADC
      ADCSRA |= (1 << ADSC); //start ADC measurements
      
      sampling_period_us = round(1000000*(1.0/samplingFrequency));

//      //Serial Communication Setup
//      Serial.begin(115200);
//      delay(500);
//      Serial.println("LightWave Note Display");
//      Serial.println("Ready");
  }
  
  void loop() {
    /*
     * Define static iterator that is self contained to the loop() function.
     * To count backwards from the "far end" of the strip (the end NOT CONNECTED to the microcontroller), initialize interator to the number of LEDs in the strip.
     */
    static int i = NUM_LEDS;      //Initialize interator to the number of LEDs in the strip.
    if(i > 0) {
      //SAMPLING
      for(int i=0; i<samples; i++)
      {
        microseconds = micros();    //Overflows after around 70 minutes!
        vReal[i] = ADCH;  //get new value from A0
        vImag[i] = 0;
        while(micros() < (microseconds + sampling_period_us)){
          //empty loop - wait until sampling period has elapsed
        }
      }
      // Compute FFT results from the sampled values
      FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);  // Weigh data
      FFT.Compute(vReal, vImag, samples, FFT_FORWARD); // Compute FFT
      FFT.ComplexToMagnitude(vReal, vImag, samples); // Compute magnitudes

//      Serial.println(noteNames_Mega[getPeak(vReal, (samples >> 1))]); //Print the name of the musical note
//      Serial.println(noteColors_samples[getPeak(vReal, (samples >> 1))]); //Print the hueColors value
//      Serial.println(i);

      color_index = noteColors_samples[getPeak(vReal, (samples >> 1))];   //Get the color index based on the peak note frequency
      if(color_index == 0) {
        leds[i] = CHSV(hueColors[color_index], 255, 0); FastLED.show(); delay(30);    //Set Value parameter to zero to set the LED to OFF.
      }
      else {
        leds[i] = CHSV(hueColors[color_index], 255, 255); FastLED.show(); delay(30);  //Set Value paramater to max of 255 to illuminate the LED as the selected color.
      }
      i--;
    }
    else {
      i = NUM_LEDS;
      fadeall();
      delay(led_speed);   //pause between cycles
    }
  }

  uint8_t getPeak(double *vData, uint16_t bufferSize) {
    uint8_t max_mag_index = 0;
    double max_freq = 0;
    for (uint16_t i = 2; i < bufferSize; i++) //Start loop at the third element of the array (index 2) to avoid the first two entries of the vData array which are always garbage values.
    {
      double frequency; //store the detected frequency
      frequency = ((i * 1.0 * samplingFrequency) / samples);
      //check for maximum magnitude 
      if(vData[i] > max_mag) {  //Find max magnitude for the current sample set
        max_mag = vData[i];     //Update max magnitude value
        max_mag_index = i-2;    //Record index (adjust index value by -2 to account for the first two array entries that wre discarded from the for loop count)
        max_freq = frequency;   //Record frequency with the max magnitude
      }
    }
    max_mag = 0;    //Reset the maximum value for the next call
    if(max_freq >= 490) {   //Adjust index values by minus 1 starting with B4 until the final note of A6 (the A6 limit uses the last detectable frequency because the actual note frequency is out of sample range)
      max_mag_index = max_mag_index - 1;
    }
    /*SPECIAL CASES - Assign specific colors to known frequencies that are difficult for the program to detect due to the fixed sample frequencies.
     * All array indices are zero-indexed.
    */
    if(abs(max_freq - 523.25) < 5) {  //Check for C5
      max_mag_index = 36;  //36th entry (C5) in the notesColorsSamples = 1 for RED
    }
    
    if(abs(max_freq - 207.65) < 5) {  //Check for G3#
      max_mag_index = 13;  //13th entry (G3#) in the notesColorsSamples = 9 for BLUE
    }
    
    if(abs(max_freq - 174.61) < 5) {  //Check for F3
      max_mag_index = 11;  //11th entry (F3) in the notesColorsSamples = 6 for GREEN
    }
    
    return(max_mag_index);
  }

  void fadeall() { for(int i = NUM_LEDS; i > 0; i--) { leds[i] = CRGB::Black; } }   //Reset colors by setting all LEDs to Black.
