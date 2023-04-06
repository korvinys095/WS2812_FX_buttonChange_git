/*
  Скетч создан на основе FASTSPI2 EFFECTS EXAMPLES автора teldredge (www.funkboxing.com)
  А также вот этой статьи https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#cylon
  Доработан, переведён и разбит на файлы 2017 AlexGyver
  Смена выбранных режимов кнопкой. Кнопка подключена на D2 и GND
*/

#include "FastLED.h"          // библиотека для работы с лентой
#include "FastLED_RGBW.h"

#define LED_COUNT 30          // число светодиодов в кольце/ленте
#define LED_DT   3           // пин, куда подключен DIN ленты

int max_bright = 255;          // максимальная яркость (0 - 255)
boolean adapt_light = 0;       // адаптивная подсветка (1 - включить, 0 - выключить)
int Brightness;
//byte fav_modes[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 101, 102, 103, 104, 105, 106, 888, 889};  // список "любимых" режимов
//byte fav_modes[] = {1, 6, 7, 101, 102, 103, 104, 105, 106};  // список "любимых" режимов
byte fav_modes[] = {1, 2, 3, 6, 7, 8, 12, 14, 15, 16, 23, 25, 27, 30, 34, 101, 102, 103, 104, 105, 106};
//byte fav_modes[] = {1, 3, 5, 14, 15};  // список "любимых" режимов
//byte fav_modes[] = {101, 102, 103, 104, 105, 106};  // список "любимых" режимов

byte num_modes = sizeof(fav_modes);         // получить количество "любимых" режимов (они все по 1 байту..)
unsigned long change_time, last_change, last_bright;
int new_bright;
float col[3];
volatile byte ledMode = 0;
/*
  Стартовый режим
  0 - все выключены
  1 - все включены
  3 - кольцевая радуга
  888 - демо-режим
*/

// ----- настройка ИК пульта
#define REMOTE_TYPE 0       // 0 - без пульта, 1 - пульт 
// система может работать С ЛЮБЫМ ИК ПУЛЬТОМ (практически). Коды для своего пульта можно задать начиная со строки 160 в прошивке. Коды пультов определяются скетчем IRtest_2.0, читай инструкцию

#define BTN_PIN 5          // кнопка переключения режимов (PIN --- КНОПКА --- GND)
#define BTN_PIN1 7          // кнопка переключения режимов (PIN --- КНОПКА --- GND)
#define BTN_PIN2 9          // кнопка переключения режимов (PIN --- КНОПКА --- GND)
#define IR_PIN 12                // пин ИК приёмника

#include "GyverButton.h"
GButton butt1(BTN_PIN);
GButton butt2(BTN_PIN1);
GButton butt3(BTN_PIN2);

#include "IRLremote.h"
CHashIR IRLremote;
uint32_t IRdata;

// цвета мячиков для режима
byte ballColors[3][3] = {
  {0xff, 0, 0},
  {0xff, 0xff, 0xff},
  {0   , 0   , 0xff}
};

// ---------------СЛУЖЕБНЫЕ ПЕРЕМЕННЫЕ-----------------
int BOTTOM_INDEX = 0;        // светодиод начала отсчёта
int TOP_INDEX = int(LED_COUNT / 2);
int EVENODD = LED_COUNT % 2;
//struct CRGB leds[LED_COUNT];
CRGBW leds[LED_COUNT];
CRGB *ledsRGB = (CRGB *) &leds[0];
int ledsX[LED_COUNT][3];     //-ARRAY FOR COPYING WHATS IN THE LED STRIP CURRENTLY (FOR CELL-AUTOMATA, MARCH, ETC)

int thisdelay = 20;          //-FX LOOPS DELAY VAR
int thisstep = 10;           //-FX LOOPS DELAY VAR
int thishue = 0;             //-FX LOOPS DELAY VAR
int thissat = 255;           //-FX LOOPS DELAY VAR

int thisindex = 0;
int thisRED = 0;
int thisGRN = 0;
int thisBLU = 0;

int idex = 0;                //-LED INDEX (0 to LED_COUNT-1
int ihue = 0;                //-HUE (0-255)
int ibright = 0;             //-BRIGHTNESS (0-255)
int isat = 0;                //-SATURATION (0-255)
int bouncedirection = 0;     //-SWITCH FOR COLOR BOUNCE (0-1)
float tcount = 0.0;          //-INC VAR FOR SIN LOOPS
int lcount = 0;              //-ANOTHER COUNTING VAR

