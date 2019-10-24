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

Serial pc( USBTX, USBRX );

DigitalIn but1(PTC9);
DigitalIn but2(PTC10);
DigitalIn but3(PTC11);
DigitalIn but4(PTC12);

#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

class pcf8574{
public:
    unsigned char add;
    pcf8574(unsigned char ADDRESS) : add(ADDRESS){};

    void bar(unsigned char VALUE){
		uint8_t ack = 0;
		i2c_start();

		// PCF8574 addressing
		// The address is composed from 3 parts!
		ack = i2c_output( 0b0100<<4 | add<<1 | W );

		// Check ack! Return value must be 0!
		// ....
		if(ack != 0){
			printf("Chyba");
		}
		else{
		unsigned char led = 0b1 << VALUE;
		ack |= i2c_output(led);
		i2c_stop();
		}
    }
};

class Expander{
public:

    void leds(uint8_t value)
    {
		uint8_t l_ack = 0;

		i2c_start();

		l_ack = i2c_output(0b01001110 | 0b1110);

		if (!l_ack)
			l_ack |= i2c_output(value);

		i2c_stop();
    }
};

class Radio{
public:
    Radio(){};

    void seek_up()
    {
        uint8_t ack = 0;
        i2c_start();
        ack |= i2c_output( SI4735_ADDRESS | W );
        ack |= i2c_output( 0x21 );          // FM_TUNE_FREQ
        ack |= i2c_output( 0b00000000 );    // ARG1
        i2c_stop();
        printf("Tuned freq : %d \r\n", get_freq());
    }

    void seek_down()
    {
        uint8_t ack = 0;
        i2c_start();
        ack |= i2c_output( SI4735_ADDRESS  | W );
        ack |= i2c_output( 0x21 );          // FM_TUNE_FREQ
        ack |= i2c_output( 0b00001000 );    // ARG1
        i2c_stop();
        printf("Tuned freq : %d \r\n", get_freq());
    }

    void volume(char VOL)
    {
        uint8_t ack = 0;
        i2c_start();
        ack |= i2c_output( SI4735_ADDRESS  | W );
        ack |= i2c_output( 0x12 );
        ack |= i2c_output( 0x00 );
        ack |= i2c_output( 0x40 );
        ack |= i2c_output( 0x00 );
        ack |= i2c_output( 0x00 );
        ack |= i2c_output( 0b000 + VOL );

        i2c_stop();

    }
    int get_freq()
    {
        int freq;
        uint8_t S1, S2, RSSI, SNR, MULT, CAP;
        uint8_t ack = 0;
        i2c_start();
		ack |= i2c_output( SI4735_ADDRESS  | W );
		ack |= i2c_output( 0x22 );          // FM_TUNE_STATUS
		ack |= i2c_output( 0x00 );          // ARG1
		// repeated start
		i2c_start();
		// change direction of communication
		ack |= i2c_output( SI4735_ADDRESS | R );
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
};

int main( void )
{
    i2c_init();

    pc.baud( 115200 );
    pc.printf( "K64F-KIT ready...\r\n" );

    // communication with 8 bit expander PCF8574

    //pcf8574 exp(0b01000110);
    //exp.bar(0b00000001);

    uint8_t b = 'S';
    b = ((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;

    Expander expander;
    expander.leds(b);

    Radio r1;
    r1.volume();

    uint8_t l_ack = 0;

    if ((l_ack = si4735_init()) != 0)
	{
		pc.printf("Initialization of SI4735 finish with error (%d)\r\n",
				l_ack);
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

    while(1)
    {
    	if(!but1)
    	{
    		r1.seek_up();

    		wait_ms(150);
    	}

    	if(!but2)
		{
    		r1.seek_down();

			wait_ms(150);
		}
    }

    return 0;
}
