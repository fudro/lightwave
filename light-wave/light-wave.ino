/*
	Example of use of the FFT libray to compute FFT for a signal sampled through the ADC.
  Copyright (C) 2018 Enrique Condés and Ragnar Ranøyen Homb

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
  
  *********************************************************************
  6/30/2018
  With modifications by Anthony Fudd based on the following sources:
  https://www.instructables.com/id/Arduino-Audio-Input/
  This provides a faster way to sample audio on the Arduino.
  Also include multiple upgrades for musical note detection from a input signal using an Arduino Uno or Mega.
  Arduino Mega allows for more samples and includes recognition of sharps and flats which the Uno cannot do.
  *********************************************************************
  FREQUENCY MEASUREMENT NOTES:
  Tested with the Signal Gen app on the iPhone: https://apps.apple.com/us/app/audio-signal-generator/id543661843

  HARDWARE NOTES:
  This program works with standard Arduino AVR boards using the ATMega328 chip (e.g. Uno, Mega, and Nano).
  DOES NOT work on SAMD chips (e.g. MKRZero board) because that chip does not have the ADCSRA and ADCSRB registers.
 
  NOTE DETECTION:
  To detect notes from the serial monitor output of the FFT calculations:
  1) Ignore the first two elements of the "vData" array of frequency magnitudes. (The first two elements seem to be collecting noise and are filled with unusally high junk values.)
  2) The array is zero indexed, so start with array element [2].
  3) Compare the value in current element to the next element in the array.
  4) Keep doing this until the next element is less than the current element.
  5) Use the current element for note lookup. This may need to be adjusted according to the accuracy of the circuit.
  6) Using a table of frequency values for musical notes: https://mixbutton.com/mixing-articles/music-note-to-frequency-chart/

  IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
  pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
  and minimize distance between Arduino and first pixel.  Avoid connecting
  on a live circuit...if you must, connect GND first.
*/

#include "arduinoFFT.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6

/*
 Parameter 1 = number of pixels in strip
 Parameter 2 = Arduino pin number (most are valid)
 Parameter 3 = pixel type flags, add together as needed:
 NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
 NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
 NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
 NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
 NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
*/
const int led_total = 60;    //total number of LEDs in the strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(led_total, PIN, NEO_GRB + NEO_KHZ800);
arduinoFFT FFT = arduinoFFT();          //Create FFT object
const uint16_t samples = 128;           //This value MUST ALWAYS be a power of 2 (Arduino Uno max value is 128. Best value for Arduino Mega is 256)
const double samplingFrequency = 1760; /* Set samplingFrequency in Hz.
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
char *noteNames_Uno[] =     {" ",   " ",    " ",    " ",    " ",    " ",    " ",    " ",  "C3",     "D3",     "E3",   "F3",   "G3",   " ",    "A3",   " ",      "B3",   "C4",   " ",      "D4",   "D4",   " ",      "E4",   " ",    "F4",   " ",      " ",    "G4",   " ",    " ",            "A4",   " ",  " ",  " ",  "B4",   " ",    "C5",   "C5",   " ",      " ",    " ",    "D5",   " ",    " ",      " ",    " ",    "E5",   " ",    " ",    "F5",   " ",    " ",    " ",      " ",    " ",    "G5",   "G5",   " ",    " ",      " ",    " ",          "A5"};
char *noteNames_Mega[] =    {" ",   " ",    " ",    " ",    "E",    "G",    "A2",   "B2",   "C3#",    "D3#",  "E3",   "F3",   "G3",   "G3#",  "A3",   "A3#",    "B3",   "C4",   "C4#",    "D",    "D4",   "D4#",    "E4",   "F",    "F4",   "F4#",    "G",    "G4",   "G#",   "G4#",    "A",  "A4",   "A4#",    "B4",   "B4",   " ",    "C5",   " ",    "C5#",    " ",    "D5",   "D",    " ",    "D5#",    " ",    " ",    "E5",   " ",    " ",    "F5",   " ",    " ",    "F5#",    " ",    " ",    "G5",   " ",    " ",    "G5#",    " ",    " ",    " ",  "A5", " ",  " ",  " ",  "A5#",  " ",  " ",  " ",  "B5", " ",  " ",  " ",  "C6", " ",  " ",  " ",  " ",  "C6#",  " ",  " ",  " ",  " ",  "D6", " ",  " ",  " ",  "D#",   "D6#",  " ",  " ",  " ",  "E6",   " ",  " ",  " ",  " ",  " ",  "F6",   "F",  " ",  " ",  " ",  " ",  " ",  "F6#",  " ",  " ",  " ",  " ",  " ",  " ",  "G6",   " ",  " ",  " ",  " ",  " ",  " ",  "G6#",  " ",  " ",  " ",  " ",  " ",  "A"};                                                                            
int noteColors_samples[] =  {0,     0,      0,      0,      5,      8,      10,     12,     2,        4,      5,      6,      8,      9,      10,     11,       12,     1,      2,        3,      3,      4,        5,      0,      6,      7,        0,      8,      9,      9,        10,    10,     11,      12,     12,     0,      1,      0,      2,        0,      3,      0,      0,      4,        0,      0,      5,      0,      0,      6,      0,      0,      7,        0,      0,      8,      0,      0,      9,        0,      0,      0,    10,   0,    0,    0,    11,     0,    0,    0,    12,   0,    0,    0,    1,    0,    0,        0,      2,      0,    0,    0,    0,    3,    0,    0,    0,    0,      4,      0,    0,    0,    5,      0,    0,    0,    0,    0,    6,      0,    0,    0,    0,    0,    0,    7,      0,    0,    0,    0,    0,    0,    8,      0,    0,    0,    0,    0,    0,    9,      0,    0,    0,    0,    0,    10};
/*Corresponding Sample Frequency:                           82.5    96.25   110     123.75  137.5     151.25  165     178.75  192.5   206.25  220     233.75    247.5   261.25  275               302.5   316.25    330             357.5   371.25            398.75          426.25          453.75  467.5     481.25  495             522.5           550               577.5   591.25          618.75                    660                     701.25                  742.5                     783.75                  825                             880                     935                       990                     1045                          1113.75                         1182.5                   1237.5 1251.25                   1306.25                               1388.75 1402.5                              1471.25                                     1567.5                                      1663.75                               Cannot be detected - out of sample range (last detectable sample frequency is 1746.25)
 *Actual Note Frequency:                                    82.41   98      110     123.47  138.59    155.56  164.81  174.61  196     207.65  220     233.08    246.94  261.63  277.18    293.66          311.13    329.63  349.23          369.99    392             415.3             440           466.16    493.88                  523.25          554.37            587.33                  622.25                    659.25                  698.46                  739.99                    783.99                  830.61                          880                     932.33                    987.77                  1046.5                        1108.73                         1174.66                         1244.51                   1318.51                               1396.91                                     1479.98                                     1567.98                                     1661.22                               1760
 *NOTES:                                                    GUITAR  GUITAR
 */