volatile uint32_t btnTimer;
volatile byte modeCounter;
volatile byte SwitchOff = 1;
volatile boolean changeFlag;
// ---------------СЛУЖЕБНЫЕ ПЕРЕМЕННЫЕ-----------------


void setup()
{
  Serial.begin(9600);              // открыть порт для связи
  LEDS.setBrightness(max_bright);  // ограничить максимальную яркость
  Serial.println(num_modes);
  //LEDS.addLeds<WS2812B, LED_DT, GRB>(leds, LED_COUNT);  // настрйоки для нашей ленты (ленты на WS2811, WS2812, WS2812B)
  FastLED.addLeds<WS2812B, LED_DT, RGB>(ledsRGB, getRGBWsize(LED_COUNT));
  one_color_all(0, 0, 0, 0);          // погасить все светодиоды
  LEDS.show();                     // отослать команду

  randomSeed(analogRead(0));
  //pinMode(12, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(12), btnISR, FALLING);
  //attachInterrupt(digitalPinToInterrupt(5), buttonTick, FALLING);
  butt1.setTimeout(900);
  butt2.setTimeout(900);
  butt3.setTimeout(900);
  IRLremote.begin(IR_PIN);
}

void one_color_all(int cred, int cgrn, int cblu, int cwht) {       //-SET ALL LEDS TO ONE COLOR
  for (int i = 0 ; i < LED_COUNT; i++ ) {
    leds[i].setRGBW( cred, cgrn, cblu, cwht);
  }
}

void loop() {
  if (adapt_light) {                        // если включена адаптивная яркость
    if (millis() - last_bright > 500) {     // каждые полсекунды
      last_bright = millis();               // сброить таймер
      new_bright = map(analogRead(6), 1, 1000, 3, max_bright);   // считать показания с фоторезистора, перевести диапазон
      LEDS.setBrightness(new_bright);        // установить новую яркость
    }
  }


  buttonTick();     // опрос и обработка кнопки
#if REMOTE_TYPE != 0
  remoteTick();     // опрос ИК пульта
#endif
  
/*if (Serial.available() > 0) {     // если что то прислали
    ledMode = Serial.parseInt();    // парсим в тип данных int
    change_mode(ledMode);           // меняем режим через change_mode (там для каждого режима стоят цвета и задержки)
  }*/
  
  switch (ledMode) {
    case 999: break;                           // пазуа
    case  2: rainbow_fade(); break;            // плавная смена цветов всей ленты
    case  3: rainbow_loop(); break;            // крутящаяся радуга
    case  4: random_burst(); break;            // случайная смена цветов
    case  5: color_bounce(); break;            // бегающий светодиод
    case  6: color_bounceFADE(); break;        // бегающий паровозик светодиодов
    case  7: ems_lightsONE(); break;           // вращаются красный и синий
    case  8: ems_lightsALL(); break;           // вращается половина красных и половина синих
    case  9: flicker(); break;                 // случайный стробоскоп
    case 10: pulse_one_color_all(); break;     // пульсация одним цветом
    case 11: pulse_one_color_all_rev(); break; // пульсация со сменой цветов
    case 12: fade_vertical(); break;           // плавная смена яркости по вертикали (для кольца)
    case 13: rule30(); break;                  // безумие красных светодиодов
    case 14: random_march(); break;            // безумие случайных цветов
    case 15: rwb_march(); break;               // белый синий красный бегут по кругу (ПАТРИОТИЗМ!)
    case 16: radiation(); break;               // пульсирует значок радиации
    case 17: color_loop_vardelay(); break;     // красный светодиод бегает по кругу
    case 18: white_temps(); break;             // бело синий градиент (?)
    case 19: sin_bright_wave(); break;         // тоже хрень какая то
    case 20: pop_horizontal(); break;          // красные вспышки спускаются вниз
    case 21: quad_bright_curve(); break;       // полумесяц
    //case 22: flame(); break;                   // эффект пламени
    case 23: rainbow_vertical(); break;        // радуга в вертикаьной плоскости (кольцо)
    case 24: pacman(); break;                  // пакман
    case 25: random_color_pop(); break;        // безумие случайных вспышек
    case 26: ems_lightsSTROBE(); break;        // полицейская мигалка
    case 27: rgb_propeller(); break;           // RGB пропеллер
    case 28: kitt(); break;                    // случайные вспышки красного в вертикаьной плоскости
    case 29: matrix(); break;                  // зелёненькие бегают по кругу случайно
    case 30: new_rainbow_loop(); break;        // крутая плавная вращающаяся радуга
    case 31: strip_march_ccw(); break;         // чёт сломалось
    case 32: strip_march_cw(); break;          // чёт сломалось
    //case 33: colorWipe(0x00, 0xff, 0x00, thisdelay);
    //  colorWipe(0x00, 0x00, 0x00, thisdelay); break;                                // плавное заполнение цветом
    case 34: CylonBounce(0xff, 0, 0, 4, 10, thisdelay); break;                      // бегающие светодиоды
    case 35: Fire(55, 120, thisdelay); break;                                       // линейный огонь
    //case 36: NewKITT(0xff, 0, 0, 8, 10, thisdelay); break;                          // беготня секторов круга (не работает)
    case 37: rainbowCycle(thisdelay); break;                                        // очень плавная вращающаяся радуга
    case 38: TwinkleRandom(20, thisdelay, 1); break;                                // случайные разноцветные включения (1 - танцуют все, 0 - случайный 1 диод)
    case 39: RunningLights(0xff, 0xff, 0x00, thisdelay); break;                     // бегущие огни
    case 40: Sparkle(0xff, 0xff, 0xff, thisdelay); break;                           // случайные вспышки белого цвета
    case 41: SnowSparkle(0x10, 0x10, 0x10, thisdelay, random(100, 1000)); break;    // случайные вспышки белого цвета на белом фоне
    case 42: theaterChase(0xff, 0, 0, thisdelay); break;                            // бегущие каждые 3 (ЧИСЛО СВЕТОДИОДОВ ДОЛЖНО БЫТЬ КРАТНО 3)
    case 43: theaterChaseRainbow(thisdelay); break;                                 // бегущие каждые 3 радуга (ЧИСЛО СВЕТОДИОДОВ ДОЛЖНО БЫТЬ КРАТНО 3)
    case 44: Strobe(0xff, 0xff, 0xff, 10, thisdelay, 1000); break;                  // стробоскоп

    case 45: BouncingBalls(0xff, 0, 0, 3); break;                                   // прыгающие мячики
    case 46: BouncingColoredBalls(3, ballColors); break;                            // прыгающие мячики цветные

    //case 888: demo_modeA(); break;             // длинное демо
    //case 889: demo_modeB(); break;             // короткое демо
  }
}

