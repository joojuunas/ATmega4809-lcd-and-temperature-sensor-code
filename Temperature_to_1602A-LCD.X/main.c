/*
 * main.c
 *
 * Created: 20-Jan-20 12:29:28
 * Author : David Prentice
 * Modified: joonas.arajarvi@gmail.com on 28.4.2024 11:45
 * 
 * Implements functions for controlling the 1602A LCD screen
 * and checks for ATmega4809 curiosity nano or other AVRs for timer.
 * Also testing code for the LCD most of which is commented out. 
 * Configuring and reading the on-chip temperature sensor. 
 */ 

#define F_CPU 3333333ul

#define _BV(bit) (1 << (bit))
#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "lcd_functions.h"

#define Wait(x) _delay_ms(x)
//#define LCDInit()  lcd_init(0)
#define LCDInit()  lcd_init(LCD_DISP_ON)
#define LCDClear() lcd_command(0x01)
#define LCDReturn() lcd_command(0x02)
#define LCDEntry(direction,shift) lcd_command(0x04+direction*2+shift)
#define LCDOnOff(display,cursor,blink) lcd_command(0x08+display*4+cursor*2+blink)
#define LCDShift(cursor,right) lcd_command(0x10+(!cursor)*8+right*4)
#define LCDDefChar(c,row) lcd_command(0x40+c*8+(row))
#define LCDPrintAt(x,y) lcd_command(0x80+x+(y*0x40))
#define LCD_String(s)    LCD_String_P(PSTR(s))
#define LCD_String_Slow(s,f)    LCD_String_Slow_P(PSTR(s),f)
#define FLASHREAD(ads)  pgm_read_byte(ads)
#define GFLASH  const

/* Special characters
unsigned char defs[][8] = {
    {0x4, 0xe, 0xe, 0xe, 0x1f, 0x0, 0x4},
    {0x2, 0x3, 0x2, 0xe, 0x1e, 0xc, 0x0},
    {0x0, 0xe, 0x15, 0x17, 0x11, 0xe, 0x0},
    {0x0, 0xa, 0x1f, 0x1f, 0xe, 0x4, 0x0},
    {0x0, 0xc, 0x1d, 0xf, 0xf, 0x6, 0x0},
    {0x0, 0x1, 0x3, 0x16, 0x1c, 0x8, 0x0},
    {0x0, 0x1b, 0xe, 0x4, 0xe, 0x1b, 0x0},
    {0x1, 0x1, 0x5, 0x9, 0x1f, 0x8, 0x4}
};
*/

void LCDPutc(char c)
{
    lcd_data(c);
}

void LCD_CustomChar(char n, unsigned char dat[])
{
    unsigned char i;
    LCDDefChar(n, 0);
    for (i = 0; i < 8; i++) LCDPutc(dat[i]);
}

void LCD_String_S(char *s)
{
    char c;
    while ((c = *s++) != 0)
        LCDPutc(c);
}

void LCD_String_P(char GFLASH * s)
{
    char c;
    while ((c = FLASHREAD(s++)) != 0)
        LCDPutc(c);
}

void LCDNum(unsigned int n)
{
    char buf[14];
#if   defined(SDCC)
    _ltoa(n, buf, 10);
#elif defined(__GNUC__) || defined(__CROSSWORKS)
    ltoa(n, buf, 10);
#else
    ltoa(n, buf);
#endif
    LCD_String_S(buf);
}

#if defined(__AVR_ATmega4809__)  //F_CPU div8
#define START_TIMER()   { \
        TCA0.SINGLE.CTRLA=0;\
        TCA0.SINGLE.CTRLB=0;\
        TCA0.SINGLE.CNT=0x0000;\
        TCA0.SINGLE.PER=0xFFFF;\
        TCA0.SINGLE.CTRLA=7;\
    }
