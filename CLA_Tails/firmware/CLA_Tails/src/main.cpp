/*

0
b
0
0 - stopLight
0 - eStopLight
0 - parkLight
0 - turnLight
0 - eTurnLight
0 - fogLight
0 - rearLight

*/

#include <Arduino.h>
#include <SPI.h>

#define LA_1 6
#define LA_2 5

#define OE_1 7
#define OE_2 8

#define DATA 11
#define CLK 13

#define BIT_STOP_LIGHT_NUM 7
#define BIT_ESTOP_LIGHT_NUM 6
#define BIT_PARK_LIGHT_NUM 5
#define BIT_TURN_LIGHT_NUM 4
#define BIT_ETURN_LIGHT_NUM 3
#define BIT_FOG_LIGHT_NUM 2
#define BIT_REAR_LIGHT_NUM 1

unsigned int pause = 5000;
unsigned long ct = millis();

unsigned int eStopLimit = 16;
unsigned int eStopCount = 0;
unsigned long eStopTime  = millis();
unsigned int eStopPause = 10;

unsigned int eTurnLimit = 4;
unsigned int eTurnCount = 0;
unsigned long eTurnTime  = millis();
unsigned int eTurnPause = 75;

unsigned long turnTime = millis();
unsigned int turnPause = 300;
bool turnEnabled = false;

unsigned int state = 0b00000000;

byte f_1;
byte f_2;
byte f_3;
byte f_4;

int pwm_1;
int pwm_2;

void setLights() {    

    analogWrite(LA_1, pwm_1);
    analogWrite(LA_2, pwm_2);   

    digitalWrite(OE_1, LOW);
    digitalWrite(OE_2, LOW);

    shiftOut(DATA, CLK, MSBFIRST, f_1);
    shiftOut(DATA, CLK, MSBFIRST, f_2);
    shiftOut(DATA, CLK, MSBFIRST, f_3);
    shiftOut(DATA, CLK, MSBFIRST, f_4); 

    digitalWrite(OE_1, HIGH);
    digitalWrite(OE_2, HIGH);
}

void resetLights() {
    pwm_1 = 0; //255;
    pwm_2 = 0; //255;

    f_1 = 0x0;
    f_2 = 0x0;
    f_3 = 0x0;
    f_4 = 0x0;
}

void setup() {
    Serial.begin(9600);
    pinMode(LA_1, OUTPUT);
    pinMode(LA_2, OUTPUT);

    analogWrite(LA_1, 200);
    analogWrite(LA_2, 200);

    pinMode(OE_1, OUTPUT);
    pinMode(OE_2, OUTPUT);

    pinMode(DATA, OUTPUT);
    pinMode(CLK, OUTPUT);
}

/* State checks */
bool isStopLightEnabled() {
    return (state >> (BIT_STOP_LIGHT_NUM - 1) & 1) == 1 ? true : false;
}

bool isEmergencyStopLightEnabled() {
    return (state >> (BIT_ESTOP_LIGHT_NUM - 1) & 1) == 1 ? true : false;
}

bool isParkLightEnabled() {
    return (state >> (BIT_PARK_LIGHT_NUM - 1) & 1) == 1 ? true : false;
}

bool isTurnLightEnabled() {
    return (state >> (BIT_TURN_LIGHT_NUM - 1) & 1) == 1 ? true : false;
}

bool isEmergencyTurnLightEnabled() {
    return (state >> (BIT_ETURN_LIGHT_NUM - 1) & 1) == 1 ? true : false;
}

bool isFogLightEnabled() {
    return (state >> (BIT_FOG_LIGHT_NUM - 1) & 1) == 1 ? true : false;
}

bool isRearLightEnabled() {
    return (state >> (BIT_REAR_LIGHT_NUM - 1) & 1) == 1 ? true : false;
}
/* End of State checks */


void setFogLights() {
    if(isFogLightEnabled()) {
        pwm_2 = 0; //255;

        f_3 = f_3 | 0b00001001;
        f_4 = f_4 | 0b00100100;        
    }
}

void setRearLights() {
    if(isRearLightEnabled()) {
        pwm_2 = 0; //255;

        f_3 = f_3 | 0b11111111;
        f_4 = f_4 | 0b11111111;        
    }
}

void setParkingLights() {
    if(isParkLightEnabled()) {
        pwm_1 = 242;
        pwm_2 = 242;

        f_1 = f_1 | 0b00001010;
        f_2 = f_2 | 0b10101111;
        f_3 = f_3 | 0b00001001;
        f_4 = f_4 | 0b00100100;
    }
}

void setStopLights() {
    if(isStopLightEnabled()) {
        pwm_1 = 0;
        pwm_2 = 0;

        f_1 = f_1 | 0b00001010;
        f_2 = f_2 | 0b10101111;
        f_3 = f_3 | 0b00001001;
        f_4 = f_4 | 0b00100100;
    }
}

void setEmergencyStopLights() {
    if(isEmergencyStopLightEnabled()) {
        eStopTime = millis();
    }
}

