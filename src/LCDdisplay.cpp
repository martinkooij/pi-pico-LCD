#include <stdlib.h>
#include "hardware/gpio.h"
#include "hardware/timer.h"
//typedef unsigned int uint;
//typedef unsigned int uint32_t;


// Pin positions in LCDpins array
#define RS 4
#define E 5
// Pin values
#define HIGH 1
#define LOW 0
// LCD pin RS meaning
#define COMMAND 0
#define DATA 1

// ########### WIRING SCHEMA ###########
//# DB0-DB3 : open or grounded, basically reads as 0 for LCD display
//# DB4-DB7 : connected on gpiopins (LCDpins[0],...,LCDpins[3])
//# RS      : connected on LCDpins[4]
//# R/W     : grounded (never read from LCD) 
//# E       : connected on LCDpins[5]
//##########################################

/*
void gpio_put_masked(uint32_t mask, uint32_t setting) {
	printf("gpio put mask = %X, setting = %X\n",mask,setting);
};

void gpio_clr_mask(uint32_t mask) {
	printf("gpio clr mask = %X\n", mask);
};

void gpio_init_mask(uint32_t mask) {
	printf("gpio init mask = %X\n", mask);
};

void gpio_put(int pin, int value) {
	printf("gpio put pin %d to value %d\n", pin,value);
};

void gpio_set_dir_out_masked(uint32_t mask) {
    printf("gpio set output pins mask = %X\n", mask);
};

void sleep_ms(int x) {
    printf("gpio sleep for %d millisecond\n", x);
};
*/

extern "C" void sleep_ms(int x);

class LCDdisplay {
	private:
	uint LCDpins[6];
	uint32_t LCDmask_c = 0 ; // with clock
	uint32_t LCDmask = 0 ; //without clock
	uint no_chars;
	uint no_lines;
	
	uint32_t pin_values_to_mask(uint raw_bits[],int length) {   // Array of Bit 7, Bit 6, Bit 5, Bit 4, RS(, clock)
		uint32_t result = 0 ;
		uint pinArray[32] ;
		for (int i = 0 ; i < 32; i++) {pinArray[i] = 0;}
		for (int i = 0 ; i < length ; i++) {pinArray[LCDpins[i]]= raw_bits[i];}
		for (int i = 0 ; i < 32; i++) {
			result = result << 1 ;
			result += pinArray[31-i] ;
		}
		return result ;
	};
	
	void uint_into_8bits(uint raw_bits[], uint one_byte) {  	
		for (int i = 0 ; i < 8 ; i++ ) {
			raw_bits[7-i] = one_byte % 2 ;
			one_byte = one_byte >> 1 ;
		}
	};
	
	void send_raw_data_one_cycle(uint raw_bits[]) { // Array of Bit 7, Bit 6, Bit 5, Bit 4, RS
		uint32_t bit_value = pin_values_to_mask(raw_bits,5) ;
		gpio_put_masked(LCDmask, bit_value) ;
		gpio_put(LCDpins[E], HIGH) ;
		sleep_ms(5) ;
		gpio_put(LCDpins[E], LOW) ; // gpio values on other pins are pushed at the HIGH->LOW change of the clock. 
		sleep_ms(5) ;
	};
		
	void send_full_byte(uint rs, uint databits[]) { // RS + array of Bit7, ... , Bit0
		// send upper nibble (MSN)
		uint rawbits[5];
		rawbits[4] = rs ;
		for (int i = 0 ; i<4 ; i++) { rawbits[i]=databits[i];}
		send_raw_data_one_cycle(rawbits);
		// send lower nibble (LSN)
		for (int i = 0; i<4 ; i++) { rawbits[i]=databits[i+4];}
		send_raw_data_one_cycle(rawbits);
	};
	
	
	
	public:
	
	LCDdisplay(uint bit4_pin, uint bit5_pin, uint bit6_pin, uint bit7_pin, uint rs_pin, uint e_pin, uint width, uint depth) { // constructor
		LCDpins[0] = bit7_pin;
		LCDpins[1] = bit6_pin;
		LCDpins[2] = bit5_pin;
		LCDpins[3] = bit4_pin;
		LCDpins[4] = rs_pin;
		LCDpins[5] = e_pin;
		no_chars = width;
		no_lines = depth;
	};
	
	void clear() {
		uint clear_display[8] = {0,0,0,0,0,0,0,1};
		send_full_byte(COMMAND, clear_display);
		sleep_ms(10) ; // extra sleep due to equipment time needed to clear the display
	};
	
		
	void cursor_off() {
		uint no_cursor[8] = {0,0,0,0,1,1,0,0};
		send_full_byte(COMMAND, no_cursor);
	};

	void cursor_on() {
		uint blink_cursor[8] = {0,0,0,0,1,1,1,1};
		send_full_byte(COMMAND, blink_cursor);
	};	
	
	void init() { // initialize the LCD
	
		uint all_ones[6] = {1,1,1,1,1,1};
		uint set_function_8[5] = {0,0,1,1,0};
		uint set_function_4a[5] = {0,0,1,0,0};
		
		uint set_function_4[8] = {0,0,1,0,0,0,0,0};
		uint cursor_set[8] = {0,0,0,0,0,1,1,0};
		uint display_prop_set[8] = {0,0,0,0,1,1,0,0};
		
		//set mask, initialize masked pins and set to LOW 
		LCDmask_c = pin_values_to_mask(all_ones,6);
		LCDmask = pin_values_to_mask(all_ones,5);
		gpio_init_mask(LCDmask_c);   			// init all LCDpins
		gpio_set_dir_out_masked(LCDmask_c);	// Set as output all LCDpins
		gpio_clr_mask(LCDmask_c);				// LOW on all LCD pins 
		
		//set LCD to 4-bit mode and 1 or 2 lines
		//by sending a series of Set Function commands to secure the state and set to 4 bits
		if (no_lines == 2) { set_function_4[4] = 1; };
		send_raw_data_one_cycle(set_function_8);
		send_raw_data_one_cycle(set_function_8);
		send_raw_data_one_cycle(set_function_8);
		send_raw_data_one_cycle(set_function_4a);
		
		//getting ready
		send_full_byte(COMMAND, set_function_4);
		send_full_byte(COMMAND, cursor_set);
		send_full_byte(COMMAND, display_prop_set);
		clear() ;
	};

	void goto_pos(uint pos, uint line) {
		uint eight_bits[8];
		switch (no_lines) {
			case 2: 
				pos = 64*line+ pos + 0b10000000; 
				break ;
			case 4: 	if (line == 0 || line == 2) {
					pos = 64*(line/2) + pos + 0b10000000;
				} else {
					pos = 64*((line-1)/2) + 20 + pos + 0b10000000;
				};
				break;
			default:
				pos = pos ;
		};
		uint_into_8bits(eight_bits,pos);
		send_full_byte(COMMAND,eight_bits);
	};
	
	void print(const char * str) {
		uint eight_bits[8];
		int i = 0 ;
		while (str[i] != 0) {
			uint_into_8bits(eight_bits,(uint)(str[i]));
			send_full_byte(DATA, eight_bits);
			++i;
		}
	};
		
	void print_wrapped(const char * str) {
		uint eight_bits[8];
		int i = 0 ;
		
		goto_pos(0,0);

		while (str[i] != 0) {
			uint_into_8bits(eight_bits,(uint)(str[i]));
			send_full_byte(DATA, eight_bits);
			++i;
			if (i%no_chars == 0) { goto_pos(0,i/no_chars); }
		}
	};
				
};
