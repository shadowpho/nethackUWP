#include <cstdlib>
#include <Windows.h>


#define boolean boolean2
extern "C"
{
#include "hack.h"


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
char * get_username(int *username_size) { *username_size = 10;  return "LOL_WHAT"; }
void append_slash(char * name) {
	return;
}
void map_subkeyvalue(char *op)
{}

boolean2 authorize_wizard_mode() { return TRUE; }
int has_color(int color) { return 1; }

void port_help() {} //no help for you

void append_port_id(char*buf) { return; }


//these are so broken
void tty_procs() {};
void win_tty_init() {};
void xputs() {};
void load_keyboard_handler() {};
}
#undef boolean
