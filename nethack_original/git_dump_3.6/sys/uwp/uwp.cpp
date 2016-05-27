#include <cstdlib>
#include <Windows.h>
#include <cassert>
#include <vector>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <unordered_map>

#include "../../NativeMainPage.h"

#define terminate terminate2
#define boolean boolean2
extern "C"
{
#include "hack.h"
#include "func_tab.h"
}


	using namespace std;

    extern "C"
	{

		void mswin_suspend_nhwindows(const char *) { abort(); } //doesn't happen
		void mswin_resume_nhwindows(void) { abort(); } //doesn't happen
		void mswin_destroy_nhwindow(winid wid) { return; } //nethack doesn't own these

		void mswin_init_nhwindows(int *argc, char **argv) {}
		void mswin_player_selection(void) {}
		void mswin_askname(void) { strcpy(plname, "best_playa-wizard-elf-female-chaos"); } //ask user for name, bail if cancel.
		void mswin_get_nh_event(void) {}
		void mswin_exit_nhwindows(const char *) {}

		/*  Create a window of type "type" which can be
		NHW_MESSAGE     (top line)
		NHW_STATUS      (bottom lines)
		NHW_MAP         (main dungeon)
		NHW_MENU        (inventory or other "corner" windows)
		NHW_TEXT        (help/text, full screen paged window)
		*/
		winid mswin_create_nhwindow(int type) {
            static winid g_window_id = 0;
			assert(type <= NHW_TEXT);
			assert(type >= 0);
			//we'll create them at different points.
			return g_window_id++;
		}

		void mswin_clear_nhwindow(winid wid) 
		{
            if (wid == WIN_ERR)
                abort();

            if (wid == WIN_MESSAGE)
                NativeMainPage::clear_notifications();

            if (wid == WIN_STATUS)
				NativeMainPage::clear_statusbar();
		}
		void mswin_display_nhwindow(winid wid, BOOLEAN_P block) 
		{
            if (wid == WIN_ERR)
                abort();
            //NativeMainPage::write_notification();
		}

		void mswin_curs(winid wid, int x, int y) {}
		void mswin_display_file(const char *filename, BOOLEAN_P must_exist) {}

        std::unordered_map<winid, menu_t> g_menus;
        std::string g_inven_prompt;

		void mswin_start_menu(winid wid)
        {
            if (wid == WIN_ERR)
                abort();
            if (wid == WIN_INVEN)
            {
                NativeMainPage::clear_inv();
            }
            else
            {
                auto& menu = g_menus[wid];
                menu.prompt.clear();
                menu.choices.clear();
            }
        }
		void mswin_add_menu(winid wid, int glyph, const ANY_P *identifier,
			CHAR_P accelerator, CHAR_P group_accel, int attr,
			const char *str, BOOLEAN_P presel)
        {
            if (wid == WIN_ERR)
                abort();
            if (wid == WIN_INVEN)
            {
                NativeMainPage::add_inv_str(str, identifier == nullptr, attr, accelerator);
            }
            else
            {
                choice_t ch;
                ch.accel = accelerator;
                ch.attr = attr;
                ch.selectable = identifier != nullptr;
                ch.value = identifier != nullptr ? identifier->a_int : 0;
                ch.str = str;
                ch.preselected = presel;
                g_menus[wid].choices.push_back(std::move(ch));
            }
        }
		void mswin_end_menu(winid wid, const char *prompt) {
            if (wid == WIN_INVEN && prompt == nullptr)
                return;
            if (wid == WIN_INVEN)
            {
                g_inven_prompt = prompt;
                return;
            }

            g_menus[wid].prompt = prompt;
        }
		int mswin_select_menu(winid wid, int how, MENU_ITEM_P **selected)
        {
            if (how == PICK_NONE)
                return 0;

            int selection_value = 0;
            auto cancelled = NativeMainPage::ask_menu(g_menus[wid], selection_value);

            if (cancelled)
                return -1;

            *selected = (MENU_ITEM_P*)malloc(sizeof(MENU_ITEM_P) * 1);
            memset(*selected, 0, sizeof(MENU_ITEM_P) * 1);
            (*selected)->item.a_int = selection_value;
            (*selected)->count = -1;
            return 1;
        }
        void mswin_update_inventory(void) {
            if (flags.perm_invent && program_state.something_worth_saving
                && iflags.window_inited && WIN_INVEN != WIN_ERR)
                display_inventory(NULL, FALSE);
        }
		void mswin_mark_synch(void) {}
		void mswin_wait_synch(void) {}
		void mswin_cliparound(int x, int y) {}
        //xxx
		void mswin_print_glyph(winid wid, XCHAR_P x, XCHAR_P y, int glyph, int bkglyph) 
        {
            int ch;
            boolean reverse_on = FALSE;
            int color;
            unsigned special;

#ifdef CLIPPING
            //if (clipping) {
            //    if (x <= clipx || y < clipy || x >= clipxmax || y >= clipymax)
            //        return;
            //}
#endif
            /* map glyph to character and color */
            (void)mapglyph(glyph, &ch, &color, &special, x, y);

            //print_vt_code2(AVTC_SELECT_WINDOW, window);

            ///* Move the cursor. */
            //tty_curs(window, x, y);

            //print_vt_code3(AVTC_GLYPH_START, glyph2tile[glyph], special);

#ifndef NO_TERMS
            if (ul_hack && ch == '_') { /* non-destructive underscore */
                (void)putchar((char) ' ');
                backsp();
            }
#endif

#ifdef TEXTCOLOR
            //if (color != ttyDisplay->color) {
            //    if (ttyDisplay->color != NO_COLOR)
            //        term_end_color();
            //    ttyDisplay->color = color;
            //    if (color != NO_COLOR)
            //        term_start_color(color);
            //}
#endif /* TEXTCOLOR */

            /* must be after color check; term_end_color may turn off inverse too */
            if (((special & MG_PET) && iflags.hilite_pet)
                || ((special & MG_OBJPILE) && iflags.hilite_pile)
                || ((special & MG_DETECT) && iflags.use_inverse)) {
                //term_start_attr(ATR_INVERSE);
                reverse_on = TRUE;
            }

#if defined(USE_TILES) && defined(MSDOS)
            if (iflags.grmode && iflags.tile_view)
                xputg(glyph, ch, special);
            else
#endif
            if (ch > 127) abort();
            NativeMainPage::write_char(x, y, (char)ch);
            //g_putch(ch); /* print the character */

            if (reverse_on) {
                //term_end_attr(ATR_INVERSE);
#ifdef TEXTCOLOR
                /* turn off color as well, ATR_INVERSE may have done this already */
                //if (ttyDisplay->color != NO_COLOR) {
                //    term_end_color();
                //    ttyDisplay->color = NO_COLOR;
                //}
#endif
            }

            //print_vt_code1(AVTC_GLYPH_END);

            //wins[window]->curx++; /* one character over */
            //ttyDisplay->curx++;   /* the real cursor moved too */
        }
		
		void mswin_raw_print(const char *str) 
		{
			NativeMainPage::write_notification(str);
		}
		void mswin_raw_print_bold(const char *str) 
		{
            NativeMainPage::write_notification(str);
        }
        void mswin_putstr(winid wid, int attr, const char *text) 
		{ 
			if (wid == WIN_STATUS)
				NativeMainPage::update_statusbar(text);
			else
				mswin_raw_print(text);
		}
        void mswin_putstr_ex(winid wid, int attr, const char *text, int) 
		{ 
			mswin_raw_print(text);
		}
        int mswin_nhgetch(void) { return tgetch(); }
		int mswin_nh_poskey(int *x, int *y, int *mod) 
		{ 
			int pass_x = 0;
			int pass_y = 0;
			int ret = NativeMainPage::read_char(pass_x, pass_y);
			if (ret == 0)
			{
				*x = pass_x;
				*y = pass_y;
				*mod = CLICK_1;
				return 0;
			}
			else
				return ret;
			//return tgetch(); 
		}
		void mswin_nhbell(void) {}
		int mswin_doprev_message(void) { return 0; }
		char mswin_yn_function(const char *question, const char *choices, CHAR_P def)
        {
            assert(question != nullptr);
            if (strcmp(question, "In what direction?") == 0) // directions
                return NativeMainPage::ask_direction(def);
            else if(strncmp(question, "What do you want to ", sizeof("What do you want to")) == 0)
            {
                return NativeMainPage::ask_inv_function(question, def);
            }
            else
            {
                return NativeMainPage::ask_yn_function(question, choices, def);
            }
        }
		void mswin_getlin(const char *question, char *input) {}

		int mswin_get_ext_cmd(void) {
            menu_t ext_command_menu;
            auto p_extcmd = extcmdlist;
            while (p_extcmd->ef_txt != nullptr)
            {
                ext_command_menu.choices.push_back({});
                auto& choice = ext_command_menu.choices.back();
                choice.str = p_extcmd->ef_txt;
                choice.str.append(" - ");
                choice.str.append(p_extcmd->ef_desc);
                choice.value = p_extcmd - extcmdlist;
                choice.selectable = true;
                ++p_extcmd;
            }
            ext_command_menu.prompt = "What extended command?";
            int val = -1;
            NativeMainPage::ask_menu(ext_command_menu, val);
            return val;
        }
		void mswin_number_pad(int state) {}
		void mswin_delay_output(void) 
		{
			Sleep(50); //50 ms NAP TIME lol
		}
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

    int uwp_kbhit()
    {
        return 0;
    }

	int(*nt_kbhit)() = &uwp_kbhit;

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
		//return NativeMainPage::read_char();
		abort();
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