double max_mag = 0;   //store the last maximum magnitude
int led_index = 0;    //track the individual LED that should be illuminated each loop. Defaults to 0 to match zero indexed array.
int led_speed = 10;   //how fast the LEDs illuminate in sequence
uint32_t led_colors[] = { strip.Color(0, 0, 0),         //Off       0
                          strip.Color(127, 0, 0),       //Red       1   C
                          strip.Color(127, 0, 63),      //Pink      2   C#
                          strip.Color(127, 31, 0),      //Orange    3   D
                          strip.Color(127, 80, 0),      //Yellow    4   D#
                          strip.Color(63, 127, 0),      //Lime      5   E
                          strip.Color(0, 127, 0),       //Green     6   F
                          strip.Color(0, 0, 127),       //Blue      7   F#
                          strip.Color(0, 127, 127),     //Cyan      8   G
                          strip.Color(0, 127, 30),      //Ocean     9   G#
                          strip.Color(40, 0, 127),      //Violet    10  A
                          strip.Color(127, 0, 127),     //Magenta   11  A#
                          strip.Color(127, 127, 127)};  //White     12  B


void setup()
{
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

  //LED Strip Setup
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  //Serial Communication Setup
  Serial.begin(115200);
  delay(500);
  Serial.println("LightWave Note Display");
  Serial.println("Ready");
}

void loop()
{
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
  FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);	// Weigh data
  FFT.Compute(vReal, vImag, samples, FFT_FORWARD); // Compute FFT
  FFT.ComplexToMagnitude(vReal, vImag, samples); // Compute magnitudes

//  Serial.println(noteNames_Uno[getPeak(vReal, (samples >> 1))]); //Print the name of the musical note
//  Serial.println(noteNames_Mega[getPeak(vReal, (samples >> 1))]); //Print the name of the musical note
//  while(1); // Run Once
//  delay(5); // Repeat after delay

  if(led_index < led_total) {    //repeat until end of strip
    set_LED(led_index, led_colors[noteColors_samples[getPeak(vReal, (samples >> 1))]]);
    led_index++;
  }
  else{   //end of strip
    strip_Off();      //shut off all LEDs. DISABLE WHEN USING ONLY 1 LED.
    led_index = 0;    //reset LED index to the front of the strip
    delay(led_speed);   //pause between cycles
  }
}

int getPeak(double *vData, uint16_t bufferSize) {
  int max_mag_index = 0;
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

// Fill LED strip one after the other with a color
void set_LED(int index, uint32_t c) {
  strip.setPixelColor(index, c);
  strip.show();
}

// Clear LED strip (Turn all LEDs OFF)
void strip_Off() {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
    strip.show();
  }
}
