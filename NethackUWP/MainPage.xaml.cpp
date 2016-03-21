//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include <deque>
#include <thread>
#include <mutex>

bool game_is_running = false;
bool waiting_for_input = false;
#define boolean boolean2
#define terminate terminate2

extern "C" {
#include "hack.h"
#include "dlb.h"
}
#undef terminate
#undef boolean

using namespace NethackUWP;
using namespace std;


using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

MainPage::MainPage()
{
	InitializeComponent();
}


void NethackUWP::MainPage::button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (game_is_running == true) return;
	game_is_running = true;
	static thread nethack_thread([]()
	{
		sys_early_init();
		choose_windows("tty");//dun worry
							  //	tty_procs (NULL, NULL); //dun worry
		initoptions(); //nuh nuh nuh

		dlb_init();
		init_nhwindows(0, 0);
		newgame();

		//resuming = pcmain(argc, argv);

		//moveloop(resuming);
		//trololololololololololololololololololo
	});
	
}

extern deque<char> input_string;
extern deque<char> output_string; //XXX
extern mutex blocked_on_input;
extern condition_variable input_string_cv;

void NethackUWP::MainPage::Send_butt_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

		unique_lock<mutex> blocked_on_input_lock(blocked_on_input);

		input_string.insert(input_string.end(),begin(InputBox->Text), end(InputBox->Text));
		input_string.push_back('\n');
		InputBox->Text = "";
		input_string_cv.notify_one();
		//XXX XXX HACK NOT SAFE DO NOT USE NSFW PLZ REMOVE
		//THIS WILL BREAK.
		wstring my_sad_performance;
		while (!output_string.empty())
		{
			char wtf = output_string.at(0);
			output_string.pop_front();
			//my_sad_performance.append(wtf);
			my_sad_performance.push_back(wtf);
			
			
		}
		

		OutputBox->Text = ref new String(my_sad_performance.c_str());
		//OutputBox->Text = my_sad_performance;

}


void NethackUWP::MainPage::recv_char_print(int c)
{
	//OutputBox->Text += String c;
}

