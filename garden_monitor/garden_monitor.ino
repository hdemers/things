#include <Console.h>

// The time we give the sensor to calibrate
int calibrationTime = 1;

//the time when the sensor outputs a low impulse
long unsigned int lowIn;
long unsigned int highIn;
long unsigned int onTime = 0;
long unsigned int startWatering = 0;

// The amount of milliseconds the sensor has to be low before we assume all
// motion has stopped
long unsigned int pause = 3000;
long unsigned int minHighTime = 1500;
long unsigned int maxHighTime = 10000;
long unsigned int expDelay = 4000;

boolean motion = false;
boolean takeLowTime;
boolean takeHighTime = true;


//Define the states of the machine
#define ON_WAIT 0
#define ON 1
#define OFF 2
#define OFF_WAIT 3

//This is the memory element that contains the state variable
uint8_t fsm_state = OFF;

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
    while (!Console){
        ; // wait for Console port to connect.
    }
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
    /*water();*/

    if(startWatering == 0) {
        detect2();
    }
}

void detect2() {
    //state machine
    switch (fsm_state) {
        case OFF:
            if(digitalRead(pirPin) == HIGH) {
                fsm_state = OFF_WAIT;
                highIn = millis();
                Console.println("detecting, moving to OFF_WAIT");
            }
            break;
        case OFF_WAIT:
            if(digitalRead(pirPin) == LOW) {
                Console.println("stopped detecting, moving to OFF");
                fsm_state = OFF;
            }
            else if(millis() - onTime < 30000) {
                Console.println("was ON 5 seconds ago, moving to OFF");
                fsm_state = OFF;
            }
            else if(millis() - highIn > minHighTime) {
                Console.println("still detecting, moving to ON"); 
                fsm_state = ON;
            }
            break;
        case ON:
            Console.print("Motion detected at ");
            Console.print(millis() / 1000);
            Console.println(" sec");

            Console.println("opening valve, moving to ON_WAIT");
            digitalWrite(ledPin, HIGH);
            digitalWrite(valvePin, HIGH);

            onTime = millis();
            fsm_state = ON_WAIT;
            break;
        case ON_WAIT:
            if(millis() - onTime > 1000) {
                Console.println("closing valve, moving to OFF");
                digitalWrite(ledPin, LOW);
                digitalWrite(valvePin, LOW);
                fsm_state = OFF;
            }
            break;
        default:
            Console.println("default case reached");
            break;
    }

}

void water() {
    if(digitalRead(buttonPin) == HIGH){
        if(startWatering > 0 && ((millis() - startWatering) > 2000)){
            // Stop watering
            Console.println("stopping water");
            startWatering = 0;
            digitalWrite(valvePin, LOW);
        }
        else{
            Console.println("start of watering");
            startWatering = millis();
            digitalWrite(valvePin, HIGH);
        }
        delay(1000);
    }
    if(startWatering > 0){
        if((millis() - startWatering) > 1800000){
            // Stop watering
            startWatering = 0;
            Console.print("Stopping water after ");
            Console.print((millis() - startWatering) / 1000);
            Console.println(" sec");
            digitalWrite(valvePin, LOW);
        }
    }
}

////////////////////////////
// Detect motion and open valve
void detect() {
    if(digitalRead(pirPin) == HIGH) {
        if(!motion && takeHighTime) {
            highIn = millis();
            takeHighTime = false;
            Console.println("Taking high time");
        }
        if(!motion && (millis() - highIn > minHighTime)) {
            digitalWrite(ledPin, HIGH);
            digitalWrite(valvePin, HIGH);

            // Makes sure we wait for a transition to LOW before any further
            // output is made:
            motion = true;
            Console.println("---");
            Console.print("Motion detected at ");
            Console.print(millis() / 1000);
            Console.println(" sec");
            delay(50);
        }
        // If it's been too long in the high state, close the valve and 
        // backoff
        if(millis() - highIn > maxHighTime){
            digitalWrite(valvePin, LOW);
            if(expDelay >= 600000) {
                Console.println("Error! Too long in high state. Exiting.");
                delay(500);
                exit(0);
            }
            Console.print("Too long, closing valve, waiting for ");
            Console.print(expDelay / 1000);
            Console.println(" sec");
            delay(expDelay);
            expDelay = expDelay * 2;
        }
        takeLowTime = true;
    }

    if(digitalRead(pirPin) == LOW)
    {
        if(motion && takeLowTime)
        {
            // Save the time of the transition from HIGH to LOW
            lowIn = millis();
            // Make sure this is only done at the start of a LOW phase
            takeLowTime = false;
            Console.println("Taking low time");
        }
        // If the sensor is LOW for more than the given pause,
        // we assume that no more motion is going to happen
        if(motion && (millis() - lowIn > pause))
        {
            digitalWrite(ledPin, LOW);
            digitalWrite(valvePin, LOW);

            // Makes sure this block of code is only executed again after
            // a new motion sequence has been detected
            motion = false;
            Console.print("Motion ended at ");
            Console.print((millis() - pause) / 1000);
            Console.println(" sec");
            delay(50);
        }
        takeHighTime = true;
        expDelay = 4000;

    }
}
