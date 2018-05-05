#include <Arduino.h>
#include <WS2812.h>
#include <MPU6050_tockn.h>
#include <Wire.h>

#define LED_COUNT 27
#define MPU_UPDATE_TIMER 10
#define MPU_UPDATE_COUNT 4
#define BLINK_UPDATE_TIMER 500

WS2812 LED(LED_COUNT);
MPU6050 mpu6050(Wire);
	
cRGB white;
cRGB black;

int mode;
int updateCount = 0;
int updateDelay = 0;
int timeUpdateDelay = 0;

long oldTime = millis();
long oldLightTime = millis();
long curTime = 0;
float accel  = 0;

int i;
bool lighted = true;

void setup() {
    Serial.begin(9600);
	LED.setOutput(9); // Digital Pin 9
    Wire.begin();
    mpu6050.begin();    

    white.r = 255; white.g = 255; white.b = 255;
    black.r = 0; black.g = 0; black.b = 0;

    for(i=0; i<27; i++) {
	    LED.set_crgb_at(i, white); // Set value at LED found at index 0
    }
    LED.sync();    
}

void loop() {
    curTime = millis();

    // Approximation of MPU data with interval of 25ms
    if(curTime > oldTime + MPU_UPDATE_TIMER + updateDelay) {
        updateDelay = 0;
        oldTime = millis();
        mpu6050.update();
        updateCount++;

        accel += (
            sqrt(
                pow(mpu6050.getAccX(), 2) + 
                pow(mpu6050.getAccY(), 2) +
                pow(mpu6050.getAccZ(), 2)
            ));
    }

    if(updateCount >= MPU_UPDATE_COUNT) {
        updateDelay = 500;
        accel = accel / MPU_UPDATE_COUNT;
        updateCount = 0;

        mode = 0;

        if(accel >= 1.46f && accel < 1.52f) {
            mode = 1;
        }

        if(accel >= 1.52f && accel < 1.58f) {
            mode = 2;
        }

        if(accel >= 1.58f && accel < 1.64f) {
            mode = 4;
        }

        if(accel >= 1.64f) {
            mode = 8;
        }

        Serial.println(accel, 4);
        Serial.println(mode);
    }
    
    if(mode > 0) {        
        if(curTime > oldLightTime + BLINK_UPDATE_TIMER / mode + timeUpdateDelay) {
            if(lighted == true) {
                lighted = false;
                timeUpdateDelay = 100;
                for(i=0; i<27; i++) {
                    LED.set_crgb_at(i, black);
                }           
                LED.sync();    
     
            } else {
                lighted = true;
                oldLightTime = millis();
                timeUpdateDelay = 0;
                for(i=0; i<27; i++) {
                    LED.set_crgb_at(i, white);
                }                
                LED.sync();    
            }
        }
    } else {
        lighted = true;
        oldLightTime = millis();
        timeUpdateDelay = 0;
        for(i=0; i<27; i++) {
            LED.set_crgb_at(i, white);
        }      
        LED.sync();            
    }
}
