#include "pico/stdlib.h"
#include "lcd_display.hpp"

LCDdisplay myLCD(2,3,4,5,14,15,16,2); // DB4, DB5, DB6, DB7, RS, E, character_width, no_of_lines

int main() {
	myLCD.init();
	while (true) {
		myLCD.clear();
		myLCD.cursor_on();
		myLCD.print("HELLO   ");
		sleep_ms(3000);
		myLCD.clear() ;
		myLCD.cursor_off();
		myLCD.print_wrapped("0123456789012345BYE");
		sleep_ms(3000);
	};
	return 0;
};
