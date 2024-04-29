/*
 * File:   main.c
 * Author: joonas.arajarvi@gmail.com
 *
 * Created on 21 April 2024, 19:43
 * Blinking the led and stopping it with a button press 
 * with the ATmega4809 curiosity nano
 */

#define F_CPU   3333333

#include <avr/io.h>
#include <util/delay.h>

#define STEP_DELAY 10
#define THRESHOLD 100   /* 100 steps x 10 ms/step = 1000 ms */
#define LONG_DELAY 1000
#define SHORT_DELAY 200
#define NUMBER_OF_BLINKS 2

inline static void LED_BLINK(uint32_t time_ms);

int main(void) {
    
    uint8_t counter = 0;
    
    PORTF.DIRSET = PIN5_bm; /* Set pin PF5 (LED) as output */
    PORTF.OUTSET = PIN5_bm; /* Set pin PF5 (LED) to be off at start */
    PORTF.DIRCLR = PIN6_bm; /* Set pin PF6 (Button) as input */
    PORTF.PIN6CTRL = PORT_PULLUPEN_bm; /* Enable internal pull-up for PF6 */
    
    while (1) {
        
        if (~PORTF.IN & PIN6_bm) /* check if PF6 (Button) is not pressed (pulled to GND) */
        {
            while (~PORTF.IN & PIN6_bm) /* check if PF6 (Button) is pressed (pulled to VDD) */
            {
                _delay_ms(STEP_DELAY);
                counter++;
                if (counter >= THRESHOLD)
                {
                    LED_BLINK(LONG_DELAY);
                    while (~PORTF.IN & PIN6_bm) /* check if PF6 (Button) is pressed (pulled to VDD) */
                    {
                        ;
                    }
                    break;
                }
            }
            if (counter < THRESHOLD)
            {
                LED_BLINK(SHORT_DELAY);
            }
            counter = 0;
        }
    }
}

inline static void LED_BLINK(uint32_t time_ms)
{
    for (uint8_t i = 0; i < NUMBER_OF_BLINKS; i++)
    {
        PORTF.OUTTGL = PIN5_bm; /* toggle PF5 (LED) */
        _delay_ms(time_ms);
        PORTF.OUTTGL = PIN5_bm; /* toggle PF5 (LED) */
        _delay_ms(time_ms);
    }
}