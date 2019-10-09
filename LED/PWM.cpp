#include "mbed.h"

void demo_leds();
void demo_lcd();
void demo_i2c();

// DO NOT REMOVE OR RENAME FOLLOWING GLOBAL VARIABLES!!

// Serial line for printf output
Serial g_pc(USBTX, USBRX);

// LEDs on K64F-KIT - instances of class DigitalOut
DigitalOut g_led1(PTA1);
DigitalOut g_led2(PTA2);

// Buttons on K64F-KIT - instances of class DigitalIn
DigitalIn g_but9(PTC9);
DigitalIn g_but10(PTC10);
DigitalIn g_but11(PTC11);
DigitalIn g_but12(PTC12);

#define numOfLEDs 8
float LEDIncrease = 0.1;
int FirstLEDPosition = 0;
int LastLEDPosition = 0;

//Buttons
DigitalIn NextButton(PTC9);
DigitalIn PrevButton(PTC10);

float brightnessIncrease = 0.1;
int T = 15;

struct PWM
{
	DigitalOut LED;
	float brightness;

/*
 * @param -int from 0 - T based on number of ms passed from last cycle iteration
 * */
	void Update(int Tick)
	{
		if (Tick < (T * brightness))
		{
			LED = true;
		}
		else
		{
			LED = false;
		}
	}

//Return True if brightness is bigger than 1 (100%), false if otherwise
	bool HasMaxBrigthness()
	{
		return brightness > 1;
	}

//Return True if brightness is lower than 0 (0%), false if otherwise
	bool HasMinBrigthness()
	{
		return brightness < 0;
	}
};

PWM LEDs[numOfLEDs] =
{
	{ DigitalOut(PTC0), 0 },
	{ DigitalOut(PTC1), 0 },
	{ DigitalOut(PTC2), 0 },
	{ DigitalOut(PTC3), 0 },
	{ DigitalOut(PTC4), 0 },
	{ DigitalOut(PTC5), 0 },
	{ DigitalOut(PTC7), 0 },
	{ DigitalOut(PTC8), 0 }
};

int main()
{
// Serial line initialization
	g_pc.baud(115200);

// default demo for 2 LEDs and 4 buttons
	while (1)
	{
		for (int tick = 0; tick < T; tick++)
		{
			for (int j = 0; j < numOfLEDs; ++j)
			{
				LEDs[j].Update(tick);
			}
			wait_ms(1);
		}

//Last LED
		if (!NextButton)
			LEDs[LastLEDPosition].brightness += brightnessIncrease;

		if (LEDs[LastLEDPosition].HasMaxBrigthness())
		{
			LastLEDPosition++;
			if (LastLEDPosition > (numOfLEDs - 1))
				LastLEDPosition = 0;
		}

//First LED
		if (!PrevButton)
			LEDs[FirstLEDPosition].brightness -= brightnessIncrease;

		if (LEDs[FirstLEDPosition].HasMinBrigthness())
		{
			FirstLEDPosition++;
			if (FirstLEDPosition > (numOfLEDs - 1))
				FirstLEDPosition = 0;
		}
	}
}
