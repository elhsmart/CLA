#include <Arduino.h>
#include <WS2812.h>
#include <MPU6050_tockn.h>
#include <Wire.h>

#define LED_COUNT 27
#define MPU_UPDATE_COUNT 4
#define MPU_UPDATE_TIMER 10
#define BLINK_UPDATE_TIMER 700
#define RESET_TIMEOUT 5000

#define WS2812B_PIN 9
#define CALIBRATION_PIN 8
#define OUTPUT_PIN 7

WS2812 LED(LED_COUNT);
MPU6050 mpu6050(Wire);
	
cRGB white;
cRGB black;

int mode = 0;
int debug = false;
int updateCount = 0;
int updateDelay = 0;
int timeUpdateDelay = 0;

long oldTime = millis();
long oldLightTime = millis();
long resetTime = millis();
long curTime = 0;
float accel  = 0;

int i;
bool lighted = true;

void setColor(cRGB color) {
    for(i=0; i<27; i++) {
	    LED.set_crgb_at(i, color); // Set value at LED found at index 0
    }
    LED.sync();
}

void setup() {
    pinMode(CALIBRATION_PIN, INPUT);
    pinMode(OUTPUT_PIN, OUTPUT);
    if(digitalRead(CALIBRATION_PIN) == HIGH) {
        debug = true;
        Serial.begin(9600);
    }

	LED.setOutput(WS2812B_PIN); // Digital Pin 9
    Wire.begin();
    mpu6050.begin();    

    white.r = 255; white.g = 255; white.b = 255;
    black.r = 0; black.g = 0; black.b = 0;
    
    setColor(white);
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
        updateDelay = 200;
        accel = accel / MPU_UPDATE_COUNT;
        updateCount = 0;

        if(accel >= 1.46f && accel < 1.52f) {
            resetTime = curTime;
            if(mode < 1) {
                mode = 1;
            }
        }

        if(accel >= 1.52f && accel < 1.58f) {
            resetTime = curTime;
            if(mode < 2) {
                mode = 2;
            }
        }

        if(accel >= 1.58f && accel < 1.64f) {
            resetTime = curTime;
            if(mode < 4) {
                mode = 4;
            }
        }

        if(accel >= 1.64f) {
            resetTime = curTime;            
            if(mode < 8) {
                mode = 8;
            }
        }

        if(debug == true) {
            Serial.println(accel, 4);
            Serial.println(mode);
        }
    }
    
    if(mode > 0) {        
        if(curTime > oldLightTime + BLINK_UPDATE_TIMER / mode + timeUpdateDelay) {
            if(lighted == true) {
                lighted = false;
                timeUpdateDelay = 100;
                setColor(black);
                digitalWrite(OUTPUT_PIN, LOW);

            } else {
                lighted = true;
                oldLightTime = millis();
                timeUpdateDelay = 0;
                setColor(white);  
                digitalWrite(OUTPUT_PIN, HIGH);
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
        digitalWrite(OUTPUT_PIN, HIGH);               
    }

    if(curTime > resetTime + RESET_TIMEOUT) {
        mode = 0;
        resetTime = curTime;
    }
}