void btnISR() {
  if (millis() - btnTimer > 300) {
    btnTimer = millis();  // защита от дребезга
    if (++modeCounter >= num_modes) modeCounter = 0;
    ledMode = fav_modes[modeCounter];    // получаем новый номер следующего режима
    change_mode(ledMode);               // меняем режим через change_mode (там для каждого режима стоят цвета и задержки)    
    changeFlag = true;
    Serial.println(ledMode);
  }
}
void buttonTick() {
  butt1.tick();  // обязательная функция отработки. Должна постоянно опрашиваться
  if (butt1.isSingle()) {                              // если единичное нажатие
    if (++modeCounter >= num_modes) modeCounter = 0;
    ledMode = fav_modes[modeCounter];    // получаем новый номер следующего режима
    change_mode(ledMode);               // меняем режим через change_mode (там для каждого режима стоят цвета и задержки)    
    changeFlag = true;
    Serial.println(F("buttn1"));
    Serial.println(ledMode);
  }
  if (butt1.isHolded()) {     // кнопка удержана
    //fullLowPass();
  }
  butt2.tick();  // обязательная функция отработки. Должна постоянно опрашиваться
  if (butt2.isSingle()) {                              // если единичное нажатие
    if (modeCounter > 0) {  
      --modeCounter;      
      ledMode = fav_modes[modeCounter];    // получаем новый номер следующего режима
      change_mode(ledMode);               // меняем режим через change_mode (там для каждого режима стоят цвета и задержки)    
      changeFlag = true;
      Serial.println(F("buttn2"));
      Serial.println(ledMode);
    }
    else if (modeCounter == 0) {
      modeCounter = num_modes;
      --modeCounter;
      ledMode = fav_modes[modeCounter];    // получаем новый номер следующего режима
      change_mode(ledMode);               // меняем режим через change_mode (там для каждого режима стоят цвета и задержки)    
      changeFlag = true;
      Serial.println(F("buttn2"));
      Serial.println(ledMode);
    }
  }
  if (butt2.isHolded()) {     // кнопка удержана
    //fullLowPass();
  }
  butt3.tick();  // обязательная функция отработки. Должна постоянно опрашиваться
  if (butt3.isSingle()) {                              // если единичное нажатие
    if (SwitchOff > 0) {
    SwitchOff = 0;
    change_mode(0);               // Выключаем    
    changeFlag = true;
    Serial.println(F("buttn3"));
    Serial.println(F("SwitchOff = 0"));
    }
    else if (SwitchOff == 0) {
    SwitchOff = 1;
    ledMode = fav_modes[modeCounter];    // получаем новый номер следующего режима
    change_mode(ledMode);               // меняем режим через change_mode (там для каждого режима стоят цвета и задержки)   
    changeFlag = true;
    Serial.println(F("buttn3"));
    Serial.println(F("SwitchOff = 1"));
    }
  }
  if (butt3.isHolded()) {     // кнопка удержана
    //fullLowPass();
  }
}

