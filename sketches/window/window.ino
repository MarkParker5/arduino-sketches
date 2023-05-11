#include <EEPROM.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <GyverStepper.h>
#include <GyverTimers.h>

GStepper< STEPPER4WIRE_HALF> stepper(4076, 4, 6, 5, 7);

RF24 radio(9,10);
String data;
String me = "window";
//byte address[][6] = {"1Node","2Node","3Node","4Node","5Node","6Node"};  //возможные номера труб

void setup(){
    Serial.begin(9600);
    radio.begin(); //активировать модуль
    radio.setAutoAck(1);         //режим подтверждения приёма, 1 вкл 0 выкл
    radio.setRetries(0,15);     //(время между попыткой достучаться, число попыток)
//    radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал
    radio.setPayloadSize(2);     //размер пакета, в байтах
    //radio.openReadingPipe(1, address[0]);      //хотим слушать трубу 0
    radio.openReadingPipe(1, 0xF0F0F0F0E1LL);
    radio.setChannel(0x60);  //выбираем канал (в котором нет шумов!)
    radio.setPALevel (RF24_PA_HIGH); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
    radio.setDataRate (RF24_2MBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
    //должна быть одинакова на приёмнике и передатчике!
    //при самой низкой скорости имеем самую высокую чувствительность и дальность!!
    radio.powerUp(); //начать работу
    radio.startListening();  //начинаем слушать эфир, мы приёмный модуль



    Timer1.setPeriod(stepper.getMinPeriod()/4);
    Timer1.enableISR();

    //pinMode(2, INPUT);
    //attachInterrupt(0, setopen, RISING);
}

ISR(TIMER1_A) {
    stepper.tick(); // тикаем тут
}

void setopen(){
    stepper.setCurrent(0);
}

void loop() {
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

void start(){
    StaticJsonDocument<256> doc;
    auto error = deserializeJson(doc, data);
    if(error) return;
    JsonObject root = doc.as<JsonObject>();
    String target = root["target"];
    if(target!=me) return;
    //Serial.println(target);
    String cmd = root["cmd"];
    //Serial.println(cmd);
    if(cmd == "open") open();
    else if(cmd == "close") close();
    else if(cmd == "toggle") toggle();
}

void open() {
    stepper.setMaxSpeed(300);
    stepper.setTarget(0);
    EEPROM.write(0, 0);
}

void close() {
    stepper.setMaxSpeed(1500);
    stepper.setTarget(-56500);
    EEPROM.write(0, 1);
}

void toggle() {
    if(EEPROM.read(0)) {
        open();
    } else {
        close();
    }
}
