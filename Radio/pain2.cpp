// **************************************************************************
//
//               Demo program for labs
//
// Subject:      Computer Architectures and Parallel systems
// Author:       Petr Olivka, petr.olivka@vsb.cz, 09/2019
// Organization: Department of Computer Science, FEECS,
//               VSB-Technical University of Ostrava, CZ
//
// File:         Main program for I2C bus
//
// **************************************************************************

#include <mbed.h>

#include "i2c-lib.h"
#include "si4735-lib.h"

//************************************************************************

// Direction of I2C communication
#define R	0b00000001
#define W	0b00000000

// Serial line for printf output
Serial g_pc(USBTX, USBRX);

DigitalIn g_but9(PTC9);
DigitalIn g_but10(PTC10);

DigitalIn g_but11(PTC11);
DigitalIn g_but12(PTC12);

#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

class Expander
{
public:

	void startLeds(uint8_t value)
	{
		uint8_t l_ack = 0;

		i2c_start();

		l_ack = i2c_output(0b01000000);

		if (!l_ack)
			l_ack |= i2c_output(value);
		else
			l_ack |= i2c_output(0b11111111);

		i2c_stop();
	}

	uint8_t selectLeds(uint8_t number)
	{
		switch (number)
		{
            case 0:
                return 0b00000000;
            case 1:
                return 0b00000001;
            case 2:
                return 0b00000011;
            case 3:
                return 0b00000111;
            case 4:
                return 0b00001111;
            case 5:
                return 0b00011111;
            case 6:
                return 0b00111111;
            case 7:
                return 0b01111111;
            case 8:
                return 0b11111111;
        }
		return 0;
	}
};

Expander expander;

class Radio
{
public:
	uint8_t volume;

	void seekStation(bool higher)
	{
		uint8_t l_ack = 0;
		i2c_start();
		l_ack |= i2c_output(SI4735_ADDRESS | W);
		l_ack |= i2c_output(0x21);
		if (higher)
			l_ack |= i2c_output(0b00001100);
		else
			l_ack |= i2c_output(0b00000100);

		i2c_stop();
	}

	void setVolume(uint8_t vol)
	{
		uint8_t l_ack = 0;
		i2c_start();
		l_ack |= i2c_output(SI4735_ADDRESS | W);
		l_ack |= i2c_output(0x12);
		l_ack |= i2c_output(0x00);
		l_ack |= i2c_output(0x40);
		l_ack |= i2c_output(0x00);
		l_ack |= i2c_output(0x00);
		l_ack |= i2c_output(vol);
		i2c_stop();
		volume = vol;
		ledsByVolume();
	}

	void decreaseVolume()
	{
		if (volume > 0)
			setVolume(volume - 1);
	}

	void increaseVolume()
	{
		if (volume < 63)
			setVolume(volume + 1);
	}

	void ledsByVolume()
	{
		uint8_t ledNumber = (volume + 1) / 8;
		expander.startLeds(expander.selectLeds(ledNumber));
	}

	int getTunedFreq()
	{
		uint8_t l_S1, l_S2, l_RSSI, l_SNR, l_MULT, l_CAP;
		uint8_t l_ack = 0;
		int l_freq;

		i2c_start();
		l_ack |= i2c_output(SI4735_ADDRESS | W);
		l_ack |= i2c_output(0x22);			// FM_TUNE_STATUS
		l_ack |= i2c_output(0x00);			// ARG1
		// repeated start
		i2c_start();
		// change direction of communication
		l_ack |= i2c_output(SI4735_ADDRESS | R);
		// read data
		l_S1 = i2c_input();
		i2c_ack();
		l_S2 = i2c_input();
		i2c_ack();
		l_freq = (int) i2c_input() << 8;
		i2c_ack();
		l_freq |= i2c_input();
		i2c_ack();
		l_RSSI = i2c_input();
		i2c_ack();
		l_SNR = i2c_input();
		i2c_ack();
		l_MULT = i2c_input();
		i2c_ack();
		l_CAP = i2c_input();
		i2c_nack();
		i2c_stop();

		return l_freq;
	}

