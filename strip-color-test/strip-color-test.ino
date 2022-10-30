  /*
   * This program can be used to test colors for an LED strip (NEOPIXEL or generic WS2812B)
   * It assigns colors to each LED based on the hues provided in the hueColors array.
   * The number of LEDs that display each color can be controlled by the hue_subset variable.
   * The hue_subset variable is static to the loop() function.
   */
  
  #include "FastLED.h"

  #define LED_PIN   6
  #define NUM_LEDS  300
  CRGB leds[NUM_LEDS];
  uint8_t hue = 0;
  uint8_t hueColors[] = { 1,    //0   A   Fire Orange
                          10,   //1   A#  Pumpkin
                          27,   //2   B   Yellow
                          90,   //3   C   Green
                          80,   //4   C#  Lime
                          160,  //5   D   Blue
                          140,  //6   D#  Light Blue
                          120,  //7   E   Teal
                          250,  //8   F   Hot Pink
                          235,  //9   F#  Pink
                          185,  //10  G   Purple
                          200}; //11  G#  Violet                    
                          
  
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
//    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  }
  
  void loop() {
    /*
     * Define static iterator that is self contained to the loop() function.
     * To count backwards from the "far end" of the strip (the end NOT CONNECTED to the microcontroller), initialize interator to the number of LEDs in the strip.
     */
    static int i = NUM_LEDS;      //Initialize interator to the number of LEDs in the strip.
    static int count = 0;         //FOR TESTING: Define static counter that controls the number of LEDs that show each hue.
    static int hue_subset = 10;   //FOR TESTING: Define the number of LEDs between each LED count marker.
    if(i > 0) {
      if((i % hue_subset) == 0) {   //Use modulus operator to check the count of LEDs between each hue marker.
        leds[i] = CRGB::Black; FastLED.show(); delay(30);   //Creat a visual indicator for every Nth LED (based on the hue_susbet variable) to separate hue colors.
        if(i != NUM_LEDS && count < (sizeof(hueColors) / sizeof(hueColors[0]))) {   //Prevent trigger on the first run AND check if the counter is still less than the full length of the hueColors array.
          count++;                                                                  //Triggering on the first run will cause the counter to iterate and the LED strip will skip the first color in the array.
        }
        else {    //...else reset the counter if the end of the hueColors array has been reached.
          count = 0;
        }
      }
      else {
//        leds[i] = CHSV(hue, 255, 255); FastLED.show(); delay(30);   //Basic instruction format for displaying HSV colors 
//        leds[i] = CRGB::Black; FastLED.show(); delay(30);           //Basic instruction format for displaying pre-made colors from FastLED.
        leds[i] = CHSV(hueColors[count], 255, 255); FastLED.show(); delay(30);    //Assign a hue from the hueColors array based on the counter.
      } 
      hue++;
      i--;
    }
    else {
      i = NUM_LEDS;
      hue = 0;
      fadeall();
    }
  }

  void fadeall() { for(int i = NUM_LEDS; i > 0; i--) { leds[i] = CRGB::Black; } }   //Reset colors by setting all LEDs to Black.
