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

#include "mbed.h"

#include "i2c-lib.h"
#include "si4735-lib.h"

#include <string>
//************************************************************************

// Read nebo Write po i2c
#define R	0b00000001
#define W	0b00000000
#define SeekUP 0b00001100
#define SeekDown 0b00000100
#define HWADR_PCF8574  0b01000000
#define A012  0b0110

DigitalIn btn9(PTC9);
DigitalIn btn10(PTC10);
DigitalIn btn11(PTC11);
DigitalIn btn12 (PTC12);
DigitalOut led1(PTA1);

// Serial line for printf output
Serial g_pc(USBTX, USBRX);

#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

/*
Trida Radio
*/
class Radio{
public:
	uint8_t l_S1, l_S2, l_RSSI, l_SNR, l_MULT, l_CAP;
	uint8_t l_ack = 0;
	void radio_init();
	void led_login();
	void seek_freq();
	void set_freq();
	void strenghtOfSignal();
	void change_volume();
	void increase_volume();
	void decrease_volume();
	void led_strength(int _RSSI);
	void qualityOfSignal();
};

void Radio::radio_init()
{
	g_pc.baud(115200); // inicializace serial linky
	g_pc.printf("K64F-KIT ready...\r\n");
    i2c_init();
    // start communication
    i2c_start();

    // PCF8574 addressing
    // The address is composed from 3 parts!
    //ack = I2C_Output( HWADR_PCF8574 | A012 | W );

    // Check ack! Return value must be 0!
    // ....

    //ack = I2C_Output( Any_8_bit_value );
    // selected LEDs should light

    // stop communication
    i2c_stop();


    if ((l_ack = si4735_init()) != 0)
    {
        g_pc.printf("Initialization of SI4735 finish with error (%d)\r\n",
                l_ack);
    }
    else
        g_pc.printf("SI4735 initialized.\r\n");

    g_pc.printf("\nTunig of radio station...\r\n");
}

void Radio::led_login()
{
    i2c_start();
    l_ack |= i2c_output( HWADR_PCF8574 | A012 | W);
    l_ack |= i2c_output( 0b00100011 );
    i2c_stop();
}

void Radio::led_strength(int _RSSI)
{
    i2c_start();
    l_ack |= i2c_output( HWADR_PCF8574 | A012 | W);
    l_ack |= i2c_output( _RSSI );
    i2c_stop();
}

void Radio::seek_freq()
{
	uint8_t S1,S2,STBL,RSSI,SNR,MULT,FREQ;
	//uint8_t radFREQ[6];
	i2c_start(); // Start komunikace - Odeslani prikazu
	l_ack |= i2c_output ( SI4735_ADDRESS | W );
	l_ack |= i2c_output ( 0x21 );
	l_ack |= i2c_output ( SeekUP );
	i2c_stop();
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

	g_pc.printf("... signal radia - %d\n",RSSI);
	led_strength(RSSI); // zobrazeni sily pri seeku

}

void Radio::set_freq()
{
    uint8_t freq=9370; //Helax
    uint8_t ack = 0;
    // Required frequency in MHz * 100


    // Tuning of radio station
    i2c_start();
    l_ack |= i2c_output( SI4735_ADDRESS | W );
    l_ack |= i2c_output( 0x20 );            // FM_TUNE_FREQ
    l_ack |= i2c_output( 0x00 );            // ARG1
    l_ack |= i2c_output( freq >> 8 );        // ARG2 - FreqHi
    l_ack |= i2c_output( freq & 0xff );    // ARG3 - FreqLo
    l_ack |= i2c_output( 0x00 );            // ARG4
    i2c_stop();

    //pc.printf( \"FREQ %d, %d\\r\\n\", freq,is_station);
    qualityOfSignal();

    //setBarLedsFromStrenghtofSignal();
    // Check ack!
    // if...

}

void Radio::qualityOfSignal()
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

    led_strength(RSSI); // zobrazeni sily pri seeku
}

Radio radio = Radio();

int main(void)
{

    radio.radio_init();
    radio.led_login();
    while(1){
        radio.seek_freq();
        radio.qualityOfSignal();
        //radio.set_freq();
    }
    return 0;
}
