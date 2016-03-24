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

		void mswin_init_nhwindows(int *argc, char **argv) {}
		void mswin_player_selection(void) {}
		void mswin_askname(void) { strcpy(plname, "best_playa"); } //ask user for name, bail if cancel.
		void mswin_get_nh_event(void) {}
		void mswin_exit_nhwindows(const char *) {}
		void mswin_suspend_nhwindows(const char *) {}
		void mswin_resume_nhwindows(void) {}
		winid mswin_create_nhwindow(int type) { return 0; }
		void mswin_clear_nhwindow(winid wid) {}
		void mswin_display_nhwindow(winid wid, BOOLEAN_P block) {}
		void mswin_destroy_nhwindow(winid wid) {}
		void mswin_curs(winid wid, int x, int y) {}
		void mswin_putstr(winid wid, int attr, const char *text) {}
		void mswin_putstr_ex(winid wid, int attr, const char *text, int) {}
		void mswin_display_file(const char *filename, BOOLEAN_P must_exist) {}
		void mswin_start_menu(winid wid) {}
		void mswin_add_menu(winid wid, int glyph, const ANY_P *identifier,
			CHAR_P accelerator, CHAR_P group_accel, int attr,
			const char *str, BOOLEAN_P presel) {}
		void mswin_end_menu(winid wid, const char *prompt) {}
		int mswin_select_menu(winid wid, int how, MENU_ITEM_P **selected) { return 0; }
		void mswin_update_inventory(void) {}
		void mswin_mark_synch(void) {}
		void mswin_wait_synch(void) {}
		void mswin_cliparound(int x, int y) {}
		void mswin_print_glyph(winid wid, XCHAR_P x, XCHAR_P y, int glyph, int bkglyph) {}
		
		std::vector<std::string> v;

		void mswin_raw_print(const char *str) 
		{
			v.push_back(str);
		}
		void mswin_raw_print_bold(const char *str) 
		{
			v.push_back(str);
		}
		int mswin_nhgetch(void) { return 0; }
		int mswin_nh_poskey(int *x, int *y, int *mod) { return 0; }
		void mswin_nhbell(void) {}
		int mswin_doprev_message(void) { return 0; }
		char mswin_yn_function(const char *question, const char *choices, CHAR_P def) { return 0; }
		void mswin_getlin(const char *question, char *input) {}
		int mswin_get_ext_cmd(void) { return 0; }
		void mswin_number_pad(int state) {}
		void mswin_delay_output(void) {}
		void mswin_change_color(void) {}
		char *mswin_get_color_string(void) { return 0; }
		void mswin_start_screen(void) {}
		void mswin_end_screen(void) {}
		void mswin_outrip(winid wid, int how, time_t when) {}
		void mswin_preference_update(const char *pref) {}
		char *mswin_getmsghistory(BOOLEAN_P init) { return 0; }
		void mswin_putmsghistory(const char *msg, BOOLEAN_P) {}


		/* Interface definition, for windows.c */
		struct window_procs mswin_procs = {
			"MSWIN",
			WC_COLOR | WC_HILITE_PET | WC_ALIGN_MESSAGE | WC_ALIGN_STATUS | WC_INVERSE
			| WC_SCROLL_AMOUNT | WC_SCROLL_MARGIN | WC_MAP_MODE | WC_FONT_MESSAGE
			| WC_FONT_STATUS | WC_FONT_MENU | WC_FONT_TEXT | WC_FONT_MAP
			| WC_FONTSIZ_MESSAGE | WC_FONTSIZ_STATUS | WC_FONTSIZ_MENU
			| WC_FONTSIZ_TEXT | WC_TILE_WIDTH | WC_TILE_HEIGHT | WC_TILE_FILE
			| WC_VARY_MSGCOUNT | WC_WINDOWCOLORS | WC_PLAYER_SELECTION
			| WC_SPLASH_SCREEN | WC_POPUP_DIALOG,
			0L, mswin_init_nhwindows, mswin_player_selection, mswin_askname,
			mswin_get_nh_event, mswin_exit_nhwindows, mswin_suspend_nhwindows,
			mswin_resume_nhwindows, mswin_create_nhwindow, mswin_clear_nhwindow,
			mswin_display_nhwindow, mswin_destroy_nhwindow, mswin_curs, mswin_putstr,
			genl_putmixed, mswin_display_file, mswin_start_menu, mswin_add_menu,
			mswin_end_menu, mswin_select_menu,
			genl_message_menu, /* no need for X-specific handling */
			mswin_update_inventory, mswin_mark_synch, mswin_wait_synch,
			mswin_cliparound,
			mswin_print_glyph, mswin_raw_print, mswin_raw_print_bold, mswin_nhgetch,
			mswin_nh_poskey, mswin_nhbell, mswin_doprev_message, mswin_yn_function,
			mswin_getlin, mswin_get_ext_cmd, mswin_number_pad, mswin_delay_output,

			/* other defs that really should go away (they're tty specific) */
			mswin_start_screen, mswin_end_screen, mswin_outrip,
			mswin_preference_update, mswin_getmsghistory, mswin_putmsghistory,

			genl_can_suspend_yes,
		};




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


	void xputs(char* printme) { printf("%s", printme); };
	void load_keyboard_handler() {};
}
#undef boolean
#undef terminate