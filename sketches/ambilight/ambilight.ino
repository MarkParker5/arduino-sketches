#include "FastLED.h"

#define LED_PIN 3
const int LED_COUNT = 232;
CRGB leds[LED_COUNT];
int current_mode = 3;      // 0 = off, 1 = led, 2 = colormusic, 3 = ambilight
bool needs_update = false;

void setmode(int mode) {
    current_mode = mode;
    for (int i = 0 ; i < LED_COUNT; i++) {
        setPixel(i, 0, 0, 0);
    }
    FastLED.show();
    //Serial.println(mode);
}

void setup() {
    Serial.begin(115200);
    Serial.setTimeout(0);
    FastLED.addLeds<WS2811, LED_PIN, RGB>(leds, LED_COUNT).setCorrection(TypicalLEDStrip);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 3000);
    FastLED.setBrightness(20);
    for (int i = 0; i < LED_COUNT; i++) {
        setPixel(i, 0, 0, 0);
    }
    FastLED.show();
}

void loop() {
    //  protocol: COMMAND<r1g1b1r2g2b2r3g3b3>
    //  where COMMAND: String, r1 - b3: bytes, 1,2,3 - index of pixel
    //  ambilight<123123123123123123123123123123123>
    if(Serial.available()) {
        String cmd = read_command();
        if(cmd == "off") setmode(0);
        else if(cmd == "led") setmode(1);
        else if(cmd == "colormusic") setmode(2);
        else if(cmd == "ambilight") setmode(3);
        needs_update = true;
    }

    switch(current_mode) {
        case 0: return;
        case 1: led_loop(); break;
        case 2: colormusic_loop(); break;
        case 3: ambilight_loop(); break;
    }
}

String read_command() {
    String cmd = "";
    while(true) {
        while(!Serial.available());
        char temp = Serial.read();
        if(temp == ';') return cmd;
        cmd += temp;
    }
}

void led_loop() {

}

void colormusic_loop() {

}

void ambilight_loop() {
    if(!needs_update) return;
    for(int i = 0; i < LED_COUNT; i++) {
        while(!Serial.available());
        leds[i].b = Serial.read();
        while(!Serial.available());
        leds[i].r = Serial.read();
        while(!Serial.available());
        leds[i].g = Serial.read();

        /*while(!Serial.available());
        byte r = Serial.read();
        while(!Serial.available());
        byte g = Serial.read();
        while(!Serial.available());
        byte b = Serial.read();
        setPixel(i, r, g, b);
        Serial.print("\t");
        Serial.print(i);
        Serial.print(" ");
        Serial.print(r, DEC);
        Serial.print(" ");
        Serial.print(g, DEC);
        Serial.print(" ");
        Serial.print(b, DEC);
        Serial.print(" ");
        Serial.print("\n");
        */
    }
    FastLED.show();
    needs_update = false;
    Serial.flush();
}

void setPixel(int i, byte red, byte green, byte blue) {
    leds[i].r = red;
    leds[i].g = green;
    leds[i].b = blue;
}
