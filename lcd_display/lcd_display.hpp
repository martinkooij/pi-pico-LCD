#pragma once

#define BLINK true
#define NO_BLINK false

class LCDdisplay {
	
	private:
	int LCDpins[6];
	uint bl_pwm_pin = 255;
	uint32_t LCDmask_c = 0 ; // with clock
	uint32_t LCDmask = 0 ; //without clock
	int no_chars;
	int no_lines;
	int cursor_status[2] = {0,0};
	
	uint32_t pin_values_to_mask(uint raw_bits[],int length);

	void init_pwm_pin(uint pin);
	
	void uint_into_8bits(uint raw_bits[], uint one_byte);
	
	void send_raw_data_one_cycle(uint raw_bits[]);
		
	void send_full_byte(uint rs, uint databits[]);
	
	
	public:
	
	LCDdisplay(int bit4_pin, int bit5_pin, int bit6_pin, int bit7_pin, int rs_pin, int e_pin, int width, int depth);
    LCDdisplay(int bit4_pin, int bit5_pin, int bit6_pin, int bit7_pin, int rs_pin, int e_pin, int bl_pin, int width, int depth);
	
	void set_backlight(int brightness);
	
	void clear();

	void cursor_off();

	void cursor_on();

	void cursor_on(bool blink);
	
	void init() ;

	void goto_pos(int pos, int line);
	
	void print(const char * str);
		
	void print_wrapped(const char * str);
	
	void display_off() ;
	
	void display_on() ;
				
};

