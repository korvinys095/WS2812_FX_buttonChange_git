//------------------------------------- UTILITY FXNS --------------------------------------
//---SET THE COLOR OF A SINGLE RGB LED
void set_color_led(int adex, int cred, int cgrn, int cblu, int cwht) {
  leds[adex].setRGBW( cred, cgrn, cblu, cwht);
}

//---FIND INDEX OF HORIZONAL OPPOSITE LED
int horizontal_index(int i) {
  //-ONLY WORKS WITH INDEX < TOPINDEX
  if (i == BOTTOM_INDEX) {
    return BOTTOM_INDEX;
  }
  if (i == TOP_INDEX && EVENODD == 1) {
    return TOP_INDEX + 1;
  }
  if (i == TOP_INDEX && EVENODD == 0) {
    return TOP_INDEX;
  }
  return LED_COUNT - i;
}

//---FIND INDEX OF ANTIPODAL OPPOSITE LED
int antipodal_index(int i) {
  int iN = i + TOP_INDEX;
  if (i >= TOP_INDEX) {
    iN = ( i + TOP_INDEX ) % LED_COUNT;
  }
  return iN;
}

//---FIND ADJACENT INDEX CLOCKWISE
int adjacent_cw(int i) {
  int r;
  if (i < LED_COUNT - 1) {
    r = i + 1;
  }
  else {
    r = 0;
  }
  return r;
}

//---FIND ADJACENT INDEX COUNTER-CLOCKWISE
int adjacent_ccw(int i) {
  int r;
  if (i > 0) {
    r = i - 1;
  }
  else {
    r = LED_COUNT - 1;
  }
  return r;
}

void copy_led_array() {
  for (int i = 0; i < LED_COUNT; i++ ) {
    ledsX[i][0] = leds[i].r;
    ledsX[i][1] = leds[i].g;
    ledsX[i][2] = leds[i].b;
    ledsX[i][3] = leds[i].w;
  }
}

void setPixel(int Pixel, byte red, byte green, byte blue) {
  leds[Pixel].r = 0;
  leds[Pixel].g = red;
  leds[Pixel].b = blue;
  leds[Pixel].w = green;
}

void setAll(byte red, byte green, byte blue) {
  for (int i = 0; i < LED_COUNT; i++ ) {
    setPixel(i, red, green, blue);
  }
  FastLED.show();
}

// HSV->RGB conversion based on GLSL version
// expects hsv channels defined in 0.0 .. 1.0 interval
float fract(float x) { return x - int(x); }

float mix(float a, float b, float t) { return a + (b - a) * t; }

float step(float e, float x) { return x < e ? 0.0 : 1.0; }

float* hsv2rgb(float h, float s, float b, float* rgb) {
  rgb[0] = b * mix(1.0, constrain(abs(fract(h + 1.0) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  rgb[1] = b * mix(1.0, constrain(abs(fract(h + 0.6666666) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  rgb[2] = b * mix(1.0, constrain(abs(fract(h + 0.3333333) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  return rgb;
}
/*
void setColor(float i, float *rgb) {
  leds[i] = CRGBW(rgb[0], rgb[1], rgb[2], 0);
  //analogWrite(redPin, (int)((1.0 - rgb[0]) * 255));
  //analogWrite(greenPin, (int)((1.0 - rgb[1]) * 255));
  //analogWrite(bluePin, (int)((1.0 - rgb[2]) * 255));  
}*/
