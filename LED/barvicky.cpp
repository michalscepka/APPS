#include "mbed.h"

void demo_leds();
void demo_lcd();
void demo_i2c();

void transition();

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

#define numOfLEDs 6
float LEDIncrease = 0.1;
int FirstLEDPosition = 0;
int LastLEDPosition = 0;

//Buttons
DigitalIn NextButton(PTC9);
DigitalIn PrevButton(PTC10);



float brightnessIncrease = 0.0025;
int T = 15;
int timer = 0;
bool switcher = true;

struct LEDHolder
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

LEDHolder LEDs[numOfLEDs] =
{
{ DigitalOut(PTB9), 0 },
{ DigitalOut(PTB3), 0 },
{ DigitalOut(PTB2), 0 },
{ DigitalOut(PTB19), 0 },
{ DigitalOut(PTB18), 0 },
{ DigitalOut(PTB11), 0 } };

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


		timer++;
		transition();

	}

}

void transition()
{
	//Last LED
	if (switcher) //&& timer % 20 == 0)
	{
		LEDs[0].brightness += brightnessIncrease;
		LEDs[1].brightness -= brightnessIncrease;
		if (LEDs[1].HasMinBrigthness())
		{
			LEDs[1].brightness = 0.0;
		}
		if (LEDs[0].HasMaxBrigthness())
		{
			switcher = !switcher;
		}
	}
	else
	{
		LEDs[0].brightness -= brightnessIncrease;
		LEDs[1].brightness += brightnessIncrease;
		if (LEDs[0].HasMinBrigthness())
		{
			LEDs[0].brightness = 0.0;
		}
		if (LEDs[1].HasMaxBrigthness())
		{
			switcher = !switcher;
		}
	}
}