#define READ_TIMER(val) { TCA0.SINGLE.CTRLA = 0; val = TCA0.SINGLE.CNT;}
#define ADJUST_TIME(us) { us = ((unsigned long) us * 8000) / (F_CPU / 1000); }
#elif defined(AVR)
#define START_TIMER()   { TCCR1A = 0; TCNT1H = 0; TCNT1L = 0; TCCR1B = 2;}
#define READ_TIMER(val) { TCCR1B = 0; val = TCNT1L; val += (unsigned int)TCNT1H << 8;}
#define ADJUST_TIME(us) { us = ((unsigned long) us * 8000) / (F_CPU / 1000); }
#else
#define START_TIMER()   { }
#define READ_TIMER(val) { }
#define ADJUST_TIME(us) { us = 0; }
#endif
    // tells how long function f took in microseconds.
#define FUNCTIME(msg, f) {init_us(); f; us = calc_us(); \
        LCDClear(); LCD_String(msg); LCDPrintAt(0, 1); LCDNum(us); LCD_String(" us"); Wait(2000);}

void init_us(void)
{
    Wait(30);                   // make sure any putchar() is finished
    START_TIMER();
}

unsigned int calc_us(void)
{
    unsigned int us;
    READ_TIMER(us);             // val = current timer ticks
    ADJUST_TIME(us);            // convert ticks to us
    return us;
}

void LCD_String_Slow_P(char GFLASH * s, char backwards)
{
    char c, len;
    for (len = 0; (c = FLASHREAD(s)) != 0; s++)
        len++;
    if (!backwards)
        s -= len;
    while (len--) {
        c = backwards ? FLASHREAD(--s) : FLASHREAD(s++);
        LCDPutc(c);
        Wait(200);
    }
}


int main(void)
{
    /*
    unsigned char j, count;
    unsigned int us;
    */
    
    int8_t sigrow_offset = SIGROW.TEMPSENSE1; // Read signed value from signature row
    uint8_t sigrow_gain = SIGROW.TEMPSENSE0; // Read unsigned value from signature row
    uint16_t adc_reading =  0; // ADC conversion result with 1.1 V internal reference
    uint32_t temp = 0;
    uint8_t time;
    
#if defined(__AVR_ATmega4809__) && F_CPU >= 16000000
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLA, CLKCTRL_CLKSEL_OSC20M_gc);
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0); //Peripheral Clock x1
#endif
#if defined(__AVR_ATmega32__)
    MCUCSR = (1 << JTD);
    MCUCSR = (1 << JTD);
