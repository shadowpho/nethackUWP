#include <cstdlib>
#include <Windows.h>
#include <vector>
#include <deque>
#include <mutex>
#include <condition_variable>




#define terminate terminate2
#define boolean boolean2
extern "C"
{
#include "hack.h"
}


	using namespace std;

	deque<char> input_string;
	mutex blocked_on_input;
	condition_variable input_string_cv;

	mutex blocked_on_output;
	deque<char> output_string;
	extern "C"
	{

	static char interjection_buf[INTERJECTION_TYPES][1024];
	static int interjection[INTERJECTION_TYPES];

	void win32_abort()
	{
		abort();
	}

	void Delay(int ms)
	{
		Sleep(ms);
	}


	void error(const char*, ...)
	{
		0;
	}

	int(*nt_kbhit)() = 0;

	void win32con_debug_keystrokes()
	{
		0;
	}

	void win32con_handler_info()
	{}

	void interject_assistance(int num, int interjection_type, genericptr_t ptr1, genericptr_t ptr2)
	{
		//yeah everything broked
	}

	void play_usersound(const char* filename, int volume)
	{
		//play sound so good
	}

	void interject(int interjection_type)
	{
		//if (interjection_type >= 0 && interjection_type < INTERJECTION_TYPES)
		//	msmsg(interjection_buf[interjection_type]);
	}

	void nethack_exit(int reason)
	{
		exit(reason);
	}
	int findfirst(char* path) { return 0; }
	int findnext() { return 0; }
	char* foundfile_buffer() { return 0; }
	char * get_username(int *username_size)
	{
		if (username_size != NULL)
			*username_size = 10;
		return "LOL_WHAT";
	}
	void append_slash(char * name) {
		return;
	}
	void map_subkeyvalue(char *op)
	{}

	boolean2 authorize_wizard_mode() { return TRUE; }
	int has_color(int color) { return 1; }

	void port_help() {} //no help for you

	void append_port_id(char*buf) { return; }
	/*
	void do_something_display(){}

	struct window_procs tty_procs = {
		"uwp_tty_i_guess",
		WC_COLOR | WC_MOUSE_SUPPORT | WC_PRELOAD_TILES | WC_INVERSE | WC_EIGHT_BIT_IN,
		0,
		do_something_display,do_something_display,do_something_display,
		do_something_display,do_something_display,do_something_display,
		do_something_display,do_something_display,do_something_display,
		do_something_display,do_something_display,do_something_display,
		do_something_display,do_something_display,do_something_display,


	};
	*/

	//these are so broken
	//void win_tty_init() {};

	void tty_nhbell() {}
	void tty_number_pad() {}
	void tty_delay_output() {}
	void tty_start_screen() {}
	void tty_end_screen() {}
	void nttty_preference_update(const char*) {}
	int tgetch() 
	{
		unique_lock<mutex> blocked_on_input_lock(blocked_on_input) ;
		while (input_string.empty())
		{
			input_string_cv.wait(blocked_on_input_lock);
		}

		int ret = '\n';

		ret = input_string.back();
		input_string.pop_back();

		return ret;

	}
	void gettty() { return;  } //called after ! or ^Z. Don't think we need it.
	void settty(const char*) {}
	void setftty() { return; } //set forward? i dunoo
	void tty_startup() { return; } //this is where I will do startup I guess

								
	//extern deque<char> output_string;
	/*extern mutex blocked_on_output;
	extern void recv_char_print(int c);
	void write_to_button(char c)
	{
		unique_lock<mutex> blocked_on_output_lock(blocked_on_output);

		recv_char_print((int)c);
		
	}
	*/
	
	void xputc(char c) {
		output_string.push_back(c); //it's 1 am and I wanna sleep

	}
	void cl_end() {}
	void clear_screen() {}
	void home() {}
	void standoutbeg() {}
	void standoutend() {}
	void cl_eos() {}
	void term_start_attr() {}
	void term_end_attr() {}
	void term_start_raw_bold() {}
	void term_end_raw_bold() {}
	void term_end_color() {}
	void term_start_color() {}
	void g_putch() {}
	void cmov() {}
	void nocmov() {}
	void backsp() {}
	void erase_char() {}
	void kill_char() {}

	void mswin_procs() {}
	void xputs(char* printme) { printf("%s", printme); };
	void load_keyboard_handler() {};
}
#undef boolean
#undef terminate