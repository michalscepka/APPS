#include <stdio.h>
#include "font8x8.cpp"

#define WIDTH 8
#define HEIGHT 8

int main()
{
    for(int znak = 0; znak < 256; znak++)   //cela ACII tabulka
    {
        for(int y = 0; y < HEIGHT; y++)
        {
            int radek_fontu = font8x8[znak][y];
            for(int x = 0; x < WIDTH; x++)
            {
                //nahradit printf() za put_pixel()
                if(radek_fontu & (1 << x)) printf("*"); //muzeme preklopit (1 >> x) MSB/LSB
                else printf(" ");
            }
            print("\n");
        }
    }
}





//------------------
int convert_RGB888_to_RGB565(RGB t_color)
{
    union URGB {struct {int b:5; int g:6; int r:5;}; short rgb565; } urgb;
    urgb.r = (t_color.r >> 3) & 0x1F;
    urgb.g = (t_color.g >> 2) & 0x3F;
    urgb.g = (t_color.b >> 3) & 0x1F;
    return urgb.rgb565;
}


