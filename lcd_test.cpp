// **************************************************************************
//
//               Demo program for labs
//
// Subject:      Computer Architectures and Parallel systems
// Author:       Petr Olivka, petr.olivka@vsb.cz, 08/2016
// Organization: Department of Computer Science, FEECS,
//               VSB-Technical University of Ostrava, CZ
//
// File:         Main program for LCD module
//
// **************************************************************************

#include "mbed.h"
#include "lcd_lib.h"
#include <string>
//#include "font8x8.cpp"		//neodkomentovavat
//#include "font22x36_msb.h"

#define WIDTH 8
#define HEIGHT 8
//#define WIDTH 36
//#define HEIGHT 36

// Serial line for printf output
Serial pc(USBTX, USBRX);

// two dimensional array with fixed size font
extern uint8_t font8x8[256][8];		//odkomenovat u 8x8

using namespace std;

DigitalOut bl(PTC3, 0);		// backlight

int offset = 8;
int dist = 1;
bool sviti = true;
int T = 15;

// Simple graphic interface

struct Point2D
{
    int32_t x, y;
};

struct RGB
{
    uint8_t r, g, b;
};

class GraphElement
{
public:
    // foreground and background color
    RGB fg_color, bg_color;

    // constructor
    GraphElement( RGB t_fg_color, RGB t_bg_color ) :
        fg_color( t_fg_color ), bg_color( t_bg_color ) {}

    // ONLY ONE INTERFACE WITH LCD HARDWARE!!!
    void drawPixel( int32_t t_x, int32_t t_y ) { lcd_put_pixel( t_x, t_y, convert_RGB888_to_RGB565( fg_color ) ); }

    // Draw graphics element
    virtual void draw() = 0;

    // Hide graphics element
    virtual void hide() { swap_fg_bg_color(); draw(); swap_fg_bg_color(); }
    //virtual void hideVert() { swap_fg_bg_color(); drawVert(); swap_fg_bg_color(); } //pridal jsem ja
private:
    // swap foreground and backgroud colors
    void swap_fg_bg_color() { RGB l_tmp = fg_color; fg_color = bg_color; bg_color = l_tmp; }

    // conversion of 24-bit RGB color into 16-bit color format
    int convert_RGB888_to_RGB565( RGB t_color )
    {
        union URGB {struct {int b:5; int g:6; int r:5;}; short rgb565; } urgb;
        urgb.r = (t_color.r >> 3) & 0x1F;
        urgb.g = (t_color.g >> 2) & 0x3F;
        urgb.b = (t_color.b >> 3) & 0x1F;
        return urgb.rgb565;
    }
};

class Pixel : public GraphElement
{
public:
    // constructor
    Pixel( Point2D t_pos, RGB t_fg_color, RGB t_bg_color ) : pos( t_pos ), GraphElement( t_fg_color, t_bg_color ) {}
    // Draw method implementation
    virtual void draw() { drawPixel( pos.x, pos.y ); }
    // Position of Pixel
    Point2D pos;
};


class Circle : public GraphElement
{
public:
    // Center of circle
    Point2D center;
    // Radius of circle
    int32_t radius;

    Circle( Point2D t_center, int32_t t_radius, RGB t_fg, RGB t_bg ) :
        center( t_center ), radius( t_radius ), GraphElement( t_fg, t_bg ) {};

    void draw()
    {
        int f = 1 - radius;
        int ddF_x = 0;
        int ddF_y = -2 * radius;
        int x = 0;
        int y = radius;

        int x0 = center.x;
        int y0 = center.y;

        drawPixel(x0, y0 + radius);
        drawPixel(x0, y0 - radius);
        drawPixel(x0 + radius, y0);
        drawPixel(x0 - radius, y0);

        while(x < y)
        {
            if(f >= 0)
            {
                y--;
                ddF_y += 2;
                f += ddF_y;
            }
            x++;
            ddF_x += 2;
            f += ddF_x + 1;
            drawPixel(x0 + x, y0 + y);
            drawPixel(x0 - x, y0 + y);
            drawPixel(x0 + x, y0 - y);
            drawPixel(x0 - x, y0 - y);
            drawPixel(x0 + y, y0 + x);
            drawPixel(x0 - y, y0 + x);
            drawPixel(x0 + y, y0 - x);
            drawPixel(x0 - y, y0 - x);
        }
    }
};