#endif
    LCDInit();
    
    /* custom characters
    for (j = 0; j < 8; j++)
        LCD_CustomChar(j, defs[j]);
    */
    LCDOnOff(1, 0, 0);      // no cursor no blink
    LCDEntry(1, 0);         // INC cursor
    LCDPrintAt(0, 0);
    
    // Temperature sensor init
    VREF.CTRLA = 0b00010000; /* VREF to 1.1 V */
    ADC0.CTRLC = 0b00000000; /* Select VREF by writing REFSEL bits to 0x0 */
    ADC0.MUXPOS = 0x1E; /* Select ADC temperature sensor channel */
    ADC0.CTRLD = 0b01000000; /* INITDLY init delay*/
    ADC0.SAMPCTRL = 0x2; /* SAMPLEN sample length*/
    ADC0.CTRLC = 0b01000000; /* SAMPCAP sample cap */
    ADC0.CTRLA = 0x01; /* ENABLE ADC in 10 bit mode */
    
    while (1) {
        // testing temperature
        ADC0.COMMAND = 0x01; /* start conversion */
        Wait(1000);
        LCDPrintAt(12, 0);
        LCD_String_Slow("   ", 0);
        if (~(ADC0.COMMAND & (1 << 0)))
        {
            adc_reading = ADC0.RES;
            temp = adc_reading - sigrow_offset;
            temp *= sigrow_gain; // Result might oveflow 16 bit variable (10bit+8bit)
            temp += 0x80; // Add 1/2 to get correct rounding on division below
            temp >>= 8; // Divide result to get Kelvin
            uint16_t kelvin = temp; 
            uint16_t celcius = temp - 273; // Convert to Celcius
            LCDPrintAt(12, 1);
            LCDNum(celcius);
            LCD_String_Slow(" C", 0);
            LCDPrintAt(0, 1);
            LCDNum(kelvin);
            LCD_String_Slow(" K",0);
            LCDPrintAt(0, 0);
            
        } else {
            LCDPrintAt(0, 0);
            LCD_String_Slow("not working", 0);
            Wait(30000);
        }
        LCD_String_Slow("Updates in: ", 0);
        LCDPrintAt(14, 0);
        LCD_String_Slow(" s", 0);
        for (time = 30; time >= 0 && time < 31; )
        {
            if (time > 9)
            {
                LCDPrintAt(12, 0);
            } else {
                LCDPrintAt(13, 0);
            }
            LCDNum(time);
            Wait(1000);
            time--;
            if (time == 9)
            {
                LCDPrintAt(12, 0);
                LCD_String_Slow(" ", 0);
                LCDPrintAt(13, 0);
            } else if (time == 0)
            {
                LCDPrintAt(12, 0);
                LCD_String_Slow("NOW ", 0);
                break;
            }
        }
        
        /*for (time = 0; time < 100; time++) 
        {
            LCDNum(time);
            LCD_String_Slow(" s", 0);
            LCDPrintAt(0, 0);
            Wait(100);
        }
        LCDClear();
        */
        
        /* testing variable so these are commented out
        LCD_String_Slow("HD44780 display ", 0);
        LCDEntry(0, 0);         // DEC cursor
        LCDPrintAt(15, 1);
        LCD_String_Slow("has 16x2 chars  ", 1);
        Wait(1000);
        for (count = 16; count--; Wait(200))
            LCDShift(0, 0);     //LEFT
        Wait(1000);
        for (count = 16; count--; Wait(200))
            LCDShift(0, 1);     //RIGHT
        Wait(1000);
         */
/*
        // 1290 bytes without FUNCTIME sequences on t2313
        FUNCTIME("Wait(10)", {Wait(10);});
        FUNCTIME("11 Return", {for (count = 11; count--;) LCDReturn();});
        FUNCTIME("11 Clear", {for (count = 11; count--;) LCDClear();});
        FUNCTIME("11 putc(x)", {for (count = 11; count--;) LCDPutc('x');});

        FUNCTIME("(\"10 letters\")", {LCD_String("10 letters");});
        LCDClear();
        LCDOnOff(1, 1, 1);      // cursor blink
        LCDEntry(1, 0);         // INC cursor
        LCDPrintAt(0, 0);
        LCD_String_Slow("Use blink cursor ", 0);
        LCDOnOff(1, 1, 0);      // cursor underline
        LCDEntry(0, 0);         // DEC cursor
        LCDPrintAt(15, 1);
        LCD_String_Slow("or u/line cursor", 1);
        Wait(1000);
        LCDPrintAt(0, 0);
        LCDOnOff(1, 0, 0);      // cursor off
        LCDEntry(1, 0);         // INC cursor
        LCD_String_Slow("Print at any pos even off screen", 0);
        LCDEntry(0, 0);         // DEC cursor
        LCDPrintAt(15, 1);
        LCD_String_Slow("\1", 1);
        LCD_String_Slow("8 User defined ", 1);
        for (count = 16; count--; Wait(200))
            LCDPrintAt(count, j);       //posn
        for (count = 16; count--; Wait(200))
            LCDShift(0, 0);     //LEFT
        Wait(1000);
        for (count = 16; count--; Wait(200))
            LCDShift(0, 1);     //RIGHT
        Wait(1000);
        LCDClear();             // resets entry mode and shift
        LCD_String_Slow("Show chars 0-255", 0);
        LCDPrintAt(0, 1);
        LCD_String_Slow("0-7 repeats 8-15", 0);
        Wait(1000);
        for (j = 0; j < 8; j++) {
            char i, row;
            for (row = 0; row < 2; row++) {
                LCDPrintAt(0, row);
                for (i = 0; i < 40; i++) {
                    LCDPutc((j * 32) + (row * 16) + i);
                }
            }
            Wait(2000);
        }
 */
    }
}