/*
Example Timer1 Interrupt
Flash LED every second
*/

//Github request test 2

#pragma SPARK_NO_PREPROCESSOR
//#include "application.h"

#define ledPin 13
#define mistMaker 3
#define indicator 12
int timer1_counter;
int seconds = 0;
int minutes= 0;

bool thirtyMinMarkOn = false;
bool thirtyMinMarkOff = false;


void setup()
{
	pinMode(ledPin, OUTPUT);

	Serial.begin(9600);

	// initialize timer1
	noInterrupts();           // disable all interrupts
	TCCR1A = 0;
	TCCR1B = 0;

	// Set timer1_counter to the correct value for our interrupt interval
	//timer1_counter = 64911;   // preload timer 65536-16MHz/256/100Hz
	//timer1_counter = 64286;   // preload timer 65536-16MHz/256/50Hz
	timer1_counter = 34286;   // preload timer 65536-16MHz/256/2Hz

	TCNT1 = timer1_counter;   // preload timer
	TCCR1B |= (1 << CS12);    // 256 prescaler
	TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
	interrupts();             // enable all interrupts

	pinMode(mistMaker, OUTPUT);
	pinMode(indicator, OUTPUT);
}


ISR(TIMER1_OVF_vect)        // interrupt service routine
{
	TCNT1 = timer1_counter;   // preload timer
	digitalWrite(ledPin, digitalRead(ledPin) ^ 1);
	seconds++;
	//Serial.println(seconds);

		if (seconds==120)
		{
			minutes++;
			Serial.println(minutes);
			seconds=0;
    }

		if (minutes < 30)
		{
			thirtyMinMarkOn = true;
			thirtyMinMarkOff = false;
		}
		if (minutes > 30)
		{
		thirtyMinMarkOn = false;
		thirtyMinMarkOff = true;
   	}
		if (minutes>60)
		{
			minutes=0;
		}
	}

void loop()
{

		while (thirtyMinMarkOn)
		{
			digitalWrite(3, HIGH);
			digitalWrite(12, HIGH);
		}

		while (thirtyMinMarkOff)
		{
			digitalWrite(3, LOW);
			digitalWrite(12, LOW);
		}

}
