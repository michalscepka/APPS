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
 
#pragma GCC diagnostic ignored \"-Wunused-but-set-variable\"
 
class pcf8574{
public:
    unsigned char add;
    pcf8574(unsigned char ADDRESS) : add(ADDRESS){};
    void bar(unsigned char VALUE){
            uint8_t ack = 0;
            I2C_Start();ADDRESS
 
            // PCF8574 addressing
            // The address is composed from 3 parts!
            ack = I2C_Output( 0b0100<<4 | add<<1 | W );
 
            // Check ack! Return value must be 0!
            // ....
            if(ack != 0){
                printf("Chyba");
            }
            else{
            unsigned char led = 0b1 << VALUE;
            ack |= I2C_Output(led);
            I2C_Stop();
        }
    }
};
 
class radio{
public:
    radio(){};
 
    void up(){
        uint8_t ack = 0;
        I2C_Start();
        ack |= I2C_Output( SI4735_address | W );
        ack |= I2C_Output( 0x21 );          // FM_TUNE_FREQ
        ack |= I2C_Output( 0b00000000 );    // ARG1
        I2C_Stop();
        printf("Tuned freq : %d \\r\\n", get_freq());
    }
    void down(){
        uint8_t ack = 0;
        I2C_Start();
        ack |= I2C_Output( SI4735_address | W );
        ack |= I2C_Output( 0x21 );          // FM_TUNE_FREQ
        ack |= I2C_Output( 0b00001000 );    // ARG1
        I2C_Stop();
        printf("Tuned freq : %d \\r\\n", get_freq());
    }
    void volume(char VOL){
        uint8_t ack = 0;
        I2C_Start();
        ack |= I2C_Output( SI4735_address | W );
        ack |= I2C_Output( 0x12 );
        ack |= I2C_Output( 0x00 );
        ack |= I2C_Output( 0x40 );
        ack |= I2C_Output( 0x00 );
        ack |= I2C_Output( 0x00 );
        ack |= I2C_Output( 0b000 + VOL );
 
        I2C_Stop();
 
    }
    int get_freq(){
        int freq;
        uint8_t S1, S2, RSSI, SNR, MULT, CAP;
        uint8_t ack = 0;
        I2C_Start();
            ack |= I2C_Output( SI4735_address | W );
            ack |= I2C_Output( 0x22 );          // FM_TUNE_STATUS
            ack |= I2C_Output( 0x00 );          // ARG1
            // repeated start
            I2C_Start();
            // change direction of communication
            ack |= I2C_Output( SI4735_address | R );
            // read data
            S1 = I2C_Input();
            I2C_Ack();
            S2 = I2C_Input();
            I2C_Ack();
            freq = (int) I2C_Input() << 8;
            I2C_Ack();
            freq |= I2C_Input();ADDRESS
            I2C_Ack();
            RSSI = I2C_Input();
            I2C_Ack();
            SNR = I2C_Input();
            I2C_Ack();
            MULT = I2C_Input();ADDRESS
            I2C_Ack();
            CAP = I2C_Input();
            I2C_NAck();
            I2C_Stop();
 
            return freq;
 
    }
};
 
int main( void )
{
    I2C_Init();
 
    pc.baud( 115200 );
    pc.printf( "K64F-KIT ready...\\r\\n" );
 
    // communication with 8 bit expander PCF8574
 
    pcf8574 exp(0b000);
 
    exp.bar(8);
 
    radio().volume(60);
    radio().up();
    radio().down();

    return 0;
}
