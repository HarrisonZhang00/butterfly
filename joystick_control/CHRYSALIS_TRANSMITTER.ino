
/* CHRYSALIS TRANSMITTER PROGRAM */

#include <Arduino.h>
#include <SPI.h>
#include <Servo.h>
#include <nRF24L01.h>
#include <RF24.h>

#define X A0
#define Y A1
#define Z 10

RF24 Radio(5,6); // CE, CSN
const byte address[6] = "37412";

////////////////////////////////////////////////////////////////////

struct Control {

    int x_in = 0;
    int y_in = 0;
    int z_in = 0;

};

Control ctrl_data;

void setup() {

    Serial.begin(9600);

    Radio.begin();
    Radio.openWritingPipe(address);
    Radio.setPALevel(RF24_PA_MAX); // max transceiving distance
    Radio.stopListening(); // sets as transmitter

}

void loop() {

    x_val = (int)analogRead(X);
    y_val = (int)analogRead(Y);
    z_val = (int)digitalRead(10);

    const char msg_out[] = "";
    Radio.write(&msg_out, sizeof(msg_out));
    Radio.write(&ctrl_data, sizeof(Control));

}