void change_mode(int newmode) {
  thissat = 255;
  switch (newmode) {
    case 0: one_color_all(0, 0, 0, 0); LEDS.show(); break; //---ALL OFF
    case 1: one_color_all(255, 255, 255, 0); LEDS.show(); break; //---ALL ON
    case 2: thisdelay = 60; break;                      //---STRIP RAINBOW FADE
    case 3: thisdelay = 20; thisstep = 10; break;       //---RAINBOW LOOP
    case 4: thisdelay = 20; break;                      //---RANDOM BURST
    case 5: thisdelay = 20; thishue = 0; break;         //---CYLON v1
    case 6: thisdelay = 80; thishue = 0; break;         //---CYLON v2
    case 7: thisdelay = 40; thishue = 0; break;         //---POLICE LIGHTS SINGLE
    case 8: thisdelay = 40; thishue = 0; break;         //---POLICE LIGHTS SOLID
    case 9: thishue = 160; thissat = 50; break;         //---STRIP FLICKER
    case 10: thisdelay = 15; thishue = 0; break;        //---PULSE COLOR BRIGHTNESS
    case 11: thisdelay = 30; thishue = 0; break;        //---PULSE COLOR SATURATION
    case 12: thisdelay = 60; thishue = 180; break;      //---VERTICAL SOMETHING
    case 13: thisdelay = 100; break;                    //---CELL AUTO - RULE 30 (RED)
    case 14: thisdelay = 80; break;                     //---MARCH RANDOM COLORS
    case 15: thisdelay = 80; break;                     //---MARCH RWB COLORS
    case 16: thisdelay = 60; thishue = 95; break;       //---RADIATION SYMBOL
    //---PLACEHOLDER FOR COLOR LOOP VAR DELAY VARS
    case 19: thisdelay = 35; thishue = 180; break;      //---SIN WAVE BRIGHTNESS
    case 20: thisdelay = 100; thishue = 0; break;       //---POP LEFT/RIGHT
    case 21: thisdelay = 100; thishue = 180; break;     //---QUADRATIC BRIGHTNESS CURVE
    //---PLACEHOLDER FOR FLAME VARS
    case 23: thisdelay = 50; thisstep = 15; break;      //---VERITCAL RAINBOW
    case 24: thisdelay = 50; break;                     //---PACMAN
    case 25: thisdelay = 35; break;                     //---RANDOM COLOR POP
    case 26: thisdelay = 25; thishue = 0; break;        //---EMERGECNY STROBE
    case 27: thisdelay = 100; thishue = 0; break;        //---RGB PROPELLER
    case 28: thisdelay = 100; thishue = 0; break;       //---KITT
    case 29: thisdelay = 100; thishue = 95; break;       //---MATRIX RAIN
    case 30: thisdelay = 15; break;                      //---NEW RAINBOW LOOP
    case 31: thisdelay = 100; break;                    //---MARCH STRIP NOW CCW
    case 32: thisdelay = 100; break;                    //---MARCH STRIP NOW CCW
    case 33: thisdelay = 50; break;                     // colorWipe
    case 34: thisdelay = 50; break;                     // CylonBounce
    case 35: thisdelay = 15; break;                     // Fire
    case 36: thisdelay = 50; break;                     // NewKITT
    case 37: thisdelay = 20; break;                     // rainbowCycle
    case 38: thisdelay = 10; break;                     // rainbowTwinkle
    case 39: thisdelay = 50; break;                     // RunningLights
    case 40: thisdelay = 0; break;                      // Sparkle
    case 41: thisdelay = 30; break;                     // SnowSparkle
    case 42: thisdelay = 50; break;                     // theaterChase
    case 43: thisdelay = 50; break;                     // theaterChaseRainbow
    case 44: thisdelay = 100; break;                    // Strobe

    case 101: one_color_all(255, 0, 0, 0); LEDS.show(); break; //---ALL RED
    case 102: one_color_all(0, 255, 0, 0); LEDS.show(); break; //---ALL GREEN
    case 103: one_color_all(0, 0, 255, 0); LEDS.show(); break; //---ALL BLUE
    //case 104: one_color_all(0, 0, 0, 255); LEDS.show(); break; //---ALL BLUE
    case 104: one_color_all(255, 255, 0, 0); LEDS.show(); break; //---ALL COLOR X
    case 105: one_color_all(0, 255, 255, 0); LEDS.show(); break; //---ALL COLOR Y
    case 106: one_color_all(255, 0, 255, 0); LEDS.show(); break; //---ALL COLOR Z
  }
  bouncedirection = 0;
  one_color_all(0, 0, 0, 0);
  ledMode = newmode;
}


