#include <EEPROM.h>
#include <SPI.h>
//#include <ArduinoJson.h>
#include <nRF24L01.h>
#include <RF24.h>
//#include <IRLib.h>

#define IR_PIN 3
//IRsend irsender;

RF24 radio(9,10);
String data;
String me = "light";
String lastmode = "";
String mode = "";
byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"};

void setup() {
    Serial.begin(9600);
    radio.begin();
    radio.setAutoAck(1);
    radio.setRetries(0,15);
    radio.enableAckPayload();
    radio.setPayloadSize(2);
    radio.openReadingPipe(1, 0x0000000003);
    radio.setChannel(0x60);
    radio.setPALevel (RF24_PA_HIGH);
    radio.setDataRate (RF24_2MBPS);
    radio.powerUp();
    radio.startListening();
}

void loop() {
    byte pipeNo, gotByte;
    while(radio.available(&pipeNo)) {
        radio.read(&gotByte, 2);
        Serial.println(gotByte);
//        char symbol = (char)gotByte;
//        if(symbol == '{') {
//            data = "";
//            data += symbol;
//        } else if(symbol == '}') {
//            data += symbol;
//            start();
//            data = "";
//        } else if(symbol != '\n' && symbol != ';') {
//            data += symbol;
//        }
    }
//
//    if(mode == "sunrise" && millis() % 1000){
//        irsender.send(NEC, 0xFF906F, 0);
//    }
}

void toggle() {
    // on/off
//    irsender.send(NEC,0xFF629D, 0);
}

void general() {
    mode = "general"; // 4000K, max brightness sets automaticaly
//    irsender.send(NEC, 0xFF18E7, 0);
}

void warm() {
    mode = "warm";
//    irsender.send(NEC, 0xFF30CF, 0);
}

void cold() {
    mode = "cold";
//    irsender.send(NEC, 0xFF7A85, 0);
}

void night_light() {
    mode = "night_light";
    for(int i = 0; i < 200; i++) {
//        irsender.send(NEC, 0xFFE01F, 0); // K-
        delay(i%10 ? 10 : 100);
    }
    for(int i = 0; i < 200; i++) {
//        irsender.send(NEC, 0xFF9867, 0); // -
        delay(i%10 ? 10 : 100);
    }
}

void sunrise() {
    lastmode = mode;
    mode = "sunrise";
//    irsender.send(NEC, lastmode == "night_light" ? 0xFF629D : 0xFFA857, 0);
}

void start() {
    Serial.println("START");
    return;
//    StaticJsonDocument<256> doc;
//    if(deserializeJson(doc, data)) return;
//    JsonObject root = doc.as<JsonObject>();
//    String target = root["target"];
//    if(target!=me) return;

//    String cmd = root["cmd"];
//    if(cmd == "toggle") toggle();
//    else if(cmd == "general") general();
//    else if(cmd == "night_light") night_light();
//    else if(cmd == "sunrise") sunrise();
}

/*
Light:
repeat      -   FFFFFF
onn/off     -   FF629D
night       -   FFA857
K+          -   FF906F
K-          -   FFE01F
+           -   FF02FD
-           -   FF9867
20%         -   FF38C7
50%         -   FFCA35
100%        -   FF4AB5
3000K       -   FF30CF
4000K       -   FF18E7
6000K       -   FF7A85
circle      -   FFA25D
30min       -   FFE21D

TV:
on/off      -   FDC03F
*/
