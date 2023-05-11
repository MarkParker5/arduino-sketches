#include "FastLED.h"

#define FHT_N 64         // ширина спектра х2
#define LOG_OUT 1
#include <FHT.h>         // преобразование Хартли

// ***************************** LED *******************************************


// ---------------------  Settings  --------------------------------------------
int max_bright = 255;                    // максимальная яркость (0 - 255)
boolean adapt_light = 0;             // адаптивная подсветка (1 - включить, 0 - выключить)

byte fav_modes[] = {2, 3, 4, 18, 22, 23, 24, 27, 30, 33, 37, 39, 42, 43};    // список "любимых" режимов
int ledMode = 33;
byte num_modes = sizeof(fav_modes);                 // получить количество "любимых" режимов (они все по 1 байту..)

int current_mode = 0; // 0 = off, 1 = led, 2 = colormusic

int durations[] = {30, 50, 20, 80, 45};
int duration_index = 0;
// ---------------------  /Settings  -------------------------------------------

// ---------------          СЛУЖЕБНЫЕ ПЕРЕМЕННЫЕ         -----------------------
#define LED_COUNT 57        // количество светодиодов (данная версия поддерживает до 410 штук)
#define LED_PIN 3            // пин DI светодиодной ленты

int BOTTOM_INDEX = 0;                // светодиод начала отсчёта
int TOP_INDEX = int(LED_COUNT / 2);
int EVENODD = LED_COUNT % 2;
int ledsX[LED_COUNT][3];         //-ARRAY FOR COPYING WHATS IN THE LED STRIP CURRENTLY (FOR CELL-AUTOMATA, MARCH, ETC)

int thisdelay = 20;                    //-FX LOOPS DELAY VAR
int thisstep = 10;                     //-FX LOOPS DELAY VAR
int thishue = 0;                         //-FX LOOPS DELAY VAR
int thissat = 255;                     //-FX LOOPS DELAY VAR

int thisindex = 0;
int thisRED = 0;
int thisGRN = 0;
int thisBLU = 0;

