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
#define R    0b00000001
#define W    0b00000000

DigitalIn btnVolUp( PTC9 );
DigitalIn btnVolDown( PTC10 );

Serial pc( USBTX, USBRX );

#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
//demo end

class pcf8574
{
public:
    unsigned char addr;
    unsigned const char SWITCH_ADDRESS = 0b0100;

    pcf8574(unsigned char addr)
    {
        this->addr = addr;
    };

    void bar(unsigned char VALUE)
    {
        uint8_t ack = 0;

        I2C_Start();
        ack |= I2C_Output( 0b01000000 );
        ack |= I2C_Output( VALUE );
        I2C_Stop();
    };
};

class Radio
{
public:
    uint8_t ack;
    char radioCode[65];

    void up()
    {
        uint8_t ack = 0;
        I2C_Start();
        ack |= I2C_Output( SI4735_address | W );
        ack |= I2C_Output( 0x21 );
        ack |= I2C_Output( 0b00000100 );
        I2C_Stop();
    };

    void tune(int freq)
    {
        I2C_Start();
        this->ack |= I2C_Output( SI4735_address | W );
        this->ack |= I2C_Output( 0x20 );            // FM_TUNE_FREQ
        this->ack |= I2C_Output( 0x00 );            // ARG1
        this->ack |= I2C_Output( freq >> 8 );        // ARG2 - FreqHi
        this->ack |= I2C_Output( freq & 0xff );    // ARG3 - FreqLo
        this->ack |= I2C_Output( 0x00 );            // ARG4
        I2C_Stop();
    };

    void down()
    {
        uint8_t ack = 0;
        I2C_Start();
        ack |= I2C_Output( SI4735_address | W );
        ack |= I2C_Output( 0x21 );
        ack |= I2C_Output( 0b00001100 );
        I2C_Stop();
    };

    void volume(char VOL)
    {
        uint8_t ack = 0;

        I2C_Start();
        ack |= I2C_Output( SI4735_address | W );
        ack |= I2C_Output( 0x12 );
        ack |= I2C_Output( 0x00 );
        ack |= I2C_Output( 0x40 );
        ack |= I2C_Output( 0x00 );
        ack |= I2C_Output( 0x00 );
        ack |= I2C_Output( VOL );
        I2C_Stop();
    };

    unsigned char get_rssi()
    {
        uint8_t rssi;
        uint8_t ack = 0;

        I2C_Start();
        ack |= I2C_Output( SI4735_address | W );
        ack |= I2C_Output( 0x22 );
        ack |= I2C_Output( 0x00 );

        I2C_Start();
        ack |= I2C_Output( SI4735_address | R );
        I2C_Input();
        I2C_Ack();
        I2C_Input();
        I2C_Ack();
        I2C_Input();
        I2C_Ack();
        I2C_Input();
        I2C_Ack();
        rssi = I2C_Input();
        I2C_Ack();
        I2C_Input();
        I2C_Ack();
        I2C_Input();
        I2C_Ack();
        I2C_Input();
        I2C_NAck();
        I2C_Stop();

        return rssi;
    };

    void start_rds()
    {
        uint8_t ack = 0;
        I2C_Start();
        ack |= I2C_Output( SI4735_address | W );
        ack |= I2C_Output( 0x24 );
        ack |= I2C_Output( 1 );

        //inicializuje RDS
    };

    void get_rds()
    {
        uint8_t RDS0, RDS1, Sync, RDS3, B1H, B1L, B2H, B2L, B3H, B3L, B4H, B4L, last;
        int blok2;
        bool is_complete = false;

        for(int i = 0; i < 65; i++)
        {
            this->radioCode[i] = 0;
        }

        while(!is_complete)
        {
            while(1)
            {
                this->start_rds();

                I2C_Start();
                I2C_Output( SI4735_address | R );        //nastaví pro čtení
                RDS0 = I2C_Input();
                I2C_Ack();
                RDS1 = I2C_Input();
                I2C_Ack();
                Sync = I2C_Input();
                I2C_Ack();
                RDS3 = I2C_Input();
                I2C_Ack();
                B1H = I2C_Input();
                I2C_Ack();
                B1L = I2C_Input();
                I2C_Ack();
                B2H = I2C_Input();
                I2C_Ack();
                B2L = I2C_Input();
                I2C_Ack();
                B3H = I2C_Input();
                I2C_Ack();
                B3L = I2C_Input();
                I2C_Ack();
                B4H = I2C_Input();
                I2C_Ack();
                B4L = I2C_Input();
                I2C_Ack();
                last = I2C_Input();
                I2C_NAck();
                I2C_Stop();

                blok2 = B2H<<8 | B2L;

                if((blok2 & 0xF800)>>11 == 0b00100)
                {
                    break;
                }
                pc.printf("%c", (char)B2H);
            }

            int position = blok2 & 0xF;
            this->radioCode[(position*4)] = (char)B3H;
            this->radioCode[(position*4)+1] = (char)B3L;
            this->radioCode[(position*4)+2] = (char)B4H;
            this->radioCode[(position*4)+3] = (char)B4L;

            is_complete = true;
            for(int i = 0; i < 64; i++)
            {
                if(this->radioCode[i] == 0)
                {
                    is_complete = false;
                }
            }
        }

        //spustí načítání rds dat
        //zavolá i2c funkci, vrátí 4xint
        //vyzobávat a plnit postupně radio text
        //uloží řetězec jako interní proměnnou

        //předám 24 s číslem, vrací mi odpověď, když dám 24a  něc lichého tak mu řeknu předej mi RDS data, vrátí mi 13 bajtů B(n)(HI/LO)
        //po převzatí bajtů
        //první 2 bajty odpovídají pi code
        //druhé 2 bajty 2hému bloku

        //musím se podívat jaká data se zrovna přenáší
        //co se přenáší se nachází v bloku 2
        //v bloku 2 se dovím co se nachází v bloku 3 a 4
        //v 1 bytech se přenáší informace co vlastně tam je
        //podíván se do druhého 2 bloku a dívám se jestli na prvních 5 je 00100 = přenáší se radiocode
        //pokud ano začnu vyhodnocovat radiotext
        //v 3 a 4 bloku přijdou 4 písmenka jsou v asci hodontě
        //vymskovat poslední 4 bity z poslední části druhého bloku
        //najdu nějaké číslo a podle toho zjistím kolikáté písmenka z řetězce mám

        //udělám cha[65] na konec 0
        //potom cyklicky volám rds - jestli se vysílá radio text, a písmenka umístím na strpávné místo
    };

    char* get_radio_text(){
        return this->radioCode;
    }
};

int main( void )
{
    uint8_t S1, S2, RSSI, SNR, MULT, CAP;

    pc.baud( 115200 );
    pc.printf( "K64F-KIT ready...\r\n" );

    pc.printf("\n\n");

    Radio* radio = new Radio();
    radio->tune(8950); //radiozurnal
    radio->get_rds();
    char *radio_text = radio->get_radio_text();

    for(int i = 0; i < 65; i++)
    {
       pc.printf("%c", radio_text[i]);
    }
    return 0;
}
