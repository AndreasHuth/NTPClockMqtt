#ifndef neo_h
#define neo_h
#include <inttypes.h>
#include <Arduino.h>


#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN D8
Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, LED_PIN);


//#######################################################################################


// set led on 'pos' to red
void SetLedRed (int pos) {
  uint32_t color = strip.Color(  255,   0,   0);
  strip.setPixelColor(pos, color);         //  Set pixel's color (in RAM)
  strip.show();                          //  Update strip to match
}

void ClearLed (int pos) {
  uint32_t color = strip.Color(  0,   0,   0);
  strip.setPixelColor(pos, color);         //  Set pixel's color (in RAM)
  strip.show();                          //  Update strip to match
}

void SetLedAlarmOn( int pos) {
  uint32_t color = strip.Color(  0,   0,  50);
  strip.setPixelColor(pos, color);         //  Set pixel's color (in RAM)
  strip.show();
}

void SetLedSetAlarm ( int pos) {
  uint32_t color = strip.Color(  50,   0,  00);
  strip.setPixelColor(pos, color);         //  Set pixel's color (in RAM)
  strip.show();
}


void SetLedAlarmActive ( int pos) {
  uint32_t color = strip.Color(  0,   100,  00);
  strip.setPixelColor(pos, color);         //  Set pixel's color (in RAM)
  strip.show();
}


// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe2(uint32_t color, int wait) {
  for(int i=strip.numPixels(); i>=0; i--) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}


// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 3 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 3*65536. Adding 256 to firstPixelHue each time
  // means we'll make 3*65536/256 = 768 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 3*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}

void strip_off(void)
{
  for(int c=0; c<strip.numPixels(); c ++) {
    strip.setPixelColor(c, 0); // Set pixel 'c' to value 'color'
  }
}

void setup_neopixel(void) {

  strip.begin();
  strip.clear();
  strip.setBrightness(255);
  strip_off();
  strip.show();
}

void neopixel_fun (int mode){

  if(++mode > 8) mode = 0; // Advance to next mode, wrap around after #8
  switch(mode) {           // Start the new animation...
    case 0: colorWipe(strip.Color(  0,   0,   0), 50);
      strip.clear();
      strip.show();
      break;// Black/off
    case 1: colorWipe(strip.Color(255,   0,   0), 50);
      strip.clear();
      strip.show();
      break;// Red
    case 2: colorWipe(strip.Color(  0, 255,   0), 50);
      strip.clear();
      strip.show(); ;// Green
      break;
    case 3: colorWipe(strip.Color(  0,   0, 255), 50);
      strip.clear();
      strip.show(); // Blue
      break;
    case 4: theaterChase(strip.Color(127, 127, 127), 50);
        strip.clear();
        strip.show(); // White
      break;
    case 5: theaterChase(strip.Color(127,   0,   0), 50);
        strip.clear();
        strip.show(); // Red
      break;
    case 6: theaterChase(strip.Color(  0,   0, 127), 50);
        strip.clear();
        strip.show(); // Blue
      break;
    case 7: rainbow(10);
        strip.clear();
        strip.show();
      break;
    case 8: theaterChaseRainbow(50);
        strip.clear();
        strip.show();
        break;
    default: //colorWipe(strip.Color(  0,   0,   0), 50);
        //strip.clear();
        //strip.show();
        break;
  }
}


#endif
