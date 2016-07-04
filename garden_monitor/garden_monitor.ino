// The time we give the sensor to calibrate (10-60 secs according to the
// datasheet)
int calibrationTime = 60;

//the time when the sensor outputs a low impulse
long unsigned int lowIn;
long unsigned int highIn;
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

// The digital pin connected to the PIR sensor's output
int pirPin = 3;
int ledPin = 13;
int valvePin = 9;
int buttonPin = 12;


//SETUP
void setup()
{
	Serial.begin(9600);
	pinMode(pirPin, INPUT);
	pinMode(ledPin, OUTPUT);
	pinMode(valvePin, OUTPUT);
	pinMode(buttonPin, INPUT);

	digitalWrite(pirPin, LOW);
	digitalWrite(valvePin, LOW);

	// Give the sensor some time to calibrate
	Serial.print("Calibrating sensor ");
	for(int i = 0; i < calibrationTime; i++)
	{
		Serial.print(".");
		for(int j = 0; j < 2; j++)
		{
			digitalWrite(ledPin, HIGH);
			delay(100);
			digitalWrite(ledPin, LOW);
			delay(100);
		}
		delay(600);
	}
	Serial.println(" done");
	Serial.println("Activated");
	delay(50);
}

////////////////////////////
// Loop
void loop()
{
	if(digitalRead(buttonPin) == HIGH){
		if(startWatering > 0 && ((millis() - startWatering) > 2000)){
			// Stop watering
			Serial.println("Stopping water");
			startWatering = 0;
			digitalWrite(valvePin, LOW);
		}
		else{
			Serial.println("Start of watering");
			startWatering = millis();
			digitalWrite(valvePin, HIGH);
		}
		delay(1000);
	}
	if(startWatering > 0){
		if((millis() - startWatering) > 1800000){
			// Stop watering
			startWatering = 0;
			Serial.print("Stopping water after ");
			Serial.print((millis() - startWatering) / 1000);
			Serial.println(" sec");
			digitalWrite(valvePin, LOW);
		}
	}
	if(startWatering == 0) {
		detect();
	}
}

////////////////////////////
// Detect motion and open valve
void detect()
{
	if(digitalRead(pirPin) == HIGH)
	{
		if(!motion && takeHighTime)
		{
			highIn = millis();
			takeHighTime = false;
			Serial.println("Taking high time");
		}
		if(!motion && (millis() - highIn > minHighTime))
		{
			digitalWrite(ledPin, HIGH);
			digitalWrite(valvePin, HIGH);

			// Makes sure we wait for a transition to LOW before any further
			// output is made:
			motion = true;
			Serial.println("---");
			Serial.print("Motion detected at ");
			Serial.print(millis() / 1000);
			Serial.println(" sec");
			delay(50);
		}
		// If it's been too long in the high state, close the valve and 
		// backoff
		if(millis() - highIn > maxHighTime){
			digitalWrite(valvePin, LOW);
			if(expDelay >= 600000) {
				Serial.println("Error! Too long in high state. Exiting.");
				delay(500);
				exit(0);
			}
			Serial.print("Too long, closing valve, waiting for ");
			Serial.print(expDelay / 1000);
			Serial.println(" sec");
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
			Serial.println("Taking low time");
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
			Serial.print("Motion ended at ");
			Serial.print((millis() - pause) / 1000);
			Serial.println(" sec");
			delay(50);
		}
		takeHighTime = true;
		expDelay = 4000;
		
	}
}
