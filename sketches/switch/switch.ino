#include <nRF24L01.h>
#include <RF24.h>
#include "GyverButton.h"

GButton left_btn(6, LOW_PULL, NORM_OPEN);
GButton middle_btn(5, LOW_PULL, NORM_OPEN);
GButton right_btn(4, LOW_PULL, NORM_OPEN);
GButton all_btn(0);

int ignore_actions = 0;

RF24 radio(9,10);

void setup() {
    radio.begin();
    radio.setAutoAck(1);
    radio.setRetries(0,15);
    radio.enableAckPayload();
    radio.setPayloadSize(32);
    radio.openReadingPipe(1, 0xF0F0F0F0E1LL);
    radio.setChannel(0x60);
    radio.setPALevel (RF24_PA_HIGH);
    radio.setDataRate (RF24_250KBPS);
    radio.powerUp();

    int debounce = 50;
    int timeout = 250;
    int click_timeout = 300;
    left_btn.setDebounce(debounce);
    left_btn.setTimeout(timeout);
    middle_btn.setDebounce(debounce);
    middle_btn.setTimeout(timeout);
    right_btn.setDebounce(debounce);
    right_btn.setTimeout(timeout);
    all_btn.setDebounce(debounce/2);
    all_btn.setTimeout(timeout / 2.5 );

    //Serial.begin(9600);
}

void loop() {
    left_btn.tick();
    middle_btn.tick();
    right_btn.tick();
    all_btn.tick(left_btn.state() && middle_btn.state() && right_btn.state());

    if(all_btn.isHolded()){
        send("{\"target\": \"hub\", \"cmd\": \"turn_off_all\"}");
        ignore_actions = 3;
    }

    else if(left_btn.isSingle()) send("{\"target\": \"light\", \"cmd\": \"toggle\"}");
    else if(left_btn.isDouble()) send("{\"target\": \"light\", \"cmd\": \"general\"}");
    else if(left_btn.isTriple()) send("{\"target\": \"light\", \"cmd\": \"cold\"}");
    else if(left_btn.isHolded()) send("{\"target\": \"light\", \"cmd\": \"night_light\"}");

    else if(middle_btn.isSingle()) send("{\"target\": \"lamp\", \"cmd\": \"toggle\"}");
    else if(middle_btn.isDouble()) send("{\"target\": \"led\", \"cmd\": \"toggle\"}");
    else if(middle_btn.isTriple()) send("{\"target\": \"tv\", \"cmd\": \"led_toggle\"}");
    else if(middle_btn.isHolded()){
        send("{\"target\": \"led\", \"cmd\": \"colormusic\"}");
        send("{\"target\": \"tv\",  \"cmd\": \"colormusic\"}");
    }

    else if(right_btn.isSingle()) send("{\"target\": \"hub\", \"cmd\": \"listen\"}");
    else if(right_btn.isDouble()) send("{\"target\": \"window\", \"cmd\": \"toggle\"}");
    else if(right_btn.isTriple()) send("{\"target\": \"tv\", \"cmd\": \"toggle\"}");
    //else if(right_btn.isHolded()) send("{\"target\": \"\", \"cmd\": \"\"}");
}

void send(String str) {
    if(ignore_actions) {
        ignore_actions--;
        return;
    }
    //Serial.println(str);
    for(int i = 0; i < str.length(); i++) {
        char nextChar = str.charAt(i);
        radio.write(&nextChar, sizeof(nextChar));
        delay(15);
    }
}
