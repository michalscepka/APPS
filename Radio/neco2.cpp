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
#define R	0b00000001
#define W	0b00000000

Serial pc( USBTX, USBRX );

#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

DigitalIn but9(PTC9);
DigitalIn but10(PTC10);

class Radio {
public:
	uint8_t SI_Adress;
	uint8_t PCF_Adress;
	uint8_t volume;

	Radio(uint8_t SI_Adress, uint8_t PCF_Adress){
		this->SI_Adress = SI_Adress;
		this->PCF_Adress = PCF_Adress;
	}

	void SetVolume(uint8_t volume){
		uint8_t ack = 0;
		i2c_start();
		ack |= i2c_output(SI4735_address | W);
		ack |= i2c_output(0x12);
		ack |= i2c_output(0x00);
		ack |= i2c_output(0x40);
		ack |= i2c_output(0x00);
		ack |= i2c_output(0x00);
		ack |= i2c_output(volume);
		i2c_stop();
		this->volume = volume;
		SetLEDs();
	}

	void VolumeDown(){
		if (this->volume > 0)
			SetVolume(this->volume - 1);
	}

	void VolumeUp(){
		if (this->volume < 63)
			SetVolume(this->volume + 1);
	}

	uint8_t selectLeds(uint8_t number){
			switch(number){
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

	void SetLEDs(){
		uint8_t ack = 0;
		uint8_t ledNumber= (this->volume + 1)/8;
		i2c_start();

		ack = i2c_output(this->PCF_Adress);

		ack = i2c_output(selectLeds(ledNumber));

		i2c_stop();
	}


};

int main( void )
{
	uint8_t S1, S2, RSSI, SNR, MULT, CAP;
	uint8_t ack = 0;

	i2c_init();

	pc.baud( 115200 );
	pc.printf( "K64F-KIT ready...\r\n" );

	// communication with 8 bit expander PCF8574

	// start communication
	i2c_start();

	// PCF8574 addressing
	// The address is composed from 3 parts!
	ack = i2c_output( 0b01000000 | 0b00000110 | 0b00000000 );

	// Check ack! Return value must be 0!
	// ....

	ack = i2c_output(0b10001111);
	// selected LEDs should light

	// stop communication
	i2c_stop();


	if ( ( ack = SI4735_Init() ) != 0 )
	{
		pc.printf( "Initialization of SI4735 finish with error (%d)\r\n", ack );
		return 0;
	}
	else
		pc.printf( "SI4735 initialized.\r\n" );

	pc.printf( "\nTunig of radio station...\r\n" );

	// Required frequency in MHz * 100
	int freq = 10140; // Radiozurnal

	// Tuning of radio station
	i2c_start();
	ack |= i2c_output( SI4735_address | W );
	ack |= i2c_output( 0x20 );			// FM_TUNE_FREQ
	ack |= i2c_output( 0x00 );			// ARG1
	ack |= i2c_output( freq >> 8 );		// ARG2 - FreqHi
	ack |= i2c_output( freq & 0xff );	// ARG3 - FreqLo
	ack |= i2c_output( 0x00 );			// ARG4
	i2c_stop();
	// Check ack!
	// if...

	// Tuning process inside SI4735
	wait_ms( 100 );
	printf( "... station tuned.\r\n\n" );

	// Example of reading of tuned frequency
	i2c_start();
	ack |= i2c_output( SI4735_address | W );
	ack |= i2c_output( 0x22 );			// FM_TUNE_STATUS
	ack |= i2c_output( 0x00 );			// ARG1
	// repeated start
	i2c_start();
	// change direction of communication
	ack |= i2c_output( SI4735_address | R );
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

	Radio radio(SI4735_address, 0b01000110);

	radio.SetVolume(63);

	while (true){
		if (!but9){
			radio.VolumeDown();
		}

		if (!but10){
			radio.VolumeUp();
		}
	}

	if ( ack != 0 )
		printf( "Communication error!\r\n" );
	else
		printf( "Current tuned frequency: %d.%dMHz\r\n", freq / 100, freq % 100 );

	return 0;
}

