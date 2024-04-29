#ifndef LCD_H
#define LCD_H
#include <stdint.h>

void lcdDat(uint8_t);
void lcdCmd(uint8_t);
void lcdString(char *);
void lcdInit();
void lcdClear();

#endif	
