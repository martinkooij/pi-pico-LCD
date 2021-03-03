#include "pico/stdlib.h"

#include "lcd_display.hpp"

#define BL_PIN 16

LCDdisplay myLCD(2,3,4,5,14,15,BL_PIN,16,2); // DB4, DB5, DB6, DB7, RS, E, BL_PIN, character_width, no_of_lines

int main() {
	myLCD.init();
	int bright = 60 ;
	while (true) {
		myLCD.set_backlight(bright);
		bright = (bright + 20) ;
		if (bright > 100){ bright = 0;};
		myLCD.clear();
		myLCD.cursor_on();
		myLCD.print("HELLO   ");
		sleep_ms(2500);
		myLCD.clear() ;
		myLCD.cursor_off();
		myLCD.print_wrapped("0123456789012345BYE");
		sleep_ms(2500);
	};
	return 0;
};