void updateEmergencyStopLights() {
    if(isEmergencyStopLightEnabled()) {
        if(millis() > eStopTime + eStopPause) {
            eStopTime = millis();

            eStopCount++;
            if(eStopCount > eStopLimit) {
                eStopCount = 0;
            }

            pwm_1 = (int)(255 - (int)(255 * ((float)eStopCount / (float)eStopLimit)));
            pwm_2 = (int)(255 - (int)(255 * ((float)eStopCount / (float)eStopLimit)));

            analogWrite(LA_1, pwm_1);
            analogWrite(LA_2, pwm_2);             
        }
    }
}

void setTurnLights() {
    if(isTurnLightEnabled()) {
        turnTime = millis();

        f_1 = f_1 ^ 0b00000101;
        f_2 = f_2 ^ 0b01010000;
    }
}

void setEmergencyTurnLights() {
    if(isEmergencyTurnLightEnabled()) {
        eTurnTime = millis();
    }
}

void updateEmergencyTurnLights() {
    if(isEmergencyTurnLightEnabled()) {
        if(millis() > eTurnTime + eTurnPause) {
            resetLights();

            setParkingLights();
            setEmergencyStopLights();
            setStopLights();
            setTurnLights();
            setFogLights();
            setRearLights();        
            
            eTurnTime = millis();
            eTurnCount++;

            if(eTurnCount == eTurnLimit + 1) {
                eTurnCount = 0;
            }

            Serial.println(eTurnCount);

            switch(eTurnCount) {
                case 0: {
                    f_1 = f_1 ^ 0b00000000;
                    f_2 = f_2 ^ 0b00000000;
                    setLights();
                    return;
                }
                case 1: {
                    f_1 = f_1 ^ 0b00000000;
                    f_2 = f_2 ^ 0b00010000;
                    setLights();
                    return;
                }

                case 2: {
                    f_1 = f_1 | 0b00000000;
                    f_2 = f_2 | 0b01010000;
                    setLights();
                    return;
                }

                case 3: {
                    f_1 = f_1 ^ 0b00000001;
                    f_2 = f_2 ^ 0b01010000;
                    setLights();
                    return;
                }

                case 4: {
                    f_1 = f_1 ^ 0b00000101;
                    f_2 = f_2 ^ 0b01010000;
                    setLights();
                    return;
                }
            }
        }
    }
}

void updateTurnLlights() {
    if(isTurnLightEnabled()) {
        if(millis() > turnTime + turnPause) {
            turnTime = millis();

            if(turnEnabled == true) {
                turnEnabled = false;
            } else {
                turnEnabled = true;
            }
            
            if(turnEnabled == true) {
                f_1 = f_1 ^ 0b00000101;
                f_2 = f_2 ^ 0b01010000;
            }

            setLights();            
        }
    }
}

void switchState() {
    switch(state) {
        case 0b00000000: {
            // Enable park light
            state = 0b00010000;
            return;
        }

        case 0b00010000: {
            // Enable stop light
            state = 0b01010000;
            return;
        }
        
        case 0b01010000: {
            // Enable emergency light
            state = 0b01110000;
            return;
        }

         case 0b01110000: {
            //Disable stop emergency light, enable turn light
            state = 0b01011000;
            return;
        }

        case 0b01011000: {
            //Enable emergency turn light
            state = 0b01010100;
            return;
        }

        case 0b01010100: {
            //enable fog light
            state = 0b00010010;
            return;
        }

        case 0b00010010: {
            //enable rear light
            state = 0b00010001;
            return;
        }

        /*case 0b00000001: {
            //rurn off everything
            state = 0b00000000;
        }*/

        default: {
            state = 0b00000000;
            return;
        }
    }
}

void printBits(byte myByte){
 for(byte mask = 0x80; mask; mask >>= 1){
   if(mask  & myByte)
       Serial.print('1');
   else
       Serial.print('0');
 }
}

void dumpState() {
    printBits(state); Serial.println("");
    Serial.print("STOP:\t"); Serial.println(isStopLightEnabled() ? "1" : "0");
    Serial.print("E_STOP:\t"); Serial.println(isEmergencyStopLightEnabled() ? "1" : "0");
    Serial.print("PARK:\t"); Serial.println(isParkLightEnabled() ? "1" : "0");
    Serial.print("TURN:\t"); Serial.println(isTurnLightEnabled() ? "1" : "0");
    Serial.print("E_TURN:\t"); Serial.println(isEmergencyTurnLightEnabled() ? "1" : "0");
    Serial.print("FOG:\t"); Serial.println(isFogLightEnabled() ? "1" : "0");
    Serial.print("REAR:\t"); Serial.println(isRearLightEnabled() ? "1" : "0");
    Serial.println("----"); Serial.println();
}

void loop() {
    if(millis() > ct + pause) {
        ct = millis();
        
        Serial.println(ct);

        resetLights();
        switchState();
        dumpState();

        setParkingLights();
        setEmergencyStopLights();
        setStopLights();
        setTurnLights();
        setFogLights();
        setRearLights();        
        setLights();
    }

    updateEmergencyStopLights();
    updateTurnLlights();
    updateEmergencyTurnLights();
}