class Character : public GraphElement
{
public:
    // position of character
    Point2D pos;
    // character
    char character;

    Character( Point2D t_pos, char t_char, RGB t_fg, RGB t_bg ) :
      pos( t_pos ), character( t_char ), GraphElement( t_fg, t_bg ) {};

    void draw()
    {
        for(int y = 0; y < HEIGHT; y++)
        {
            int radek_fontu = font8x8[character][y];
            for(int x = 0; x < WIDTH; x++)
            {
                if(radek_fontu & (1 << x)) drawPixel(pos.x + x, pos.y + y); //muzeme preklopit (1 >> x) MSB/LSB
            }
        }
    }
};

class Line : public GraphElement
{
public:
    // the first and the last point of line
    Point2D pos1, pos2;

    Line( Point2D t_pos1, Point2D t_pos2, RGB t_fg, RGB t_bg ) :
      pos1( t_pos1 ), pos2( t_pos2 ), GraphElement( t_fg, t_bg ) {}

    void draw()
    {
        int x0 = pos1.x;
        int y0 = pos1.y;
        int x1 = pos2.x;
        int y1 = pos2.y;

        int dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2; //error value e_xy

        for(;;){  //loop
            drawPixel(x0, y0);
            if (x0 == x1 && y0 == y1) break;
            e2 = 2*err;
            if (e2 >= dy) { err += dy; x0 += sx; } //e_xy+e_x > 0
            if (e2 <= dx) { err += dx; y0 += sy; } //e_xy+e_y < 0
        }
    }
};

class Text : public GraphElement
{
public:
    // position of character
    Point2D pos;
    // characters
    string str;

    bool horizontal = false;

    Text( Point2D t_pos, string t_str, RGB t_fg, RGB t_bg, bool horizontal ) :
      pos( t_pos ), str( t_str ), GraphElement( t_fg, t_bg )
        {
            this->horizontal = horizontal;
        };

    void draw()
    {
        int offs = 0;
        for (int i = 0; i < str.size(); i++)
        {
            for(int y = 0; y < HEIGHT; y++)
            {
                int radek_fontu = font8x8[str[i]][y];
                for(int x = 0; x < WIDTH; x++)
                {
                    if(horizontal)
                    {
                        if(radek_fontu & (1 << x))
                            drawPixel(pos.x + x + offs, pos.y + y);
                    }
                    else
                    {
                        if(radek_fontu & (1 << x))
                            drawPixel(pos.x + x, pos.y + y + offs);
                    }
                }
            }
            offs += offset;
        }
    }

    void move()
    {
        hide();
        pos.x += dist;
        draw();
    }
};

Point2D point1 = {10, 10};
Point2D point2 = {120, 120};
Point2D point3 = {100, 100};
Point2D point4 = {100, 50};
RGB black = {0, 0, 0};
RGB white = {255, 255, 255};
RGB bordo = {128, 0, 32};
RGB cyan = {0, 255, 255};

Text text1(point1, "Kokos", cyan, black, true);

Character dots[2] =
{
	Character({point4.x + offset * 2, point4.y}, ':', white, black),
	Character({point4.x + offset * 5, point4.y}, ':', white, black)
};

Character watch[6] =
{
	Character({point4.x, point4.y}, '0', white, black),
	Character({point4.x + offset, point4.y}, '0', white, black),
	Character({point4.x + offset * 3, point4.y}, '0', white, black),
	Character({point4.x + offset * 4, point4.y}, '0', white, black),
	Character({point4.x + offset * 6, point4.y}, '0', white, black),
	Character({point4.x + offset * 7, point4.y}, '0' - 1, white, black)
};



