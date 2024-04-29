/*
 * File:   lcd.c
 * Author: dtek
 *
 * Created on 26 April 2024, 14:32
 */

#define F_CPU 3333333ul

#include "lcd.h" //uncomment in lcd.c
#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>

#define lcdPortCtrl VPORTC.OUT
#define lcdDirCtrl  VPORTC.DIR
#define lcdPortDat  VPORTA.OUT
#define lcdDirDat   VPORTA.DIR
#define EN PIN0_bm
#define RS PIN2_bm
#define D7 PIN0_bm
#define D6 PIN1_bm
#define D5 PIN2_bm
#define D4 PIN3_bm
#define D3 PIN4_bm
#define D2 PIN5_bm
#define D1 PIN6_bm
#define D0 PIN7_bm

static void lcdPinInit(){ lcdDirCtrl |= EN|RS; lcdDirDat |= D0|D1|D2|D3|D4|D5|D6|D7; }
static void lcdEN(){ lcdPortCtrl |= EN; _delay_us(2); lcdPortCtrl &= ~EN; }
static void lcdRS(bool tf){ if(tf) lcdPortCtrl |= RS; else lcdPortCtrl &= ~RS; }
static void lcdD0(bool tf){ if(tf) lcdPortDat |= D0; else lcdPortDat &= ~D0; }
static void lcdD1(bool tf){ if(tf) lcdPortDat |= D1; else lcdPortDat &= ~D1; }
static void lcdD2(bool tf){ if(tf) lcdPortDat |= D2; else lcdPortDat &= ~D2; }
static void lcdD3(bool tf){ if(tf) lcdPortDat |= D3; else lcdPortDat &= ~D3; }
static void lcdD4(bool tf){ if(tf) lcdPortDat |= D4; else lcdPortDat &= ~D4; }
static void lcdD5(bool tf){ if(tf) lcdPortDat |= D5; else lcdPortDat &= ~D5; }
static void lcdD6(bool tf){ if(tf) lcdPortDat |= D6; else lcdPortDat &= ~D6; }
static void lcdD7(bool tf){ if(tf) lcdPortDat |= D7; else lcdPortDat &= ~D7; }

/* Pins are like this and then check the value of the corresbonding bit with bitwise and &*/
static void lcdD(uint8_t v){  lcdD0(v&1); lcdD1(v&2); lcdD2(v&4); 
    lcdD3(v&8); lcdD4(v&16); lcdD5(v&32); lcdD6(v&64); lcdD7(v&128); lcdEN(); }

void lcdCmd(uint8_t v){ lcdD(v); lcdRS(0); }
void lcdDat(uint8_t v){ lcdRS(1); lcdCmd(v); }
void lcdString(char *str){ for( ; *str; lcdDat((uint8_t)*str++) ); _delay_us(5); }
void lcdInit(void){
    lcdPinInit();

    //HD44780 datasheet
    _delay_ms(20);
    lcdD(0b00110000); //8bit mode
    _delay_ms(5);
    lcdD(0b00110000); //8bit mode
    _delay_us(100);
    lcdD(0b00110000); //Last 8bit mode init
    //0x32 = 4bit mode (3=8bit, 2=4bit)
    //0x28 = 4bit, 2line
    //0x08 = display off
    //0x01 = display clear
    //0x06 = entry mode, increment
    //0x0f = display on, cursor on, blink on
    //0b00111000 = 8bit, 2line, 5x8 dots
    static const uint8_t initv[]={0b00111000,0x08,0x01,0x06,0x0f};
    for(uint8_t i = 0; initv[i]; i++) lcdCmd(initv[i]);
}
void lcdClear(){
  lcdCmd(0x01);     //Clear display screen

  _delay_ms(5); //not sure what is needed, but something is needed
}
