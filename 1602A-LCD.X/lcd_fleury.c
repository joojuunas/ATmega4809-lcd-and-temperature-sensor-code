/*
 * LCD_single_Curiosity.c
 *
 * Created: 20-Jan-20 12:29:28
 * Author : David Prentice
 * Modified: joonas.arajarvi@gmail.com on 28.4.2024 11:45
 * 
 * Implements functions for controlling the 1602A LCD screen
 * with the ATmega4809 curiosity nano.
 */ 
#define F_CPU 3333333ul

#define LCD_Dir  VPORTA_DIR
#define LCD_Port VPORTA_OUT
#define RS_bm    (1<<4)
#define RW_bm    (1<<5)
#define EN_bm    (1<<6)
#define BL_bm    (1<<7)
#define D4_bp    0       //D4-D7 are on PC0-PC3

#include <stdint.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include <avr/io.h>
#include "lcd_fleury.h"

static unsigned char _base_y[4] = {0x80,0xc0};
unsigned char _lcd_x, _lcd_y, _lcd_maxx;

static void wr_lcd_nybble(unsigned char c)
{
    c <<= D4_bp;                  //shift data to correct bits
    LCD_Port &= (RS_bm|BL_bm);    //keep RS,BL. RW=0, EN=0, data=0
    LCD_Port |= c | EN_bm;        //EN=1, data=c
    _delay_us(1);
    LCD_Port &= ~EN_bm;           //EN=0 falling edge latches data
}

void lcd_command(unsigned char cmd)
{
    LCD_Port &= ~RS_bm;            // RS=0
    wr_lcd_nybble(cmd >> 4);       //hi
    wr_lcd_nybble(cmd & 0x0F);     //lo
    _delay_us(50);                 // regular data or cmd
    if (cmd <= 2) _delay_ms(2);    // CLS and HOME
}

void lcd_data(unsigned char data)
{
    LCD_Port |= RS_bm;             // RS=1
    wr_lcd_nybble(data >> 4);       //hi
    wr_lcd_nybble(data & 0x0F);     //lo
    _delay_us(50);                      // regular data or cmd
}
// set the LCD display position  x=0..39 y=0..3
void lcd_gotoxy(unsigned char x, unsigned char y)
{
    lcd_command(0x80 | (_base_y[y] + x));   //.kbv now use +
    _lcd_x=x;
    _lcd_y=y;
}
// clear the LCD and set cursor on home position
void lcd_clrscr(void)
{
    lcd_command(0x01);
    _lcd_x = _lcd_y = 0;
}
// set cursor on home position
void lcd_home(void)
{
    lcd_command(0x02);
    _lcd_x = _lcd_y = 0;
}
void lcd_putc(char c)
{
    //if (_lcd_x>=_lcd_maxx || c == '\n')
    if (c == '\n')
    {
        lcd_gotoxy(0,++_lcd_y);
    }
    if (c != '\n') {
        ++_lcd_x;
        lcd_data(c);
    }
}
// write the string str located in SRAM to the LCD
void lcd_puts(const char *str)
{
    while (*str) lcd_putc(*str++);
}
// write the string str located in FLASH to the LCD
void lcd_puts_p(const char *progmem_s)
{
    char c;
    while ((c = pgm_read_byte(progmem_s++)) != 0) {
        lcd_putc(c);
    }
}
// initialize the LCD controller
void lcd_init(unsigned char cmd)
{
    unsigned char i;
    unsigned char init_sequenz[] = { 0x28, 0x0C, 0x06, 0x01 };
    _lcd_maxx = LCD_DISP_LENGTH;
    _base_y[2] = _base_y[0] + _lcd_maxx;
    _base_y[3] = _base_y[1] + _lcd_maxx;
    LCD_Dir = 0xFF;
    _delay_ms(30);               // 30 ms Delay each power-up
    LCD_Port = 0;                //RS=0, RW=0, EN=0, BL=0
    wr_lcd_nybble(0x03);         //follow datasheet conservatively.
    _delay_ms(5);
    wr_lcd_nybble(0x03);
    _delay_us(100);
    wr_lcd_nybble(0x03);
    _delay_us(50);
    wr_lcd_nybble(0x02);
    _delay_us(50);
    for (i = 0; i < sizeof(init_sequenz); i++) {
        lcd_command(init_sequenz[i]);
    }
    lcd_command(cmd);
}

void lcd_led(unsigned char on)
{
    if (on) LCD_Port |= BL_bm;
    else LCD_Port &= ~BL_bm;
}