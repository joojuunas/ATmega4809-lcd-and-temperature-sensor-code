/*
 * File:   main.c
 * Author: dtek
 *
 * Created on 26 April 2024, 14:26
 */

// This doesn't work due to not reading the busy bit. 
// Timing problems try the other one with more detailed functions
// and more functionality which works for 4 bit interface.
#define F_CPU 3333333ul

#include "lcd.h"
#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
  lcdInit();
  while (1)
  {
    lcdString("heklo");
    _delay_ms(1000);
  }
}
