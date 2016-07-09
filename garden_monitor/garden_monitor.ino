#include <Console.h>

// The time we give the sensor to calibrate
int calibrationTime = 1;

//the time when the sensor outputs a low impulse
long unsigned int detectTime = 0;
long unsigned int valveOpenTime = 0;
long unsigned int wateringTime = 0;

long unsigned int wateringPeriod = 1800000;
// The amount of milliseconds the sensor has to be low before we assume all
// motion has stopped
long unsigned int minDetectTime = 1500;
long unsigned int maxDetectTime = 5000;
long unsigned int minTimeSinceOn = 7000;

//Define the states of the machine
#define DETECT_OFF 0
#define DETECT_OFF_WAIT 1
#define DETECT_ON 2
#define DETECT_ON_WAIT 3
#define WATER_OFF 4
#define WATER_ON 5
#define WATER_ON_WAIT 6
#define BUTTON_PRESSED 7

//This is the memory element that contains the state variable
uint8_t fsm_detect_state = DETECT_OFF;
uint8_t fsm_water_state = WATER_OFF;
uint8_t fsm_water_prev_state = WATER_OFF;

// The digital pin connected to the PIR sensor's output
int pirPin = 3;
int ledPin = 13;
int valvePin = 9;
int buttonPin = 12;


//SETUP
void setup()
{
    Bridge.begin();
    Console.begin(); 

    // REMOVE IF NOT CONNECTING TO CONSOLE
    /*while (!Console){*/
        /*; // wait for Console port to connect.*/
    /*}*/
    Console.println("console connected, commencing script");

    /*Serial.begin(9600);*/

    pinMode(pirPin, INPUT);
    pinMode(ledPin, OUTPUT);
    pinMode(valvePin, OUTPUT);
    pinMode(buttonPin, INPUT);

    digitalWrite(pirPin, LOW);
    digitalWrite(valvePin, LOW);

    // Give the sensor some time to calibrate
    Console.print("calibrating sensor ");
    for(int i = 0; i < calibrationTime; i++) {
        Console.print(".");
        for(int j = 0; j < 2; j++) {
            digitalWrite(ledPin, HIGH);
            delay(100);
            digitalWrite(ledPin, LOW);
            delay(100);
        }
        delay(600);
    }
    Console.println(" done");
    Console.println("activated");
    delay(50);
}

////////////////////////////
// Loop
void loop()
{
    water();

    if(fsm_water_state == WATER_OFF) {
        detect();
    }
}

void detect() {
    // The following is a very simple state machine
    switch (fsm_detect_state) {
        case DETECT_OFF:
            if(digitalRead(pirPin) == HIGH) {
                fsm_detect_state = DETECT_OFF_WAIT;
                detectTime = millis();
                Console.println("detecting, moving to DETECT_OFF_WAIT");
            }
            break;
        case DETECT_OFF_WAIT:
            if(digitalRead(pirPin) == LOW) {
                Console.println("stopped detecting, moving to DETECT_OFF");
                fsm_detect_state = DETECT_OFF;
            }
            else if(millis() - valveOpenTime < minTimeSinceOn) {
                Console.println("not enough time since last DETECT_ON");
            }
            else if(millis() - detectTime > maxDetectTime) {
                Console.println("detecting for too long");
            }
            else if(millis() - detectTime > minDetectTime) {
                Console.println("still detecting, moving to DETECT_ON"); 
                fsm_detect_state = DETECT_ON;
            }
            break;
        case DETECT_ON:
            Console.print("Motion detected at ");
            Console.print(millis() / 1000);
            Console.println(" sec");

            Console.println("opening valve, moving to DETECT_ON_WAIT");
            digitalWrite(ledPin, HIGH);
            digitalWrite(valvePin, HIGH);

            valveOpenTime = millis();
            fsm_detect_state = DETECT_ON_WAIT;
            break;
        case DETECT_ON_WAIT:
            if(millis() - valveOpenTime > 1000) {
                Console.println("closing valve, moving to DETECT_OFF_WAIT");
                digitalWrite(ledPin, LOW);
                digitalWrite(valvePin, LOW);
                fsm_detect_state = DETECT_OFF_WAIT;
            }
            break;
        default:
            Console.println("default case reached");
            break;
    }

}

void water() {
    switch (fsm_water_state) {
        case WATER_OFF:
            fsm_water_prev_state = WATER_OFF;
            if(digitalRead(buttonPin) == HIGH) {
                fsm_water_state = BUTTON_PRESSED;
                Console.println("button pressed, moving to BUTTON_PRESSED");
            }
            break;
        case BUTTON_PRESSED:
            if(digitalRead(buttonPin) == LOW) {
                if(fsm_water_prev_state == WATER_OFF) {
                    fsm_water_state = WATER_ON;
                    Console.println("watering activated, moving to WATER_ON");
                }
                else if(fsm_water_prev_state == WATER_ON_WAIT) {
                    fsm_water_state = WATER_OFF;
                    Console.println("watering stopped, closing valve, moving to WATER_OFF");
                    digitalWrite(ledPin, LOW);
                    digitalWrite(valvePin, LOW);
                    fsm_water_state = WATER_OFF;
                }
            }
            break;
        case WATER_ON:
            Console.println("opening valve, moving to WATER_ON_WAIT");
            digitalWrite(ledPin, HIGH);
            digitalWrite(valvePin, HIGH);

            wateringTime = millis();
            fsm_water_state = WATER_ON_WAIT;
            break;
        case WATER_ON_WAIT:
            fsm_water_prev_state = WATER_ON_WAIT;
            if(millis() - wateringTime > wateringPeriod) {
                Console.println("time elapsed, closing valve, moving to WATER_OFF");
                digitalWrite(ledPin, LOW);
                digitalWrite(valvePin, LOW);
                fsm_water_state = WATER_OFF;
            }
            else if(digitalRead(buttonPin) == HIGH) {
                fsm_water_state = BUTTON_PRESSED;
                Console.println("button pressed, moving to BUTTON_PRESSED");
            }
            break;
        default:
            Console.println("default case reached");
            break;
    }
}
