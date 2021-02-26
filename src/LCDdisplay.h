class LCDdisplay {
	
	public:
	
	LCDdisplay(uint bit4_pin, uint bit5_pin, uint bit6_pin, uint bit7_pin, uint rs_pin, uint e_pin, uint width, uint depth);
	
	void clear();

	void cursor_off();

	void cursor_on();	
	
	void init() ;

	void goto_pos(uint pos, uint line);
	
	void print(const char * str);
		
	void print_wrapped(const char * str);
				
};