	uint8_t getBarByFreq(int freq)
	{
		int freqM = (freq - 7600) / (100 * 4);
		switch (freqM)
		{
		case 0:
			return 0b00000000;
		case 1:
			return 0b00000001;
		case 2:
			return 0b00000010;
		case 3:
			return 0b00000100;
		case 4:
			return 0b00001000;
		case 5:
			return 0b00010000;
		case 6:
			return 0b00100000;
		case 7:
			return 0b01000000;
		case 8:
			return 0b10000000;
		}
		return 0;
	}

	void setLedsByFreq(uint8_t num)
	{
		uint8_t l_ack = 0;
		i2c_start();
		l_ack = i2c_output(0b01000000);
		if (!l_ack)
			l_ack |= i2c_output(num);
		else
			l_ack |= i2c_output(0b11111111);
		i2c_stop();
	}
};

int main(void)
{
	// Serial line initialization

	g_pc.baud(115200);

	uint8_t l_S1, l_S2, l_RSSI, l_SNR, l_MULT, l_CAP;
	uint8_t l_ack = 0;

	g_pc.printf("K64F-KIT ready...\r\n");

	i2c_init();

	i2c_start();

	i2c_stop();

	if ((l_ack = si4735_init()) != 0)
	{
		g_pc.printf("Initialization of SI4735 finish with error (%d)\r\n",
				l_ack);
		return 0;
	}
	else
		g_pc.printf("SI4735 initialized.\r\n");

	g_pc.printf("\nTunig of radio station...\r\n");

	// Required frequency in MHz * 100
	int l_freq = 10140; // Radiozurnal

	// Tuning of radio station
	i2c_start();
	l_ack |= i2c_output(SI4735_ADDRESS | W);
	l_ack |= i2c_output(0x20);			// FM_TUNE_FREQ
	l_ack |= i2c_output(0x00);			// ARG1
	l_ack |= i2c_output(l_freq >> 8);	// ARG2 - FreqHi
	l_ack |= i2c_output(l_freq & 0xff);	// ARG3 - FreqLo
	l_ack |= i2c_output(0x00);			// ARG4
	i2c_stop();
	// Check l_ack!
	// if...

	// Tuning process inside SI4735
	wait_ms(100);
	g_pc.printf("... station tuned.\r\n\n");

	// Example of reading of tuned frequency
	i2c_start();
	l_ack |= i2c_output(SI4735_ADDRESS | W);
	l_ack |= i2c_output(0x22);			// FM_TUNE_STATUS
	l_ack |= i2c_output(0x00);			// ARG1
	// repeated start
	i2c_start();
	// change direction of communication
	l_ack |= i2c_output(SI4735_ADDRESS | R);
	// read data
	l_S1 = i2c_input();
	i2c_ack();
	l_S2 = i2c_input();
	i2c_ack();
	l_freq = (int) i2c_input() << 8;
	i2c_ack();
	l_freq |= i2c_input();
	i2c_ack();
	l_RSSI = i2c_input();
	i2c_ack();
	l_SNR = i2c_input();
	i2c_ack();
	l_MULT = i2c_input();
	i2c_ack();
	l_CAP = i2c_input();
	i2c_nack();
	i2c_stop();

	Radio rad;
	uint8_t v = 'M';

	while (true)
	{
		//expander.startLeds(~v);
		//expander.startLeds(rad.getRSSI());

		if (!g_but9)
		{
			rad.seekStation(true);
			wait_ms(150);
			rad.setLedsByFreq(rad.getBarByFreq(rad.getTunedFreq()));
			g_pc.printf("%d\r\n", rad.getTunedFreq());
		}

		if (!g_but10)
		{
			rad.seekStation(false);
			wait_ms(150);
			rad.setLedsByFreq(rad.getBarByFreq(rad.getTunedFreq()));
			g_pc.printf("%d\r\n", rad.getTunedFreq());
		}

		if (!g_but11)
		{
			rad.increaseVolume();
		}

		if (!g_but12)
		{
			rad.decreaseVolume();
		}
	}
	return 0;
}
