// **************************************************************************
//
//               Demo program for labs
//
// Subject:      Computer Architectures and Parallel systems
// Author:       Petr Olivka, petr.olivka@vsb.cz, 08/2016
// Organization: Department of Computer Science, FEECS,
//               VSB-Technical University of Ostrava, CZ
//
// File:         Main programm for I2C bus
//
// **************************************************************************

#include <mbed.h>

#include "i2c-lib.h"
#include "si4735-lib.h"

//************************************************************************

// Direction of I2C communication
#define R   0b00000001
#define W   0b00000000

#define SeekUP 0b00001100
#define SeekDown 0b00000100
#define HWADR_PCF8574  0b01001110
#define A012  0b1110

Serial pc( USBTX, USBRX );

DigitalIn but1(PTC9);
DigitalIn but2(PTC10);
DigitalIn but3(PTC11);
DigitalIn but4(PTC12);

#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

class Radio
{
public:
	uint8_t volume;

	//EXPANDER
	void leds_bar(uint8_t value)
    {
		uint8_t l_ack = 0;
		i2c_start();
		l_ack = i2c_output(HWADR_PCF8574 | A012);
		if (!l_ack)
			l_ack |= i2c_output(value);
		i2c_stop();
    }

	//pro volume
	uint8_t select_leds(uint8_t number)
	{
		switch(number)
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

	//RADIO
    void seek(bool up)
    {
        uint8_t ack = 0;
        i2c_start();
        ack |= i2c_output(SI4735_ADDRESS | W);
        ack |= i2c_output(0x21);          // FM_TUNE_FREQ
		if(up)
        	ack |= i2c_output(SeekUP);    // ARG1
		else
			ack |= i2c_output(SeekDown);  // ARG1
        i2c_stop();

		//RSSI
		uint8_t S1, S2, RSSI, SNR, MULT, CAP;
        uint8_t ack = 0;
		int freq;

        i2c_start();
		ack |= i2c_output(SI4735_ADDRESS | W);
		ack |= i2c_output(0x22);          // FM_TUNE_STATUS
		ack |= i2c_output(0x00);          // ARG1
		// repeated start
		i2c_start();
		// change direction of communication
		ack |= i2c_output(SI4735_ADDRESS | R);
		// read data
		S1 = i2c_input();
		i2c_ack();
		S2 = i2c_input();
		i2c_ack();
		freq = (int) i2c_input() << 8;
		i2c_ack();
		freq |= i2c_input();
		i2c_ack();
		RSSI = i2c_input();
		i2c_ack();
		SNR = i2c_input();
		i2c_ack();
		MULT = i2c_input();
		i2c_ack();
		CAP = i2c_input();
		i2c_nack();
		i2c_stop();

		leds_bar(RSSI);		//zobrazeni sily signalu pri seeku
    }

    void set_volume(uint8_t vol)
    {
        uint8_t ack = 0;
        i2c_start();
        ack |= i2c_output(SI4735_ADDRESS | W);
        ack |= i2c_output(0x12);
        ack |= i2c_output(0x00);
        ack |= i2c_output(0x40);
        ack |= i2c_output(0x00);
        ack |= i2c_output(0x00);
        ack |= i2c_output(vol);
        i2c_stop();
		volume = vol;
    }

	void volume_up(bool up)
	{
		if(up)
			if (volume > 0)
				setVolume(volume - 1);
		else
			if (volume < 63)
				setVolume(volume + 1);
	}

	void leds_bar_by_volume()
	{
		uint8_t ledNumber = (volume + 1) / 8;
		leds_bar(select_leds(ledNumber));
	}

    int get_freq()
    {
        uint8_t S1, S2, RSSI, SNR, MULT, CAP;
        uint8_t ack = 0;
		int freq;

        i2c_start();
		ack |= i2c_output(SI4735_ADDRESS | W);
		ack |= i2c_output(0x22);          // FM_TUNE_STATUS
		ack |= i2c_output(0x00);          // ARG1
		// repeated start
		i2c_start();
		// change direction of communication
		ack |= i2c_output(SI4735_ADDRESS | R);
		// read data
		S1 = i2c_input();
		i2c_ack();
		S2 = i2c_input();
		i2c_ack();
		freq = (int) i2c_input() << 8;
		i2c_ack();
		freq |= i2c_input();
		i2c_ack();
		RSSI = i2c_input();
		i2c_ack();
		SNR = i2c_input();
		i2c_ack();
		MULT = i2c_input();
		i2c_ack();
		CAP = i2c_input();
		i2c_nack();
		i2c_stop();

		return freq;
    }

	//pro freq
	uint8_t selectLedsByFreq(int freq)
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

	void qualityOfSignal()
	{
		uint8_t S1,S2,STBL,RSSI,SNR,MULT,FREQ;
		i2c_start(); // Opakovany start - zjisteni RSSI
		l_ack |= i2c_output ( SI4735_ADDRESS | W );
		l_ack |= i2c_output ( 0x22 );
		l_ack |= i2c_output ( 0x00 );
		i2c_start(); // Opakovany start - read RSSI
		l_ack |= i2c_output(SI4735_ADDRESS | R );
		S1 = i2c_input();
		i2c_ack();
		S2 = i2c_input();
		i2c_ack();
		//is_station = i2c_input();
		//i2c_ack();
		STBL = i2c_input();
		i2c_ack();
		RSSI = i2c_input();
		i2c_ack();
		SNR = i2c_input();
		i2c_ack();
		MULT = i2c_input();
		i2c_ack();
		FREQ = i2c_input();
		i2c_nack();
		i2c_stop(); // Ukonceni komunikace

		leds_bar(RSSI); // zobrazeni sily pri seeku
	}
};

int main( void )
{
    i2c_init();

    pc.baud(115200);

	uint8_t l_S1, l_S2, l_RSSI, l_SNR, l_MULT, l_CAP;
	uint8_t l_ack = 0;

    pc.printf("K64F-KIT ready...\r\n");
	i2c_init();
	// communication with 8 bit expander PCF8574
	// start communication
	i2c_start();

	// PCF8574 addressing
	// The address is composed from 3 parts!
	//l_ack = i2c_output( HWADR_PCF8574 | A012 | W );

	// Check l_ack! Return value must be 0!
	// ....

	//l_ack = i2c_output( Any_8_bit_value );
	// selected LEDs should light

	// stop communication
	i2c_stop();

    if ((l_ack = si4735_init()) != 0)
	{
		pc.printf("Initialization of SI4735 finish with error (%d)\r\n", l_ack);
		return 0;
	}
	else
		pc.printf("SI4735 initialized.\r\n");

	pc.printf("\nTunig of radio station...\r\n");

    // Required frequency in MHz * 100
	int l_freq = 10140; // Radiozurnal

	// Tuning of radio station
	i2c_start();
	l_ack |= i2c_output( SI4735_ADDRESS | W);
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
	pc.printf("... station tuned.\r\n\n");

	// Example of reading of tuned frequency
	i2c_start();
	l_ack |= i2c_output( SI4735_ADDRESS | W);
	l_ack |= i2c_output(0x22);			// FM_TUNE_STATUS
	l_ack |= i2c_output(0x00);			// ARG1
	// repeated start
	i2c_start();
	// change direction of communication
	l_ack |= i2c_output( SI4735_ADDRESS | R);
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

	if (l_ack != 0)
		pc.printf("Communication error!\r\n");
	else
		pc.printf("Current tuned frequency: %d.%dMHz\r\n", l_freq / 100, l_freq % 100);

	//-----START-----

	uint8_t b = 'S';
    b = ((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;

    Radio r1;
    r1.leds_bar(b);

    while(1)
    {
    	if(!but1)
    	{
    		r1.seek(true);
    		wait_ms(150);
			r1.leds_bar(r1.selectLedsByFreq(r1.get_freq()));
			printf("Tuned freq : %d \r\n", get_freq());
    	}

    	if(!but2)
		{
    		r1.seek(false);
			wait_ms(150);
			r1.leds_bar(r1.selectLedsByFreq(r1.get_freq()));
			printf("Tuned freq : %d \r\n", get_freq());
		}

		if(!but3)
		{
			r1.volume(true);
		}

		if(!but4)
		{
			r1.volume(false);
		}

		//r1.qualityOfSignal();
    }
    return 0;
}
