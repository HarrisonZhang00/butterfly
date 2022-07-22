
#include <Arduino.h>
#include <SPI.h>
#include <Servo.h>
#include <nRF24L01.h>
#include <RF24.h>

#define X A0
#define Y A1
#define Z 10
Servo LEFT_SERVO;
Servo RIGHT_SERVO;

// 2.4 ghz radio
RF24 Radio(5,6); // CE, CSN
const byte address[6] = "37412";

// timekeeping
unsigned long time_curr = 0;
unsigned long time_prev = 0;
const int FLAP_DELAY = 200;
const int SWITCH_DELAY = 500;

// analog readings
int x_val = 0;
int y_val = 0;
int z_val = 0;
unsigned long z_vals[2][2] = {{0,0}, 
                              {0,0}};
bool is_on = false;

// analog thresholds
const int LEFT_THRES_ANLG = 400;
const int DOWN_THRES_ANLG = 400;
const int RIGHT_THRES_ANLG = 1022 - LEFT_THRES_ANLG;
const int UP_THRES_ANLG = 1022 - DOWN_THRES_ANLG;

////////////////////////////////////////////////////////////////////

struct Control {

    int x_tc = 0;
    int y_tc = 0;
    int z_tc = 0;

};

Control ctrl_data;

void setup() {

    Serial.begin(9600);
    
    LEFT_SERVO.attach(3);
    RIGHT_SERVO.attach(4);
    LEFT_SERVO.write(get_angle(0));
    RIGHT_SERVO.write(get_angle(0));

    Radio.begin();
    Radio.openReadingPipe(0,address);
    Radio.setPALevel(RF24_PA_MAX); // max transceiving distance
    Radio.startListening(); // sets as receiver

}

void loop() {

      if (Radio.available()) {
              char msg_in[32] = "";
              Radio.read(&msg_in, sizeof(msg_in));
              Radio.read(&ctrl_data, sizeof(Control));
  
              x_val = (int)ctrl_data.x_tc;
              y_val = (int)ctrl_data.y_tc;
              z_val = (int)ctrl_data.z_tc;
  
              if (z_val == 1) {
                for (byte c = 0; c <= 1; ++c) {
                    if (z_vals[0][c] == 0) {
                        z_vals[0][c] = z_val;
                    }
                    if (z_vals[1][c] == 0) {
                        z_vals[1][c] = time_curr;    
                    }
                    if (abs(z_vals[1][0] - z_vals[1][1]) <= 10) { // 10 ms minimum delay
                        z_vals[0][1] = 0;
                        z_vals[1][1] = 0;
                    }
                }
            }
        if (is_dbl_pressed(10, SWITCH_DELAY)) {
            is_on = !is_on;
            for (byte r = 0; r <= 1; ++r) {
                for (byte c = 0; c <= 1; ++c) {
                    z_vals[r][c] = 0;
                }
            }
        }
        else if (abs(z_vals[1][0] - z_vals[1][1]) > SWITCH_DELAY) {
            for (byte r = 0; r <= 1; ++r) {
                for (byte c = 0; c <= 1; ++c) {
                    z_vals[r][c] = 0;
                }
            }
        }
   }  
   
   while(is_on) {

      while (x_val < LEFT_THRES_ANLG && y_val > UP_THRES_ANLG) {
           LEFT_SERVO.write(get_angle(-60));
           delay(110);
      }
      while (x_val < RIGHT_THRES_ANLG && y_val > UP_THRES_ANLG) {
           RIGHT_SERVO.write(get_angle(-60));
           delay(110);
      }
      while (y_val < DOWN_THRES_ANLG) {
           LEFT_SERVO.write(get_angle(30));
           delay(1000);
           RIGHT_SERVO.write(get_angle(30));
           delay(1000);
      }
      
   } /*while (is_on) {} */

}



int get_uint8 (float degrees) {
// maps desired servo rotation to float voltage control (0-255)
    if (degrees > 90.0) {degrees = 90.0;}
    else if (degrees < -90.0) {degrees = -90.0;}

    int uint8_val = degrees/180.0 * 256.0 + 127.0;

    if (uint8_val > 255) {uint8_val = 255;}
    else if (uint8_val < 0) {uint8_val = 0;}

    return uint8_val;
}

int get_thres_deg (int thres_anlg) {
    if (thres_anlg < 0) {thres_anlg = 0;}
    else if (thres_anlg > 1023) {thres_anlg = 1023;}

    return (int)((511 - thres_anlg) / 511 * 180);
}

int get_angle (int displacement) {
    if (displacement < -90) {displacement = -90;}
    else if (displacement > 90) {displacement = 90;}
    
    return 90 + displacement;
}

bool is_dbl_pressed (int min_thres, int max_thres) {
    
    for (byte c = 0; c < sizeof(z_vals[0]); ++c) {
        if (z_vals[0][c] == 0) {
            return false;
        }
    }
    if (abs(z_vals[1][0] - z_vals[1][1]) > max_thres || 
        abs(z_vals[1][0] - z_vals[1][1]) < min_thres) {
        return false;
    }

    return true;
}