#if REMOTE_TYPE != 0
void remoteTick() {
  if (IRLremote.available())  {
    auto data = IRLremote.read();
    IRdata = data.command;
    ir_flag = true;
  }
  if (ir_flag) { // если данные пришли
    switch (IRdata) {
      // режимы
      case BUTT_LEFT: if (++modeCounter >= num_modes) modeCounter = 0;
        ledMode = fav_modes[modeCounter];    // получаем новый номер следующего режима
        change_mode(ledMode);               // меняем режим через change_mode (там для каждого режима стоят цвета и задержки)    
        changeFlag = true;
      
      
      case BUTT_RIGHT: if (--modeCounter == 0) modeCounter = num_modes;
        ledMode = fav_modes[modeCounter];    // получаем новый номер следующего режима
        change_mode(ledMode);               // меняем режим через change_mode (там для каждого режима стоят цвета и задержки)    
        changeFlag = true;

      case BUTT_MINUS: if (--new_bright <= 0) new_bright = 0;
        Brightness = new_bright*25;
        LEDS.setBrightness(Brightness);        // установить новую яркость
        break;

      case BUTT_PLUS: if (++new_bright => 10) new_bright = 10;
        Brightness = new_bright*25;
        LEDS.setBrightness(Brightness);        // установить новую яркость
        break;

      case BUTT_1: this_mode = 0;
        break;
      case BUTT_2: this_mode = 1;
        break;
      case BUTT_3: this_mode = 2;
        break;
      case BUTT_4: this_mode = 3;
        break;
      case BUTT_5: this_mode = 4;
        break;
      case BUTT_6: this_mode = 5;
        break;
      case BUTT_7: this_mode = 6;
        break;
      case BUTT_8: this_mode = 7;
        break;
      case BUTT_9: this_mode = 8;
        break;
      case BUTT_0: fullLowPass();
        break;
    }
    ir_flag = false;
  }
}
#endif

// ----- КНОПКИ ПУЛЬТА WAVGAT -----
#if REMOTE_TYPE == 1
#define BUTT_CH_MN     0xF39EEBAD
#define BUTT_CH_PL   0xC089F6AD
#define BUTT_CH   0xC089F6AD
#define BUTT_LEFT   0xC089F6AD //
#define BUTT_RIGHT   0xC089F6AD //
#define BUTT_PLAY   0xC089F6AD
#define BUTT_MINUS   0xE25410AD //
#define BUTT_PLUS  0x14CE54AD //
#define BUTT_EQ     0x297C76AD
#define BUTT_100     0x297C76AD
#define BUTT_200     0x297C76AD
#define BUTT_1      0x4E5BA3AD
#define BUTT_2      0xE51CA6AD
#define BUTT_3      0xE207E1AD
#define BUTT_4      0x517068AD
#define BUTT_5      0x1B92DDAD
#define BUTT_6      0xAC2A56AD
#define BUTT_7      0x5484B6AD
#define BUTT_8      0xD22353AD
#define BUTT_9      0xDF3F4BAD
#define BUTT_0      0xF08A26AD

#endif

