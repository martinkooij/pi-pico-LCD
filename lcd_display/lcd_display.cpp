#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/time.h"
#include "lcd_display.hpp"

// Pin positions in LCDpins array
#define RS 4
#define E 5
// Pin values
#define HIGH 1
#define LOW 0
// LCD pin RS meaning
#define COMMAND 0
#define DATA 1
	
	uint32_t LCDdisplay::pin_values_to_mask(uint raw_bits[],int length) {   // Array of Bit 7, Bit 6, Bit 5, Bit 4, RS(, clock)
		uint32_t result = 0 ;
		uint pinArray[32] ;
		for (int i = 0 ; i < 32; i++) {pinArray[i] = 0;}
		for (int i = 0 ; i < length ; i++) {pinArray[this->LCDpins[i]]= raw_bits[i];}
		for (int i = 0 ; i < 32; i++) {
			result = result << 1 ;
			result += pinArray[31-i] ;
		}
		return result ;
	};
	
	void LCDdisplay::uint_into_8bits(uint raw_bits[], uint one_byte) {  	
		for (int i = 0 ; i < 8 ; i++ ) {
			raw_bits[7-i] = one_byte % 2 ;
			one_byte = one_byte >> 1 ;
		}
	};

	void LCDdisplay::init_pwm_pin(uint pin) {
		this->bl_pwm_pin = pin ;
		gpio_set_function(pin, GPIO_FUNC_PWM);
		uint slice_num = pwm_gpio_to_slice_num(pin);
    	pwm_config config = pwm_get_default_config();
		pwm_config_set_clkdiv(&config, 500.f);
		pwm_config_set_wrap(&config, 100); 
		pwm_init(slice_num, &config, true);	
	}
	
	void LCDdisplay::send_raw_data_one_cycle(uint raw_bits[]) { // Array of Bit 7, Bit 6, Bit 5, Bit 4, RS
		uint32_t bit_value = pin_values_to_mask(raw_bits,5) ;
		gpio_put_masked(this->LCDmask, bit_value) ;
		gpio_put(this->LCDpins[E], HIGH) ;
		sleep_ms(5) ;
		gpio_put(this->LCDpins[E], LOW) ; // gpio values on other pins are pushed at the HIGH->LOW change of the clock. 
		sleep_ms(5) ;
	};
		
	void LCDdisplay::send_full_byte(uint rs, uint databits[]) { // RS + array of Bit7, ... , Bit0
		// send upper nibble (MSN)
		uint rawbits[5];
		rawbits[4] = rs ;
		for (int i = 0 ; i<4 ; i++) { rawbits[i]=databits[i];}
		send_raw_data_one_cycle(rawbits);
		// send lower nibble (LSN)
		for (int i = 0; i<4 ; i++) { rawbits[i]=databits[i+4];}
		send_raw_data_one_cycle(rawbits);
	};
	
	
	LCDdisplay::LCDdisplay(int bit4_pin, int bit5_pin, int bit6_pin, int bit7_pin, int rs_pin, int e_pin, int width, int depth) { // constructor
		this->LCDpins[0] = bit7_pin;
		this->LCDpins[1] = bit6_pin;
		this->LCDpins[2] = bit5_pin;
		this->LCDpins[3] = bit4_pin;
		this->LCDpins[4] = rs_pin;
		this->LCDpins[5] = e_pin;
		this->bl_pwm_pin = 255 ;
		this->no_chars = width;
		this->no_lines = depth;
	};

		LCDdisplay::LCDdisplay(int bit4_pin, int bit5_pin, int bit6_pin, int bit7_pin, int rs_pin, int e_pin, int bl_pin, int width, int depth) { // constructor
		this->LCDpins[0] = bit7_pin;
		this->LCDpins[1] = bit6_pin;
		this->LCDpins[2] = bit5_pin;
		this->LCDpins[3] = bit4_pin;
		this->LCDpins[4] = rs_pin;
		this->LCDpins[5] = e_pin;
		this->bl_pwm_pin = bl_pin;
		this->no_chars = width;
		this->no_lines = depth;
	};
	
	void LCDdisplay::clear() {
		uint clear_display[8] = {0,0,0,0,0,0,0,1};
		send_full_byte(COMMAND, clear_display);
		sleep_ms(10) ; // extra sleep due to equipment time needed to clear the display
	};
	
		
	void LCDdisplay::cursor_off() {
		uint no_cursor[8] = {0,0,0,0,1,1,0,0};
		send_full_byte(COMMAND, no_cursor);
		this->cursor_status[0] = 0;
		this->cursor_status[1] = 0;
	};

	void LCDdisplay::cursor_on() {
		uint command_cursor[8] = {0,0,0,0,1,1,1,1};
		send_full_byte(COMMAND, command_cursor);
		this->cursor_status[0] = 1;
		this->cursor_status[1] = 1;
	};	
	
	void LCDdisplay::cursor_on (bool blink) {
		uint command_cursor[8] = {0,0,0,0,1,1,1,0};		
		if (blink) command_cursor[7]=1 ;
		send_full_byte(COMMAND, command_cursor);
		this->cursor_status[0] = 1;
		this->cursor_status[1] = command_cursor[7];
	};		
		
	void LCDdisplay::display_off() {
		uint command_display[8] = {0,0,0,0,1,0,0,0};
		command_display[7] = this->cursor_status[1];
		command_display[6] = this->cursor_status[0];		
		send_full_byte(COMMAND, command_display);
	};

	void LCDdisplay::display_on() {
		uint command_display[8] = {0,0,0,0,1,1,0,0};
		command_display[7] = this->cursor_status[1];
		command_display[6] = this->cursor_status[0];		
		send_full_byte(COMMAND, command_display);
	};
	
	void LCDdisplay::set_backlight(int brightness){
		if ( this->bl_pwm_pin < 30) {
		   pwm_set_gpio_level(this->bl_pwm_pin, brightness);
		}
	}

	void LCDdisplay::init() { // initialize the LCD
	
		uint all_ones[6] = {1,1,1,1,1,1};
		uint set_function_8[5] = {0,0,1,1,0};
		uint set_function_4a[5] = {0,0,1,0,0};
		
		uint set_function_4[8] = {0,0,1,0,0,0,0,0};
		uint cursor_set[8] = {0,0,0,0,0,1,1,0};
		uint display_prop_set[8] = {0,0,0,0,1,1,0,0};
		
		//set mask, initialize masked pins and set to LOW 
		this->LCDmask_c = pin_values_to_mask(all_ones,6);
		this->LCDmask = pin_values_to_mask(all_ones,5);
		gpio_init_mask(this->LCDmask_c);   			// init all LCDpins
		gpio_set_dir_out_masked(this->LCDmask_c);	// Set as output all LCDpins
		gpio_clr_mask(this->LCDmask_c);				// LOW on all LCD pins 
		
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

		if (this->bl_pwm_pin < 30 ){
			init_pwm_pin(this->bl_pwm_pin);		};
			
		this->cursor_status[0] = 0;
		this->cursor_status[1] = 0;
	};

	void LCDdisplay::goto_pos(int pos_i, int line) {
		uint eight_bits[8];
		uint pos = (uint)pos_i;
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
	
	void LCDdisplay::print(const char * str) {
		uint eight_bits[8];
		int i = 0 ;
		while (str[i] != 0) {
			uint_into_8bits(eight_bits,(uint)(str[i]));
			send_full_byte(DATA, eight_bits);
			++i;
		}
	};
		
	void LCDdisplay::print_wrapped(const char * str) {
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
				