int idex = 0;                                //-LED INDEX (0 to LED_COUNT-1
    int ihue = 0;                                //-HUE (0-255)
    int ibright = 0;                         //-BRIGHTNESS (0-255)
    int isat = 0;                                //-SATURATION (0-255)
    int bouncedirection = 0;         //-SWITCH FOR COLOR BOUNCE (0-1)
    float tcount = 0.0;                    //-INC VAR FOR SIN LOOPS
    int lcount = 0;                            //-ANOTHER COUNTING VAR

    String cmd;
    // ---------------      /СЛУЖЕБНЫЕ ПЕРЕМЕННЫЕ      -----------------------------

    unsigned long change_time, last_change, last_bright;
    int new_bright;

    // цвета мячиков для режима
    byte ballColors[3][3] = {
        {0xff, 0, 0},
        {0xff, 0xff, 0xff},
        {0     , 0     , 0xff}
    };

    //    ==========================================================================

    // ***************************** COLORMUSIC ************************************

    // ----- настройки ленты
    #define CURRENT_LIMIT 3000  // лимит по току в МИЛЛИАМПЕРАХ, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит
    byte BRIGHTNESS = 200;      // яркость по умолчанию (0 - 255)

    // ----- пины подключения
    #define SOUND_R A1              // аналоговый пин вход аудио, правый канал
    #define SOUND_L A2              // аналоговый пин вход аудио, левый канал
    #define SOUND_R_FREQ A3         // аналоговый пин вход аудио для режима с частотами (через кондер)
    #define POT_GND A0              // пин земля для потенциометра

    // ----- настройки радуги
    float RAINBOW_STEP = 5.00;        // шаг изменения цвета радуги

    // ----- отрисовка
    #define MODE 0                    // режим при запуске
    #define MAIN_LOOP 5               // период основного цикла отрисовки (по умолчанию 5)

    // ----- сигнал
    #define MONO 0                    // 1 - только один канал (ПРАВЫЙ!!!!! SOUND_R!!!!!), 0 - два канала
    #define EXP 1.4                   // степень усиления сигнала (для более "резкой" работы) (по умолчанию 1.4)
    #define POTENT 0                  // 1 - используем потенциометр, 0 - используется внутренний источник опорного напряжения 1.1 В
    byte EMPTY_BRIGHT = 30;           // яркость "не горящих" светодиодов (0 - 255)
    #define EMPTY_COLOR HUE_PURPLE    // цвет "не горящих" светодиодов. Будет чёрный, если яркость 0

    // ----- нижний порог шумов
    uint16_t LOW_PASS = 100;          // нижний порог шумов режим VU, ручная настройка
    uint16_t SPEKTR_LOW_PASS = 40;    // нижний порог шумов режим спектра, ручная настройка
    #define AUTO_LOW_PASS 0           // разрешить настройку нижнего порога шумов при запуске (по умолч. 0)
    #define LOW_PASS_ADD 13           // "добавочная" величина к нижнему порогу, для надёжности (режим VU)
    #define LOW_PASS_FREQ_ADD 3       // "добавочная" величина к нижнему порогу, для надёжности (режим частот)

    // ----- режим шкала громкости
    float SMOOTH = 0.3;               // коэффициент плавности анимации VU (по умолчанию 0.5)
    #define MAX_COEF 1.8              // коэффициент громкости (максимальное равно срднему * этот коэф) (по умолчанию 1.8)

    // ----- режим цветомузыки
    float SMOOTH_FREQ = 0.8;          // коэффициент плавности анимации частот (по умолчанию 0.8)
    float MAX_COEF_FREQ = 1.2;        // коэффициент порога для "вспышки" цветомузыки (по умолчанию 1.5)
    #define SMOOTH_STEP 20            // шаг уменьшения яркости в режиме цветомузыки (чем больше, тем быстрее гаснет)
    #define LOW_COLOR HUE_RED         // цвет низких частот
    #define MID_COLOR HUE_GREEN       // цвет средних
    #define HIGH_COLOR HUE_YELLOW     // цвет высоких

    // ----- режим стробоскопа
    uint16_t STROBE_PERIOD = 140;     // период вспышек, миллисекунды
    #define STROBE_DUTY 20            // скважность вспышек (1 - 99) - отношение времени вспышки ко времени темноты
    #define STROBE_COLOR HUE_YELLOW   // цвет стробоскопа
    #define STROBE_SAT 0              // насыщенность. Если 0 - цвет будет БЕЛЫЙ при любом цвете (0 - 255)
    byte STROBE_SMOOTH = 200;         // скорость нарастания/угасания вспышки (0 - 255)

    // ----- режим подсветки
    byte LIGHT_COLOR = 0;             // начальный цвет подсветки
    byte LIGHT_SAT = 255;             // начальная насыщенность подсветки
    byte COLOR_SPEED = 100;
    int RAINBOW_PERIOD = 1;
    float RAINBOW_STEP_2 = 0.5;

    // ----- режим бегущих частот
    byte RUNNING_SPEED = 11;

    // ----- режим анализатора спектра
    byte HUE_START = 0;
    byte HUE_STEP = 5;
    #define LIGHT_SMOOTH 2

    // ------------------------------ ДЛЯ РАЗРАБОТЧИКОВ ----------------------------
    #define MODE_AMOUNT 9      // количество режимов

    #define STRIPE LED_COUNT / 5
    float freq_to_stripe = LED_COUNT / 40; // /2 так как симметрия, и /20 так как 20 частот

    #define FASTLED_ALLOW_INTERRUPTS 1
    CRGB leds[LED_COUNT];

    // градиент-палитра от зелёного к красному
    DEFINE_GRADIENT_PALETTE(soundlevel_gp) {
        0,    0,    255,  0,  // green
        100,  255,  255,  0,  // yellow
        150,  255,  100,  0,  // orange
        200,  255,  50,   0,  // red
        255,  255,  0,    0   // red
    };
    CRGBPalette32 myPal = soundlevel_gp;

    int Rlenght, Llenght;
    float RsoundLevel, RsoundLevel_f;
    float LsoundLevel, LsoundLevel_f;

    float averageLevel = 50;
    int maxLevel = 100;
    int MAX_CH = LED_COUNT / 2;
    int hue;
    unsigned long main_timer, hue_timer, strobe_timer, running_timer, color_timer, rainbow_timer;
    float averK = 0.006;
    byte count;
    float index = (float)255 / MAX_CH;   // коэффициент перевода для палитры
    boolean lowFlag;
    byte low_pass;
    int RcurrentLevel, LcurrentLevel;
    int colorMusic[3];
    float colorMusic_f[3], colorMusic_aver[3];
    boolean colorMusicFlash[3], strobeUp_flag, strobeDwn_flag;
    byte this_mode = MODE;
    int thisBright[3], strobe_bright = 0;
    unsigned int light_time = STROBE_PERIOD * STROBE_DUTY / 100;
    boolean settings_mode, ONstate = true;
    int8_t freq_strobe_mode, light_mode;
    int freq_max;
    float freq_max_f, rainbow_steps;
    int freq_f[32];
    int this_color;
    boolean running_flag[3];

    #define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
    #define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
    // ------------------------------ ДЛЯ РАЗРАБОТЧИКОВ ----------------------------

    void setup() {
        Serial.begin(9600);
        FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, LED_COUNT).setCorrection( TypicalLEDStrip );
        if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
        FastLED.setBrightness(BRIGHTNESS);
        one_color_all(0, 0, 0);                             // погасить все светодиоды
        LEDS.show();                                        // отослать команду

        pinMode(POT_GND, OUTPUT);
        digitalWrite(POT_GND, LOW);

        randomSeed(analogRead(0));

        // для увеличения точности уменьшаем опорное напряжение,
        // выставив EXTERNAL и подключив Aref к выходу 3.3V на плате через делитель
        // GND ---[10-20 кОм] --- REF --- [10 кОм] --- 3V3
        // в данной схеме GND берётся из А0 для удобства подключения
        if (POTENT) analogReference(EXTERNAL);
        else
        analogReference(INTERNAL);

        // жуткая магия, меняем частоту оцифровки до 18 кГц
        // команды на ебучем ассемблере, даже не спрашивайте, как это работает
        // поднимаем частоту опроса аналогового порта до 38.4 кГц, по теореме
        // Котельникова (Найквиста) частота дискретизации будет 19.2 кГц
        // http://yaab-arduino.blogspot.ru/2015/02/fast-sampling-from-analog-input.html
        sbi(ADCSRA, ADPS2);
        cbi(ADCSRA, ADPS1);
        sbi(ADCSRA, ADPS0);

        if (AUTO_LOW_PASS) {         // если разрешена автонастройка нижнего порога шумов
            autoLowPass();
        }
    }

    void loop() {
        if (Serial.available()) {
            cmd = Serial.readString();
            start();
            //Serial.print("CMD: ");Serial.println(cmd);
        }
        Serial.println(analogRead(SOUND_R));
        switch(current_mode) {
            case 0: break;
            case 1: led_loop(); break;
            case 2: colormusic(); break;
        }
    }

    //    ==========================================================================

    void start() {
        Serial.println(cmd);
        cmd.trim();
        if(cmd == "turn_off") turn_off();
        else if(cmd == "turn_on") turn_on();
        else if(cmd == "toggle") toggle();
        else if(cmd == "led_hello") hello();
        else if(cmd == "colormusic") colormusic();
        cmd = "";
    }

    void setmode(int mode) {
        current_mode = mode;
    }

    void turn_off() {
        setmode(0);
        one_color_all(0, 0, 0);
        LEDS.show();
    }

    void turn_on() {
        setmode(1);
    }

    void toggle() {
        setmode(current_mode ? 0 : 1);
    }

    void colormusic() {
        setmode(2);
    }

    void hello() {
        for(int i = 0; i < 5; i++) {
            colorWipe(0x00, 0xff, 0x00, 10);
            colorWipe(0x00, 0x00, 0x00, 10);
        }
    }

    //    ==========================================================================

    void colormusic_loop() {
        mainLoop();       // главный цикл обработки и отрисовки
    }

    void mainLoop() {
        // главный цикл отрисовки
        if (ONstate) {
            if (millis() - main_timer > MAIN_LOOP) {
                // сбрасываем значения
                RsoundLevel = 0;
                LsoundLevel = 0;

                // перваые два режима - громкость (VU meter)
                if (this_mode == 0 || this_mode == 1) {
                    for (byte i = 0; i < 100; i ++) {                                 // делаем 100 измерений
                        RcurrentLevel = analogRead(SOUND_R);                            // с правого
                        if (!MONO) LcurrentLevel = analogRead(SOUND_L);                 // и левого каналов

                        if (RsoundLevel < RcurrentLevel) RsoundLevel = RcurrentLevel;   // ищем максимальное
                        if (!MONO) if (LsoundLevel < LcurrentLevel) LsoundLevel = LcurrentLevel;   // ищем максимальное
                    }

                    // фильтруем по нижнему порогу шумов
                    RsoundLevel = map(RsoundLevel, LOW_PASS, 1023, 0, 500);
                    if (!MONO)LsoundLevel = map(LsoundLevel, LOW_PASS, 1023, 0, 500);

                    // ограничиваем диапазон
                    RsoundLevel = constrain(RsoundLevel, 0, 500);
                    if (!MONO)LsoundLevel = constrain(LsoundLevel, 0, 500);

                    // возводим в степень (для большей чёткости работы)
                    RsoundLevel = pow(RsoundLevel, EXP);
                    if (!MONO)LsoundLevel = pow(LsoundLevel, EXP);

                    // фильтр
                    RsoundLevel_f = RsoundLevel * SMOOTH + RsoundLevel_f * (1 - SMOOTH);
                    if (!MONO)LsoundLevel_f = LsoundLevel * SMOOTH + LsoundLevel_f * (1 - SMOOTH);

                    if (MONO) LsoundLevel_f = RsoundLevel_f;  // если моно, то левый = правому

                    // заливаем "подложку", если яркость достаточная
                    if (EMPTY_BRIGHT > 5) {
                        for (int i = 0; i < LED_COUNT; i++)
                        leds[i] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
                    }

                    // если значение выше порога - начинаем самое интересное
                    if (RsoundLevel_f > 15 && LsoundLevel_f > 15) {

                        // расчёт общей средней громкости с обоих каналов, фильтрация.
                        // Фильтр очень медленный, сделано специально для автогромкости
                        averageLevel = (float)(RsoundLevel_f + LsoundLevel_f) / 2 * averK + averageLevel * (1 - averK);

                        // принимаем максимальную громкость шкалы как среднюю, умноженную на некоторый коэффициент MAX_COEF
                        maxLevel = (float)averageLevel * MAX_COEF;

                        // преобразуем сигнал в длину ленты (где MAX_CH это половина количества светодиодов)
                        Rlenght = map(RsoundLevel_f, 0, maxLevel, 0, MAX_CH);
                        Llenght = map(LsoundLevel_f, 0, maxLevel, 0, MAX_CH);

                        // ограничиваем до макс. числа светодиодов
                        Rlenght = constrain(Rlenght, 0, MAX_CH);
                        Llenght = constrain(Llenght, 0, MAX_CH);

                        animation();       // отрисовать
                    }
                }

                // 3-5 режим - цветомузыка
                if (this_mode == 2 || this_mode == 3 || this_mode == 4 || this_mode == 7 || this_mode == 8) {
                    analyzeAudio();
                    colorMusic[0] = 0;
                    colorMusic[1] = 0;
                    colorMusic[2] = 0;
                    for (int i = 0 ; i < 32 ; i++) {
                        if (fht_log_out[i] < SPEKTR_LOW_PASS) fht_log_out[i] = 0;
                    }
                    // низкие частоты, выборка со 2 по 5 тон (0 и 1 зашумленные!)
                    for (byte i = 2; i < 6; i++) {
                        if (fht_log_out[i] > colorMusic[0]) colorMusic[0] = fht_log_out[i];
                    }
                    // средние частоты, выборка с 6 по 10 тон
                    for (byte i = 6; i < 11; i++) {
                        if (fht_log_out[i] > colorMusic[1]) colorMusic[1] = fht_log_out[i];
                    }
                    // высокие частоты, выборка с 11 по 31 тон
                    for (byte i = 11; i < 32; i++) {
                        if (fht_log_out[i] > colorMusic[2]) colorMusic[2] = fht_log_out[i];
                    }
                    freq_max = 0;
                    for (byte i = 0; i < 30; i++) {
                        if (fht_log_out[i + 2] > freq_max) freq_max = fht_log_out[i + 2];
                        if (freq_max < 5) freq_max = 5;

                        if (freq_f[i] < fht_log_out[i + 2]) freq_f[i] = fht_log_out[i + 2];
                        if (freq_f[i] > 0) freq_f[i] -= LIGHT_SMOOTH;
                        else freq_f[i] = 0;
                    }
                    freq_max_f = freq_max * averK + freq_max_f * (1 - averK);
                    for (byte i = 0; i < 3; i++) {
                        colorMusic_aver[i] = colorMusic[i] * averK + colorMusic_aver[i] * (1 - averK);  // общая фильтрация
                        colorMusic_f[i] = colorMusic[i] * SMOOTH_FREQ + colorMusic_f[i] * (1 - SMOOTH_FREQ);      // локальная
                        if (colorMusic_f[i] > ((float)colorMusic_aver[i] * MAX_COEF_FREQ)) {
                            thisBright[i] = 255;
                            colorMusicFlash[i] = true;
                            running_flag[i] = true;
                        } else colorMusicFlash[i] = false;
                        if (thisBright[i] >= 0) thisBright[i] -= SMOOTH_STEP;
                        if (thisBright[i] < EMPTY_BRIGHT) {
                            thisBright[i] = EMPTY_BRIGHT;
                            running_flag[i] = false;
                        }
                    }
                    animation();
                }
                if (this_mode == 5) {
                    if ((long)millis() - strobe_timer > STROBE_PERIOD) {
                        strobe_timer = millis();
                        strobeUp_flag = true;
                        strobeDwn_flag = false;
                    }
                    if ((long)millis() - strobe_timer > light_time) {
                        strobeDwn_flag = true;
                    }
                    if (strobeUp_flag) {                    // если настало время пыхнуть
                        if (strobe_bright < 255)              // если яркость не максимальная
                        strobe_bright += STROBE_SMOOTH;     // увелчить
                        if (strobe_bright > 255) {            // если пробили макс. яркость
                            strobe_bright = 255;                // оставить максимум
                            strobeUp_flag = false;              // флаг опустить
                        }
                    }

                    if (strobeDwn_flag) {                   // гаснем
                        if (strobe_bright > 0)                // если яркость не минимальная
                        strobe_bright -= STROBE_SMOOTH;     // уменьшить
                        if (strobe_bright < 0) {              // если пробили мин. яркость
                            strobeDwn_flag = false;
                            strobe_bright = 0;                  // оставить 0
                        }
                    }
                    animation();
                }
                if (this_mode == 6) animation();

                if (this_mode != 7) FastLED.clear();          // очистить массив пикселей
                main_timer = millis();    // сбросить таймер
            }
        }
    }

    void animation() {
        // согласно режиму
        switch (this_mode) {
            case 0:
            count = 0;
            for (int i = (MAX_CH - 1); i > ((MAX_CH - 1) - Rlenght); i--) {
                leds[i] = ColorFromPalette(myPal, (count * index));   // заливка по палитре " от зелёного к красному"
                count++;
            }
            count = 0;
            for (int i = (MAX_CH); i < (MAX_CH + Llenght); i++ ) {
                leds[i] = ColorFromPalette(myPal, (count * index));   // заливка по палитре " от зелёного к красному"
                count++;
            }
            if (EMPTY_BRIGHT > 0) {
                CHSV this_dark = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
                for (int i = ((MAX_CH - 1) - Rlenght); i > 0; i--)
                leds[i] = this_dark;
                for (int i = MAX_CH + Llenght; i < LED_COUNT; i++)
                leds[i] = this_dark;
            }
            break;
            case 1:
            if (millis() - rainbow_timer > 30) {
                rainbow_timer = millis();
                hue = floor((float)hue + RAINBOW_STEP);
            }
            count = 0;
            for (int i = (MAX_CH - 1); i > ((MAX_CH - 1) - Rlenght); i--) {
                leds[i] = ColorFromPalette(RainbowColors_p, (count * index) / 2 - hue);  // заливка по палитре радуга
                count++;
            }
            count = 0;
            for (int i = (MAX_CH); i < (MAX_CH + Llenght); i++ ) {
                leds[i] = ColorFromPalette(RainbowColors_p, (count * index) / 2 - hue); // заливка по палитре радуга
                count++;
            }
            if (EMPTY_BRIGHT > 0) {
                CHSV this_dark = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
                for (int i = ((MAX_CH - 1) - Rlenght); i > 0; i--)
                leds[i] = this_dark;
                for (int i = MAX_CH + Llenght; i < LED_COUNT; i++)
                leds[i] = this_dark;
            }
            break;
            case 2:
            for (int i = 0; i < LED_COUNT; i++) {
                if (i < STRIPE)          leds[i] = CHSV(HIGH_COLOR, 255, thisBright[2]);
                else if (i < STRIPE * 2) leds[i] = CHSV(MID_COLOR, 255, thisBright[1]);
                else if (i < STRIPE * 3) leds[i] = CHSV(LOW_COLOR, 255, thisBright[0]);
                else if (i < STRIPE * 4) leds[i] = CHSV(MID_COLOR, 255, thisBright[1]);
                else if (i < STRIPE * 5) leds[i] = CHSV(HIGH_COLOR, 255, thisBright[2]);
            }
            break;
            case 3:
            for (int i = 0; i < LED_COUNT; i++) {
                if (i < LED_COUNT / 3)          leds[i] = CHSV(HIGH_COLOR, 255, thisBright[2]);
                else if (i < LED_COUNT * 2 / 3) leds[i] = CHSV(MID_COLOR, 255, thisBright[1]);
                else if (i < LED_COUNT)         leds[i] = CHSV(LOW_COLOR, 255, thisBright[0]);
            }
            break;
            case 4:
            switch (freq_strobe_mode) {
                case 0:
                if (colorMusicFlash[2]) HIGHS();
                else if (colorMusicFlash[1]) MIDS();
                else if (colorMusicFlash[0]) LOWS();
                else SILENCE();
                break;
                case 1:
                if (colorMusicFlash[2]) HIGHS();
                else SILENCE();
                break;
                case 2:
                if (colorMusicFlash[1]) MIDS();
                else SILENCE();
                break;
                case 3:
                if (colorMusicFlash[0]) LOWS();
                else SILENCE();
                break;
            }
            break;
            case 5:
            if (strobe_bright > 0)
            for (int i = 0; i < LED_COUNT; i++) leds[i] = CHSV(STROBE_COLOR, STROBE_SAT, strobe_bright);
            else
            for (int i = 0; i < LED_COUNT; i++) leds[i] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
            break;
            case 6:
            switch (light_mode) {
                case 0: for (int i = 0; i < LED_COUNT; i++) leds[i] = CHSV(LIGHT_COLOR, LIGHT_SAT, 255);
                break;
                case 1:
                if (millis() - color_timer > COLOR_SPEED) {
                    color_timer = millis();
                    if (++this_color > 255) this_color = 0;
                }
                for (int i = 0; i < LED_COUNT; i++) leds[i] = CHSV(this_color, LIGHT_SAT, 255);
                break;
                case 2:
                if (millis() - rainbow_timer > 30) {
                    rainbow_timer = millis();
                    this_color += RAINBOW_PERIOD;
                    if (this_color > 255) this_color = 0;
                    if (this_color < 0) this_color = 255;
                }
                rainbow_steps = this_color;
                for (int i = 0; i < LED_COUNT; i++) {
                    leds[i] = CHSV((int)floor(rainbow_steps), 255, 255);
                    rainbow_steps += RAINBOW_STEP_2;
                    if (rainbow_steps > 255) rainbow_steps = 0;
                    if (rainbow_steps < 0) rainbow_steps = 255;
                }
                break;
            }
            break;
            case 7:
            switch (freq_strobe_mode) {
                case 0:
                if (running_flag[2]) leds[LED_COUNT / 2] = CHSV(HIGH_COLOR, 255, thisBright[2]);
                else if (running_flag[1]) leds[LED_COUNT / 2] = CHSV(MID_COLOR, 255, thisBright[1]);
                else if (running_flag[0]) leds[LED_COUNT / 2] = CHSV(LOW_COLOR, 255, thisBright[0]);
                else leds[LED_COUNT / 2] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
                break;
                case 1:
                if (running_flag[2]) leds[LED_COUNT / 2] = CHSV(HIGH_COLOR, 255, thisBright[2]);
                else leds[LED_COUNT / 2] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
                break;
                case 2:
                if (running_flag[1]) leds[LED_COUNT / 2] = CHSV(MID_COLOR, 255, thisBright[1]);
                else leds[LED_COUNT / 2] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
                break;
                case 3:
                if (running_flag[0]) leds[LED_COUNT / 2] = CHSV(LOW_COLOR, 255, thisBright[0]);
                else leds[LED_COUNT / 2] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
                break;
            }
            leds[(LED_COUNT / 2) - 1] = leds[LED_COUNT / 2];
            if (millis() - running_timer > RUNNING_SPEED) {
                running_timer = millis();
                for (int i = 0; i < LED_COUNT / 2 - 1; i++) {
                    leds[i] = leds[i + 1];
                    leds[LED_COUNT - i - 1] = leds[i];
                }
            }
            break;
            case 8:
            byte HUEindex = HUE_START;
            for (int i = 0; i < LED_COUNT / 2; i++) {
                byte this_bright = map(freq_f[(int)floor((LED_COUNT / 2 - i) / freq_to_stripe)], 0, freq_max_f, 0, 255);
                this_bright = constrain(this_bright, 0, 255);
                leds[i] = CHSV(HUEindex, 255, this_bright);
                leds[LED_COUNT - i - 1] = leds[i];
                HUEindex += HUE_STEP;
                if (HUEindex > 255) HUEindex = 0;
            }
            break;
        }
    }

    void HIGHS() {
        for (int i = 0; i < LED_COUNT; i++) leds[i] = CHSV(HIGH_COLOR, 255, thisBright[2]);
    }
    void MIDS() {
        for (int i = 0; i < LED_COUNT; i++) leds[i] = CHSV(MID_COLOR, 255, thisBright[1]);
    }
    void LOWS() {
        for (int i = 0; i < LED_COUNT; i++) leds[i] = CHSV(LOW_COLOR, 255, thisBright[0]);
    }
    void SILENCE() {
        for (int i = 0; i < LED_COUNT; i++) leds[i] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
    }

    // вспомогательная функция, изменяет величину value на шаг incr в пределах minimum.. maximum
    int smartIncr(int value, int incr_step, int mininmum, int maximum) {
        int val_buf = value + incr_step;
        val_buf = constrain(val_buf, mininmum, maximum);
        return val_buf;
    }

    float smartIncrFloat(float value, float incr_step, float mininmum, float maximum) {
        float val_buf = value + incr_step;
        val_buf = constrain(val_buf, mininmum, maximum);
        return val_buf;
    }

    void autoLowPass() {
        // для режима VU
        delay(10);                                // ждём инициализации АЦП
        int thisMax = 0;                          // максимум
        int thisLevel;
        for (byte i = 0; i < 200; i++) {
            thisLevel = analogRead(SOUND_R);        // делаем 200 измерений
            if (thisLevel > thisMax)                // ищем максимумы
            thisMax = thisLevel;                  // запоминаем
            delay(4);                               // ждём 4мс
        }
        LOW_PASS = thisMax + LOW_PASS_ADD;        // нижний порог как максимум тишины + некая величина

        // для режима спектра
        thisMax = 0;
        for (byte i = 0; i < 100; i++) {          // делаем 100 измерений
            analyzeAudio();                         // разбить в спектр
            for (byte j = 2; j < 32; j++) {         // первые 2 канала - хлам
                thisLevel = fht_log_out[j];
                if (thisLevel > thisMax)              // ищем максимумы
                thisMax = thisLevel;                // запоминаем
            }
            delay(4);                               // ждём 4мс
        }
        SPEKTR_LOW_PASS = thisMax + LOW_PASS_FREQ_ADD;  // нижний порог как максимум тишины
    }

    void analyzeAudio() {
        for (int i = 0 ; i < FHT_N ; i++) {
            int sample = analogRead(SOUND_R_FREQ);
            fht_input[i] = sample; // put real data into bins
        }
        fht_window();  // window the data for better frequency response
        fht_reorder(); // reorder the data before doing the fht
        fht_run();     // process the data in the fht
        fht_mag_log(); // take the output of the fht
    }
    void fullLowPass() {
        FastLED.setBrightness(0); // погасить ленту
        FastLED.clear();          // очистить массив пикселей
        FastLED.show();           // отправить значения на ленту
        delay(500);               // подождать чутка
        autoLowPass();            // измерить шумы
        delay(500);               // подождать
        FastLED.setBrightness(BRIGHTNESS);  // вернуть яркость
    }

    //  ============================================================================

    void one_color_all(int cred, int cgrn, int cblu) {             //-SET ALL LEDS TO ONE COLOR
        for (int i = 0 ; i < LED_COUNT; i++ ) {
            leds[i].setRGB( cred, cgrn, cblu);
        }
    }

    void led_loop() {
        if (adapt_light) {                                                // если включена адаптивная яркость
            if (millis() - last_bright > 500) {         // каждые полсекунды
                last_bright = millis();                             // сброить таймер
                new_bright = map(analogRead(6), 1, 1000, 3, max_bright);     // считать показания с фоторезистора, перевести диапазон
                LEDS.setBrightness(new_bright);                // установить новую яркость
            }
        }

        if (millis() - last_change > change_time) {
            change_time = random(5000, 30000);                                // получаем новое случайное время до следующей смены режима
            ledMode = fav_modes[random(0, num_modes - 1)];        // получаем новый случайный номер следующего режима
            change_mode(ledMode);                                                         // меняем режим через change_mode (там для каждого режима стоят цвета и задержки)
            last_change = millis();
            //Serial.println(ledMode);
        }

        switch (ledMode) {
            case 999: break;                                                     // пазуа
            case  2: rainbow_fade(); break;                        // плавная смена цветов всей ленты
            case  3: rainbow_loop(); break;                        // крутящаяся радуга
            case  4: random_burst(); break;                        // случайная смена цветов
            case  5: color_bounce(); break;                        // бегающий светодиод
            case  6: color_bounceFADE(); break;                // бегающий паровозик светодиодов
            case  7: ems_lightsONE(); break;                     // вращаются красный и синий
            case  8: ems_lightsALL(); break;                     // вращается половина красных и половина синих
            case  9: flicker(); break;                                 // случайный стробоскоп
            case 10: pulse_one_color_all(); break;         // пульсация одним цветом
            case 11: pulse_one_color_all_rev(); break; // пульсация со сменой цветов
            case 12: fade_vertical(); break;                     // плавная смена яркости по вертикали (для кольца)
            case 13: rule30(); break;                                    // безумие красных светодиодов
            case 14: random_march(); break;                        // безумие случайных цветов
            case 15: rwb_march(); break;                             // белый синий красный бегут по кругу (ПАТРИОТИЗМ!)
            case 16: radiation(); break;                             // пульсирует значок радиации
            case 17: color_loop_vardelay(); break;         // красный светодиод бегает по кругу
            case 18: white_temps(); break;                         // бело синий градиент (?)
            case 19: sin_bright_wave(); break;                 // тоже хрень какая то
            case 20: pop_horizontal(); break;                    // красные вспышки спускаются вниз
            case 21: quad_bright_curve(); break;             // полумесяц
            case 22: flame(); break;                                     // эффект пламени
            case 23: rainbow_vertical(); break;                // радуга в вертикаьной плоскости (кольцо)
            case 24: pacman(); break;                                    // пакман
            case 25: random_color_pop(); break;                // безумие случайных вспышек
            case 26: ems_lightsSTROBE(); break;                // полицейская мигалка
            case 27: rgb_propeller(); break;                     // RGB пропеллер
            case 28: kitt(); break;                                        // случайные вспышки красного в вертикаьной плоскости
            case 29: matrix(); break;                                    // зелёненькие бегают по кругу случайно
            case 30: new_rainbow_loop(); break;                // крутая плавная вращающаяся радуга
            case 31: strip_march_ccw(); break;                 // чёт сломалось
            case 32: strip_march_cw(); break;                    // чёт сломалось
            case 33: colorWipe(0x00, 0xff, 0x00, thisdelay);
            colorWipe(0x00, 0x00, 0x00, thisdelay); break;                                                                // плавное заполнение цветом
            case 34: CylonBounce(0xff, 0, 0, 4, 10, thisdelay); break;                                            // бегающие светодиоды
            case 35: Fire(55, 120, thisdelay); break;                                                                             // линейный огонь
            case 36: NewKITT(0xff, 0, 0, 8, 10, thisdelay); break;                                                    // беготня секторов круга (не работает)
            case 37: rainbowCycle(thisdelay); break;                                                                                // очень плавная вращающаяся радуга
            case 38: TwinkleRandom(20, thisdelay, 1); break;                                                                // случайные разноцветные включения (1 - танцуют все, 0 - случайный 1 диод)
            case 39: RunningLights(0xff, 0xff, 0x00, thisdelay); break;                                         // бегущие огни
            case 40: Sparkle(0xff, 0xff, 0xff, thisdelay); break;                                                     // случайные вспышки белого цвета
            case 41: SnowSparkle(0x10, 0x10, 0x10, thisdelay, random(100, 1000)); break;        // случайные вспышки белого цвета на белом фоне
            case 42: theaterChase(0xff, 0, 0, thisdelay); break;                                                        // бегущие каждые 3 (ЧИСЛО СВЕТОДИОДОВ ДОЛЖНО БЫТЬ КРАТНО 3)
            case 43: theaterChaseRainbow(thisdelay); break;                                                                 // бегущие каждые 3 радуга (ЧИСЛО СВЕТОДИОДОВ ДОЛЖНО БЫТЬ КРАТНО 3)
            case 44: Strobe(0xff, 0xff, 0xff, 10, thisdelay, 1000); break;                                    // стробоскоп

            case 45: BouncingBalls(0xff, 0, 0, 3); break;                                                                     // прыгающие мячики
            case 46: BouncingColoredBalls(3, ballColors); break;                                                        // прыгающие мячики цветные

            case 888: demo_modeA(); break;                         // длинное демо
            case 889: demo_modeB(); break;                         // короткое демо
        }
    }

    void change_mode(int newmode) {
        thissat = 255;
        switch (newmode) {
            case 0: one_color_all(0, 0, 0); LEDS.show(); break; //---ALL OFF
            case 1: one_color_all(255, 255, 255); LEDS.show(); break; //---ALL ON
            case 2: thisdelay = 60; break;                                            //---STRIP RAINBOW FADE
            case 3: thisdelay = 25; thisstep = 10; break;             //---RAINBOW LOOP
            case 4: thisdelay = 20; break;                                            //---RANDOM BURST
            case 5: thisdelay = 20; thishue = 0; break;                 //---CYLON v1
            case 6: thisdelay = 80; thishue = 0; break;                 //---CYLON v2
            case 7: thisdelay = 40; thishue = 0; break;                 //---POLICE LIGHTS SINGLE
            case 8: thisdelay = 40; thishue = 0; break;                 //---POLICE LIGHTS SOLID
            case 9: thishue = 160; thissat = 50; break;                 //---STRIP FLICKER
            case 10: thisdelay = 15; thishue = 0; break;                //---PULSE COLOR BRIGHTNESS
            case 11: thisdelay = 30; thishue = 0; break;                //---PULSE COLOR SATURATION
            case 12: thisdelay = 60; thishue = 180; break;            //---VERTICAL SOMETHING
            case 13: thisdelay = 100; break;                                        //---CELL AUTO - RULE 30 (RED)
            case 14: thisdelay = 80; break;                                         //---MARCH RANDOM COLORS
            case 15: thisdelay = 80; break;                                         //---MARCH RWB COLORS
            case 16: thisdelay = 60; thishue = 95; break;             //---RADIATION SYMBOL
            //---PLACEHOLDER FOR COLOR LOOP VAR DELAY VARS
            case 19: thisdelay = 35; thishue = 180; break;            //---SIN WAVE BRIGHTNESS
            case 20: thisdelay = 100; thishue = 0; break;             //---POP LEFT/RIGHT
            case 21: thisdelay = 100; thishue = 180; break;         //---QUADRATIC BRIGHTNESS CURVE
            //---PLACEHOLDER FOR FLAME VARS
            case 23: thisdelay = 50; thisstep = 15; break;            //---VERITCAL RAINBOW
            case 24: thisdelay = 50; break;                                         //---PACMAN
            case 25: thisdelay = 35; break;                                         //---RANDOM COLOR POP
            case 26: thisdelay = 25; thishue = 0; break;                //---EMERGECNY STROBE
            case 27: thisdelay = 100; thishue = 0; break;                //---RGB PROPELLER
            case 28: thisdelay = 100; thishue = 0; break;             //---KITT
            case 29: thisdelay = 100; thishue = 95; break;             //---MATRIX RAIN
            case 30: thisdelay = 15; break;                                            //---NEW RAINBOW LOOP
            case 31: thisdelay = 100; break;                                        //---MARCH STRIP NOW CCW
            case 32: thisdelay = 100; break;                                        //---MARCH STRIP NOW CCW
            case 33: thisdelay = 50; break;                                         // colorWipe
            case 34: thisdelay = 50; break;                                         // CylonBounce
            case 35: thisdelay = 15; break;                                         // Fire
            case 36: thisdelay = 50; break;                                         // NewKITT
            case 37: thisdelay = 20; break;                                         // rainbowCycle
            case 38: thisdelay = 10; break;                                         // rainbowTwinkle
            case 39: thisdelay = 50; break;                                         // RunningLights
            case 40: thisdelay = 0; break;                                            // Sparkle
            case 41: thisdelay = 30; break;                                         // SnowSparkle
            case 42: thisdelay = 50; break;                                         // theaterChase
            case 43: thisdelay = 50; break;                                         // theaterChaseRainbow
            case 44: thisdelay = 100; break;                                        // Strobe

            case 101: one_color_all(255, 0, 0); LEDS.show(); break; //---ALL RED
            case 102: one_color_all(0, 255, 0); LEDS.show(); break; //---ALL GREEN
            case 103: one_color_all(0, 0, 255); LEDS.show(); break; //---ALL BLUE
            case 104: one_color_all(255, 255, 0); LEDS.show(); break; //---ALL COLOR X
            case 105: one_color_all(0, 255, 255); LEDS.show(); break; //---ALL COLOR Y
            case 106: one_color_all(255, 0, 255); LEDS.show(); break; //---ALL COLOR Z
        }
        bouncedirection = 0;
        one_color_all(0, 0, 0);
        ledMode = newmode;
    }
