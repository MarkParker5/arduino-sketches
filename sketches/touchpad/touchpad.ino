#include <EEPROM.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <ArduinoJson.h>

RF24 radio(9,10);
String data;
String me = "touchpad";
byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"};  //возможные номера труб
bool lamp_state = 0;

//            TouchPad
#define lampRelayPin 14 //A0

#define b11 2
#define b12 3
#define b13 4

#define b21 5
#define b23 6

#define b31 7
#define b32 8
#define b33 15

int lastTop = 0;
int lastRight = 0;
int lastBottom = 0;
int lastLeft = 0;
int lastAll = 0;
int allTapsCount = 0;

void singleTap(){Serial.println("•\tTap!");};
void doubleTap(){Serial.println("••\tDouble Tap!");};
void tripleTap(){Serial.println("•••\tTriple Tap!");};

void swipeRight(){Serial.println("→\tSwipe Right!");};
void swipeLeft(){Serial.println("←\tSwipe Left!");};
void swipeUp(){Serial.println("↑\tSwipe Up!");};
void swipeDown(){Serial.println("↓\tSwipe Down!");};

void checkButtons() {
    bool top, right, bottom, left, all;
    top    = digitalRead(b11) || digitalRead(b12) || digitalRead(b13);
    right  = digitalRead(b31) || digitalRead(b32) || digitalRead(b33);
    bottom = digitalRead(b31) || digitalRead(b32) || digitalRead(b33);
    left   = digitalRead(b11) || digitalRead(b21) || digitalRead(b31);
    all    = top && right && bottom && left;

    if(top) lastTop = millis();
    if(right) lastRight = millis();
    if(bottom) lastBottom = millis();
    if(left) lastLeft = millis();
    if(all) lastAll = millis();
}

void recognizeGestures() {
    if(allTapsCount == 3) tripleTap();
    int current = millis();
    int dx = lastLeft - lastRight;
    int dy = lastBottom - lastTop;
    int dz = current - lastAll;
    if(dx > 50 && dx < 200) swipeRight();
    else if(-dx > 50 && -dx < 20) swipeLeft();
    else if(dy > 50 && dy < 200) swipeUp();
    else if(-dy > 50 && -dy < 200) swipeDown();
    else if(dz > 50 && dz < 200){
        allTapsCount++;
        return;
    }
    else if(allTapsCount == 2) doubleTap();
    else if(allTapsCount == 1) singleTap();
    allTapsCount = 0;
}
//            /TouchPad

void turn_on() {
    lamp_state = 1;
    digitalWrite(lampRelayPin, 1);
}

void turn_off() {
    lamp_state = 0;
    digitalWrite(lampRelayPin, lamp_state);
}

void toggle() {
    lamp_state = !lamp_state;
    digitalWrite(lampRelayPin, lamp_state);
}

//            Main
void setup(){
    Serial.begin(9600);
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
    radio.startListening();

    pinMode(lampRelayPin, OUTPUT);
    pinMode(b11, INPUT);
    pinMode(b12, INPUT);
    pinMode(b13, INPUT);
    pinMode(b21, INPUT);
    pinMode(b23, INPUT);
    pinMode(b31, INPUT);
    pinMode(b32, INPUT);
    pinMode(b33, INPUT);
    digitalWrite(lampRelayPin, lamp_state);
}

void loop() {
    //checkButtons();
    //recognizeGestures();
    byte pipeNo, gotByte;
    while(radio.available(&pipeNo)) {
        radio.read(&gotByte, sizeof(gotByte));
        char symbol = (char)gotByte;
        if(symbol == '{') {
            data = "";
            data += symbol;
        } else if(symbol == '}') {
            data += symbol;
            start();
            data = "";
        } else if(symbol != '\n' && symbol != ';') {
            data += symbol;
        }
    }
}

void start() {
    Serial.print('\n');
    Serial.println(data);
    StaticJsonDocument<256> doc;
    if(deserializeJson(doc, data)) return;
    JsonObject root = doc.as<JsonObject>();
    String target = root["target"];
    String cmd = root["cmd"];

    if(target=="led"){
        //  redirect command to second arduino
        Serial.println(cmd);
    }

    if(target=="lamp"){
        if(cmd == "toggle") toggle();
        if(cmd == "turn_on") turn_on();
        else if(cmd == "turn_off") turn_off();
    }

    //if(target!=me) return;
    //if(cmd == "lorem") lorem();
}
