/*
 * File:   main.c
 * Author: joonas.arajarvi@gmail.com
 *
 * Created on 22 April 2024, 14:01
 * 7-segment display with the ATmega4809 curiosity nano
 * Looping numbers go down while on-board LED (PF5) is on and up when LED is off.
 * LED changes state with button press interrupt.
 */

#define F_CPU 3333333

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#define DELAY 400
#define SIZE_OF_ARRAY(x) (sizeof(x)/sizeof(x[0]))
#define PF6_CLEAR_INTERRUPT_FLAG PORTF.INTFLAGS &= PIN6_bm;

ISR(PORTF_PORT_vect)
{
    PORTF.OUTTGL = PIN5_bm;
    PF6_CLEAR_INTERRUPT_FLAG;
}

/* Make your own list for digits according to your pin setup from 5611AH datasheet
 * G 7, F 6, A 5, B 4, DP 3, C 2, D 1, E 0
 */
const uint8_t digit[] =
{
    0b01110111, // 0
    0b00010100, // 1
    0b10110011, // 2
    0b10110110, // 3
    0b11010100, // 4
    0b11100110, // 5
    0b11100111, // 6
    0b00110100, // 7
    0b11110111, // 8
    0b11110110, // 9
};

int main(void) {
    
    uint8_t value = 0; /* The number to be displayed */
    
    PORTF.DIRSET = PIN5_bm; /* Set pin PF5 (LED) as output */
    PORTF.OUTSET = PIN5_bm; /* Set pin PF5 (LED) to be off at start */
    PORTF.DIRCLR = PIN6_bm; /* Set pin PF6 (Button) as input */
    PORTF.PIN6CTRL = PORT_PULLUPEN_bm | PORT_ISC_FALLING_gc; /* Enable internal pull-up for PF6 */
    PORTC.DIR = 0xFF; /* Set PORTC as output for 7-segment display */
    sei(); /* Enable global interrupts */
    
    while (1) {
        /* Output a bit pattern from digit array */
        PORTC.OUT = digit[value];
        /* Increment (and loop) 'value' */
        if (PORTF.OUT & PIN5_bm) /* LED OFF -> numbers go up */
        {
            value = value >= (SIZE_OF_ARRAY(digit) -1) ? 0 : value + 1;
        } else /* LED ON -> numbers go down */
        { 
            value = value > 0 ? value - 1 : (SIZE_OF_ARRAY(digit) -1);
        }
        _delay_ms(DELAY);
    }
}