struct PWM
{
	DigitalOut LED;
	float brightness;

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

void dotsBlick()
{
	if(!sviti)
	{
		for (int i = 0; i < 2; i++)
		{
			dots[i].draw();
		}
		sviti = !sviti;
	}
	else
	{
		for (int i = 0; i < 2; i++)
		{
			dots[i].hide();
		}
		sviti = !sviti;
	}
}

void clocks()
{
	for(int i = 0; i < 6; i++)
	{
		watch[i].hide();
	}

	if(watch[5].character >= '9')
	{
		watch[5].character = '0';

		if(watch[4].character >= '5')
		{
			watch[4].character = '0';

			if(watch[3].character >= '9')
			{
				watch[3].character = '0';

				if(watch[2].character >= '5')
				{
					watch[2].character = '0';

					if(watch[1].character >= '9')
					{
						watch[1].character = '0';

						if(watch[0].character == '2')
						{
							watch[0].character = '0';
						}
						else
							watch[0].character++;
					}
					else
						watch[1].character++;
				}
				else
					watch[2].character++;
			}
			else
				watch[3].character++;
		}
		else
			watch[4].character++;
	}
	else
		watch[5].character++;

	if (watch[0].character >= '2' && watch[1].character >= '4')
	{
		for(int i = 0; i < 6; i++)
		{
			watch[i].hide();
		}
		for(int i = 0; i < 6; i++)
		{
			watch[i].character = '0';
		}
	}

	for(int i = 0; i < 6; i++)
	{
		watch[i].draw();
	}
}

void move()
{
	text1.hide();
	text1.pos.x += dist;
	text1.draw();
}

//ODTUD
DigitalOut led1(PTA1);
InterruptIn button1(PTC9);
volatile bool button1_pressed = false; // Used in the main loop
volatile bool button1_enabled = true; // Used for debouncing
Timeout button1_timeout; // Used for debouncing

// Enables button when bouncing is over
void button1_enabled_cb(void)
{
    button1_enabled = true;
}

// ISR handling button pressed event
void button1_onpressed_cb(void)
{
    if (button1_enabled) { // Disabled while the button is bouncing
        button1_enabled = false;
        button1_pressed = true; // To be read by the main loop
        button1_timeout.attach(callback(button1_enabled_cb), 0.2); // Debounce time 300 ms
    }
}
//POTUD

int main()
{
	// Serial line initialization
	pc.baud(115200);

 	lcd_init();				// LCD initialization

	lcd_clear();			// LCD clear screen

	/*int l_color_red = 0xF800;
	int l_color_green = 0x07E0;
	int l_color_blue = 0x001F;
	int l_color_white = 0xFFFF;

	// simple animation display four color square using LCD_put_pixel function
	int l_limit = 200;
	for (int ofs = 0; ofs < 20; ofs++) // square offset in x and y axis
		for (int i = 0; i < l_limit; i++)
		{
			lcd_put_pixel(ofs + i, ofs + 0, l_color_red);
			//lcd_put_pixel(ofs + 0, ofs + i, l_color_green);
			lcd_put_pixel(ofs + i, ofs + l_limit, l_color_blue);
			lcd_put_pixel(ofs + l_limit, ofs + i, l_color_white);
		}*/

	Ticker t1, t2, t3;

	//PWM backl = { bl, 1 };

	Line line1(point1, point2, bordo, black);
	line1.draw();
	//line1.hide();

	if(watch[0].character < '2')
	{
		watch[0].hide();
		watch[0].character++;
		watch[0].draw();
	}
	Circle circle1({150, 150}, 50, bordo, black);
	circle1.draw();

	Character char1(point2, 'A', white, black);
	char1.draw();

	t1.attach(&dotsBlick, 0.5);
	t2.attach(&clocks, 1);
	t3.attach(&move, 0.025);

	//ODTUD
	button1.mode(PullUp); // Activate pull-up
	button1.fall(callback(button1_onpressed_cb)); // Attach ISR to handle button press event

	int idx = 0; // Just for printf below
	//POTUD

	while(1)
	{
		/*for (int i = 0; i < T; i++)
		{
			backl.Update(i);
			wait_ms(1);
		}*/

		//ODTUD
		if (button1_pressed) { // Set when button is pressed
			button1_pressed = false;
			pc.printf("Button pressed %d\r\n", idx++);
			//led1 = !led1;

			char1.hide();
			char1.character++;
			char1.draw();

			//backl.brightness -= 0.1f;

		}
		//POTUD
	}

	return 0;
}
