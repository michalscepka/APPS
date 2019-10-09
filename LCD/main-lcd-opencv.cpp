// **************************************************************************
//
//               Demo program for labs
//
// Subject:      Computer Architectures and Parallel systems
// Author:       Petr Olivka, petr.olivka@vsb.cz, 09/2019
// Organization: Department of Computer Science, FEECS,
//               VSB-Technical University of Ostrava, CZ
//
// File:         OpenCV simulator of LCD
//
// **************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include "font8x8.cpp"

#define LCD_WIDTH       320
#define LCD_HEIGHT      240
#define LCD_NAME        "Virtual LCD"

#define WIDTH 8
#define HEIGHT 8

using namespace std;

int offset = 8;
int dist = 10;

// LCD Simulator

// Virtual LCD
cv::Mat g_canvas( cv::Size( LCD_WIDTH, LCD_HEIGHT ), CV_8UC3 );

// Put color pixel on LCD (canvas)
void lcd_put_pixel( int t_x, int t_y, int t_rgb_565 )
{
    // Transform the color from a LCD form into the OpenCV form. 
    cv::Vec3b l_rgb_888( 
            (  t_rgb_565         & 0x1F ) << 3, 
            (( t_rgb_565 >> 5 )  & 0x3F ) << 2, 
            (( t_rgb_565 >> 11 ) & 0x1F ) << 3
            );
    g_canvas.at<cv::Vec3b>( t_y, t_x ) = l_rgb_888; // put pixel
}

// Clear LCD
void lcd_clear()
{
    cv::Vec3b l_black( 0, 0, 0 );
    g_canvas.setTo( l_black );
}

// LCD Initialization 
void lcd_init()
{
    cv::namedWindow( LCD_NAME, 0 );
    lcd_clear();
    cv::waitKey( 1 );
}

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
        int offset = 0;
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
                            drawPixel(pos.x + x + offset, pos.y + y);
                    }
                    else
                    {
                        if(radek_fontu & (1 << x))
                            drawPixel(pos.x + x, pos.y + y + offset);
                    }
                }
            }
            offset += 8;
        }
    }

    void move()
    {
        hide();
        pos.x += dist;
        draw();
    }
};

void Move(Text text)
{
    text.hide();
    text.pos.x += 100;
    text.draw();
}

Point2D point1 = {10, 10};
Point2D point2 = {120, 120};
Point2D point3 = {100, 100};
Point2D point4 = {150, 50};
Point2D point5 = {150, 150};
RGB black = {0, 0, 0};
RGB white = {255, 255, 255};
RGB bordo = {128, 0, 32};
RGB cyan = {0, 255, 255};
RGB green = {0, 255, 0};
RGB blue = {0, 0, 255};
RGB red = {255, 0, 0};

Circle circle1(point5, 52, cyan, black);
int seconds_counter = 0;
float timee = 0.0f;
float scale = 50;
float time_m = 0;
Line rucicka_s({ point5.x, point5.y }, { (point5.x + (int)(sin(timee) * scale)), (point5.y - (int)(cos(timee) * scale)) }, red, black);
Line rucicka_m({ point5.x, point5.y }, { (point5.x + (int)(sin(timee) * scale)), (point5.y - (int)(cos(timee) * scale)) }, white, black);

Character watch[6] =
{ 
    Character({point4.x, point4.y}, '0', white, black),
    Character({point4.x + offset, point4.y}, '0', white, black),
    Character({point4.x + offset * 3, point4.y}, '0', white, black),
    Character({point4.x + offset * 4, point4.y}, '0', white, black),
    Character({point4.x + offset * 6, point4.y}, '0', white, black),
    Character({point4.x + offset * 7, point4.y}, '0' - 1, white, black)
};

Character dots[2] =
{
    Character({point4.x + offset * 2, point4.y}, ':', white, black),
    Character({point4.x + offset * 5, point4.y}, ':', white, black)
};

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
    
    rucicka_s.pos2 = { (point5.x + (int)(sin(timee) * scale)), (point5.y - (int)(cos(timee) * scale)) };
    rucicka_s.draw();
    rucicka_m.pos2 = { (point5.x + (int)(sin(time_m) * scale)), (point5.y - (int)(cos(time_m) * scale)) };
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

int main()
{
    lcd_init();                     // LCD initialization

    lcd_clear();                    // LCD clear screen

    /*int l_color_red = 0xF800;
    int l_color_green = 0x07E0;
    int l_color_blue = 0x001F;
    int l_color_white = 0xFFFF;

    // simple animation display four color square using LCD_put_pixel function
    int l_limit = 200;
    for ( int ofs = 0; ofs < 20; ofs++ ) // square offset in x and y axis
        for ( int i = 0; i < l_limit; i++ )
        {
            lcd_put_pixel(ofs + i, ofs + 0, l_color_red);
            lcd_put_pixel(ofs + 0, ofs + i, l_color_green);
            lcd_put_pixel(ofs + i, ofs + l_limit, l_color_blue);
            lcd_put_pixel(ofs + l_limit, ofs + i, l_color_white);
        }*/

    Character char1(point3, 'A', white, black);
    char1.draw();

    Text text1(point1, "Kokos", cyan, black, true);
    text1.draw();

    /*for(int i = 0; i < 10; i++)
    {
        text1.move();
        cv::imshow( LCD_NAME, g_canvas );
        cv::waitKey( 0 );
    }*/

    for (int i = 0; i < 2; i++)
    {
        dots[i].draw();
    }
    
    circle1.draw();
    rucicka_s.draw();
    rucicka_m.draw();

    while(true)
    {
        digital_clocks();
        analog_clocks();

        cv::imshow( LCD_NAME, g_canvas );
        cv::waitKey( 0 );
    }

    cv::imshow( LCD_NAME, g_canvas );   // refresh content of "LCD"
    cv::waitKey( 0 );                   // wait for key 
}













