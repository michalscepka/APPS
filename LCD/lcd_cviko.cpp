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
#include "font22x36_msb.h"
#include <vector>

//#define WIDTH 8
//#define HEIGHT 8
#define WIDTH 22
#define HEIGHT 36

// Serial line for printf output
Serial pc(USBTX, USBRX);

// two dimensional array with fixed size font
//extern uint8_t font8x8[256][8];		//odkomenovat u 8x8

using namespace std;

DigitalOut bl(PTC3, 0);		// backlight

int offset = 22;
int dist = 10;
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
    GraphElement(){}

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
            //int radek_fontu = font8x8[character][y];
			int radek_fontu = font[character][y];
            for(int x = 0; x < WIDTH; x++)
            {
                //if(radek_fontu & (HEIGHT - WIDTH << x)) drawPixel(pos.x + x, pos.y + y);		//LSB
				if(radek_fontu & (HEIGHT - WIDTH << x)) drawPixel(pos.x - x + WIDTH, pos.y + y);    //MSB
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
                //int radek_fontu = font8x8[str[i]][y];
				int radek_fontu = font[str[i]][y];
                for(int x = 0; x < WIDTH; x++)
                {
                    if(horizontal)
                    {
						//if(radek_fontu & (HEIGHT - WIDTH << x)) drawPixel(pos.x + x + offs, pos.y + y);           //LSB
                        if(radek_fontu & (HEIGHT - WIDTH << x)) drawPixel(pos.x - x + WIDTH + offs, pos.y + y);    //MSB
                    }
                    else
                    {
                        //if(radek_fontu & (HEIGHT - WIDTH << x)) drawPixel(pos.x + x, pos.y + y + offs);			//LSB
						if(radek_fontu & (HEIGHT - WIDTH << x)) drawPixel(pos.x - x + WIDTH, pos.y + y + offs);    //MSB
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

class Triangle : public GraphElement
{
public:
    // Center of circle
    Point2D center;
    // Radius of circle
    int strana;

    RGB fg, bg;
    double vyska;
    Point2D point_c1, point_c2, point_c3, point_a1;
    Line *strana_a, *strana_b, *strana_c;

    Triangle( Point2D t_center, int t_strana, RGB t_fg, RGB t_bg ) :
        center( t_center ), strana( t_strana ), GraphElement( t_fg, t_bg )
    {
    	this->fg = t_fg;
    	this->bg = t_bg;

    	this->vyska = sqrt(pow(strana, 2) - pow(strana/ 2.0, 2));
    	this->point_c2 = { center.x, center.y + (vyska / 3) };
    	this->point_c1 = {center.x - (strana / 2), center.y + (vyska / 3)};
    	this->point_c3 = {center.x + (strana / 2), center.y + (vyska / 3)};
    	this->point_a1 = {center.x, center.y - ((vyska / 3) * 2)};

    	this->strana_a = new Line({point_c1.x, point_c1.y}, {point_a1.x, point_a1.y}, fg, bg);
    	this->strana_c = new Line({point_c1.x, point_c1.y}, {point_c3.x, point_c3.y}, fg, bg);
    	this->strana_b = new Line({point_c3.x, point_c3.y}, {point_a1.x, point_a1.y}, fg, bg);
    };

    void draw()
    {
    	strana_c->draw();
    	strana_a->draw();
    	strana_b->draw();
    }
};

//--- ZMACKNUTI BUTTONU ---
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
//--- ZMACKNUTI BUTTONU ---

Point2D point1 = {100, 100};
Point2D point2 = {120, 120};
Point2D point3 = {0, 0};
Point2D digital_h_p = {150, 50};
Point2D analog_h_p = {250, 150};
Point2D triangle_p = {100, 150};
RGB black = {0, 0, 0};
RGB white = {255, 255, 255};
RGB bordo = {128, 0, 32};
RGB cyan = {0, 255, 255};
RGB green = {0, 255, 0};
RGB blue = {0, 0, 255};
RGB red = {255, 0, 0};
RGB deeppink = {255, 20, 147};

//Text text1({ point3.x, point3.y + 50 }, "Hello!", cyan, black, true);

Circle circle1(analog_h_p, 52, cyan, black);
int seconds_counter = 0;
float timee = 0.0f;
float scale = 50;
float time_m = 0;
Line rucicka_s({ analog_h_p.x, analog_h_p.y }, { (analog_h_p.x + (int)(sin(timee) * scale)), (analog_h_p.y - (int)(cos(timee) * scale)) }, red, black);
Line rucicka_m({ analog_h_p.x, analog_h_p.y }, { (analog_h_p.x + (int)(sin(timee) * scale)), (analog_h_p.y - (int)(cos(timee) * scale)) }, white, black);

Character watch[6] =
{
	Character({digital_h_p.x, digital_h_p.y}, '0', white, black),
	Character({digital_h_p.x + offset, digital_h_p.y}, '0', white, black),
	Character({digital_h_p.x + offset * 3, digital_h_p.y}, '0', white, black),
	Character({digital_h_p.x + offset * 4, digital_h_p.y}, '0', white, black),
	Character({digital_h_p.x + offset * 6, digital_h_p.y}, '0', white, black),
	Character({digital_h_p.x + offset * 7, digital_h_p.y}, '0' - 1, white, black)
};

Character dots[2] =
{
	Character({digital_h_p.x + offset * 2, digital_h_p.y}, ':', white, black),
	Character({digital_h_p.x + offset * 5, digital_h_p.y}, ':', white, black)
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

bool add_time(int left, int right, char left_ch, char right_ch)
{
    if(watch[right].character >= right_ch)
    {
        watch[right].character = '0';

        if(watch[left].character >= left_ch)
        {
            watch[left].character = '0';
            return true;
        }
        else
        {
            watch[left].character++;
        }
    }
    else
    {
        watch[right].character++;
    }
    return false;
}

void digital_clocks()
{
    for(int i = 0; i < 6; i++)
    {
        watch[i].hide();
    }

    if(add_time(4, 5, '5', '9'))
    {
        if(add_time(2, 3, '5', '9'))
        {
            add_time(0, 1, '2', '9');
        }
    }

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

void analog_clocks()
{
    rucicka_s.hide();
    rucicka_m.hide();

    rucicka_s.pos2 = { (analog_h_p.x + (int)(sin(timee) * scale)), (analog_h_p.y - (int)(cos(timee) * scale)) };
    rucicka_s.draw();
    rucicka_m.pos2 = { (analog_h_p.x + (int)(sin(time_m) * scale)), (analog_h_p.y - (int)(cos(time_m) * scale)) };
    rucicka_m.draw();

    seconds_counter++;
    if(seconds_counter % 60 != 0)
    {
        timee += 0.105f;
    }
    else
    {
        timee = 0;
        seconds_counter = 0;
        time_m += 0.105f;
    }
}

DigitalIn but1(PTC9);
DigitalIn but2(PTC10);
DigitalIn but3(PTC11);
DigitalIn but4(PTC12);
Point2D start = {20, 220};
Circle circle(start, 10, white, black);
float scalee = 25;
float angle = 1.5;
Line cara(start, {(start.x + (int)(sin(angle) * scalee)), (start.y - (int)(cos(angle) * scalee))}, white, black);

vector<Circle> balls;
vector<float> angles;
vector<float> speeds;
float speed = 1;

void move()
{
	for(int i = 0; i < balls.size(); i++)
	{
		balls[i].hide();
		balls[i].center.x += ((int)(sin(angles[i]) * 10 * speeds[i]));
		balls[i].center.y -= ((int)(cos(angles[i]) * 10 * speeds[i]));
		balls[i].draw();
		if(balls[i].center.x >= 300)
		{
			balls[i].hide();
			balls.erase(balls.begin());
			angles.erase(angles.begin());
		}
		if(balls[i].center.y <= 10)
		{
			balls[i].hide();
			balls.erase(balls.begin());
			angles.erase(angles.begin());
		}
	}
}

int main()
{
	// Serial line initialization
	pc.baud(115200);
 	lcd_init();				// LCD initialization
	lcd_clear();			// LCD clear screen

	int l_color_red = 0xF800;
	int l_color_green = 0x07E0;
	int l_color_blue = 0x001F;
	int l_color_white = 0xFFFF;

	// simple animation display four color square using LCD_put_pixel function
	/*int l_limit = 200;
	for (int ofs = 0; ofs < 20; ofs++) // square offset in x and y axis
		for (int i = 0; i < l_limit; i++)
		{
			lcd_put_pixel(ofs + i, ofs + 0, l_color_red);
			//lcd_put_pixel(ofs + 0, ofs + i, l_color_green);
			lcd_put_pixel(ofs + i, ofs + l_limit, l_color_blue);
			lcd_put_pixel(ofs + l_limit, ofs + i, l_color_white);
		}*/

	Ticker t1, t2, t3, t4;

	//--- ZMACKNUTI BUTTONU ---
	button1.mode(PullUp); // Activate pull-up
	button1.fall(callback(button1_onpressed_cb)); // Attach ISR to handle button press event

	int idx = 0; // Just for printf below
	//--- ZMACKNUTI BUTTONU ---

	/*//Character char1(point3, '!', white, black);
    //char1.draw();
	//circle1.draw();
    //rucicka_s.draw();
    //rucicka_m.draw();
	//t1.attach(&dotsBlick, 0.5);
	//t2.attach(&clocks, 1);
	//t2.attach(&digital_clocks, 1);
	//t3.attach(&move, 0.5);
	//t4.attach(&analog_clocks, 1);
	//Triangle triangle1(triangle_p, 100, deeppink, black);
	//triangle1.draw();
	//text1.draw();*/

	circle.draw();
	cara.draw();
	t1.attach(&move, 0.2);

	while(1)
	{
		/*if (button1_pressed)	// Set when button is pressed
		{
			button1_pressed = false;
			pc.printf("Button pressed %d\r\n", idx++);
		}*/
		if(!but1)
		{
			angle -= 0.1;
			cara.hide();
			cara.pos2 = {(start.x + (int)(sin(angle) * scalee)), (start.y - (int)(cos(angle) * scalee))};
			cara.draw();
			wait_ms(150);
		}
		if(!but2)
		{
			angle += 0.1;
			cara.hide();
			cara.pos2 = {(start.x + (int)(sin(angle) * scalee)), (start.y - (int)(cos(angle) * scalee))};
			cara.draw();
			wait_ms(150);
		}
		if(!but3)
		{
			balls.push_back(Circle(cara.pos2, 5, white, black));
			angles.push_back(angle);
			speeds.push_back(speed);
			pc.printf("speed %f\r\n", speed);
			speed = 1;
			wait_ms(150);
		}
		if(!but4)
		{
			speed += 0.5;
			pc.printf("speed %f\r\n", speed);
			wait_ms(150);
		}
	}

	return 0;
}
