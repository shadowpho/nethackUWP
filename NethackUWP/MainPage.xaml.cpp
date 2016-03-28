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
    if (g_mainpage != nullptr)
        abort();
    g_mainpage = this;
    g_corewindow = Windows::UI::Core::CoreWindow::GetForCurrentThread();
    Notifications = ref new Platform::Collections::Vector<Platform::String^>();
    this->DataContext = this;
}

void NethackUWP::MainPage::button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (game_is_running == true) return;
	game_is_running = true;
    static thread nethack_thread([]()
	{
		sys_early_init();
		choose_windows("mswin");//dun worry
							  //	tty_procs (NULL, NULL); //dun worry
		initoptions(); //nuh nuh nuh

		dlb_init();
		init_nhwindows(0, 0);
		vision_init();
		display_gamewindows();//dunno
		newgame();
		
		//resuming = pcmain(argc, argv);

		moveloop(0);
		//trololololololololololololololololololo
	});
	
}

void NethackUWP::MainPage::Send_butt_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    {
        lock_guard<mutex> lock(blocked_on_input);

        if (input_string.empty())
            input_string_cv.notify_all();
        input_string.insert(input_string.end(), begin(InputBox->Text), end(InputBox->Text));
        input_string.push_back('\n');
    }
    InputBox->Text = "";
}

int NativeMainPage::read_char()
{
    unique_lock<mutex> lock(g_mainpage->blocked_on_input);
    while (g_mainpage->input_string.empty())
    {
        g_mainpage->input_string_cv.wait(lock);
    }

    int ret = g_mainpage->input_string.back();
    g_mainpage->input_string.pop_back();

    return ret;
}

using namespace Windows::UI::Core;

void NativeMainPage::write_char(int ch)
{
    {
        lock_guard<mutex> lock(g_mainpage->blocked_on_output);

        g_mainpage->output_string.push_back(ch);
    }
    g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([]() {
        g_mainpage->PropertyChanged(g_mainpage, ref new PropertyChangedEventArgs("OutStringBuf"));
    }));
}

void NativeMainPage::write_notification(const char * str)
{
    std::wstring strbuf;
    while (*str) { strbuf.push_back(*str); ++str; }
    Platform::String^ pcstr = ref new Platform::String(strbuf.c_str());
    g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([pcstr]() {
        g_mainpage->Notifications->Append(pcstr);
    }));
}
