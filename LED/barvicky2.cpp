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

DigitalOut LedArray[8] = { DigitalOut(PTC0),DigitalOut(PTC1),DigitalOut(PTC2),DigitalOut(PTC3),
 DigitalOut(PTC4),DigitalOut(PTC5),DigitalOut(PTC7),DigitalOut(PTC8) };


int pos1 = 0;
int pos2 = 0;

bool pressed = false;


int T = 15;

struct MYPWM
{
	DigitalOut LED;
	float brightness;

	void LightUp(int value)
	{
		if (value < (T * brightness))
		{
			LED = true;
		}
		else
		{
			LED = false;
		}
	};
};

MYPWM LEDIODS[8] = { {DigitalOut(PTC0), 0}, {DigitalOut(PTC1), 0}, {DigitalOut(PTC2), 0}, {DigitalOut(PTC3), 0}, {DigitalOut(PTC4), 0}, {DigitalOut(PTC5), 0}, {DigitalOut(PTC7), 0}, {DigitalOut(PTC8), 0} };

MYPWM rgb1[3] = { {DigitalOut(PTB9), 0}, {DigitalOut(PTB3), 0}, {DigitalOut(PTB2), 0} };
MYPWM rgb2[3] = { {DigitalOut(PTB19), 0}, {DigitalOut(PTB18), 0}, {DigitalOut(PTB11), 0} };

void ledsLightUp()
{
	if (!g_but9)
		LEDIODS[pos2].brightness += 0.05;

	if (LEDIODS[pos2].brightness > 1)
	{
		pos2++;

		if (pos2 > 7)
			pos2 = 0;
	}
}

void ledsLightDown()
{
	if (!g_but10)
		LEDIODS[pos1].brightness -= 0.05;

	if (LEDIODS[pos1].brightness < 0)
	{
		pos1++;

		if (pos1 > 7)
			pos1 = 0;
	}
}

void randomColor()
{
	for (int i = 0; i < 5; i++)
	{
		//rgb2[i].brightness = (rand()%100)/100.0;
		g_pc.printf("%d\r\n", rand() % 100);
	}
}

int main()
{
	// Serial line initialization
	g_pc.baud(115200);

	// uncomment selected demo
	//demo_leds();  count++;
	//demo_lcd();
	//demo_i2c();

	g_pc.printf("Demo program started...\r\n");

	// ******************************************************************

	// default demo for 2 LEDs and 4 buttons

	srand(time(NULL));

	randomColor();

	while (1)
	{
		/*for(int i = 0;i < T;i++)
		{
		 for(int j = 0;j < 8;j++)
		 {
		  LEDIODS[j].LightUp(i);
		 }
		 wait_ms(1);
		}*/

		//ledsLightUp();

		//ledsLightDown();

		for (int i = 0; i < T; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				rgb1[j].LightUp(i);
				rgb2[j].LightUp(i);

			}
			wait_ms(1





			);
		}


		if (!g_but9)
		{
			if (!g_but10)
				rgb1[0].brightness -= 0.01;

			if (!g_but11)
				rgb1[1].brightness -= 0.01;

			if (!g_but12)
				rgb1[2].brightness -= 0.01;
		}
		else
		{
			if (!g_but10)
				rgb1[0].brightness += 0.01;

			if (!g_but11)
				rgb1[1].brightness += 0.01;

			if (!g_but12)
				rgb1[2].brightness += 0.01;
		}



	}

}





