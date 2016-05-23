//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include <deque>
#include <thread>
#include <mutex>
#include <string>

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

constexpr const static unsigned int MAX_BUTTONS = 30;

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
    Inventory_Strings = ref new Platform::Collections::Vector<Platform::String^>();
    StatusNotify = ref new Platform::Collections::Vector<Platform::String^>();

    for (int x = 0;x < 20; ++x)
    {
        Inventory_Strings->Append(L"Silver Dragon Scale Mail +5 blessed");
    }

	this->DataContext = this;
    output_string = std::wstring(NativeMainPage::max_width_offset * NativeMainPage::max_height, L'C');
    for (int x = 1; x <= NativeMainPage::max_height; ++x)
    {
        output_string[x * NativeMainPage::max_width_offset - 1] = '\n';
    }
	for(int i=0; i< MAX_BUTTONS; i++)
	{ 
		Button ^button = ref new Button();
		button->Content=ref new Platform::String( std::to_wstring(i).c_str());
		button->Margin = 15;
		Action_Button_Stack->Children->Append(button);
		//Action_Button_Stack->Items->Append(button);
		//Action_Button_Stack->Conten
	}
	
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
void NethackUWP::MainPage::OutputBox_Tapped(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//a;
	2 + 2;
}

void NethackUWP::MainPage::Send_butt_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    {
        lock_guard<mutex> lock(blocked_on_input);

        if (input_string.empty())
            input_string_cv.notify_all();
        input_string.insert(input_string.end(), begin(InputBox->Text), end(InputBox->Text));
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

void NativeMainPage::write_char(int x, int y, char ch)
{
    {
        lock_guard<mutex> lock(g_mainpage->blocked_on_output);
        if (x >= max_width) abort();
        if (y >= max_height) abort();
        g_mainpage->output_string[max_width_offset * y + x] = ch;
    }
    g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([]() {
        g_mainpage->PropertyChanged(g_mainpage, ref new PropertyChangedEventArgs("OutStringBuf"));
    }));
}

void NativeMainPage::write_notification(const char * str)
{
    if (str == 0 || *str == 0 || !strcmp(str,""))
        return;

    std::wstring strbuf;
    while (*str) { strbuf.push_back(*str); ++str; }
    Platform::String^ pcstr = ref new Platform::String(strbuf.c_str());
    g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([pcstr]() {
        g_mainpage->Notifications->Append(pcstr);
    }));
}
void NativeMainPage::clear_statusbar()
{
	g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([]() {
				g_mainpage->StatusNotify->Clear();

	}));
}
void NativeMainPage::update_statusbar(const char * str)
{
	if (str == 0 || *str == 0)
		return;

	std::wstring strbuf;
	while (*str) { strbuf.push_back(*str); ++str; }
	Platform::String^ pcstr = ref new Platform::String(strbuf.c_str());
	g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([pcstr]() {
//		g_mainpage->StatusNotify->Clear();
		g_mainpage->StatusNotify->Append(pcstr);

	}));
}


void NethackUWP::MainPage::OutputBox_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{

}


void NethackUWP::MainPage::Button_Open_Inventory_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    splitView->IsPaneOpen = true;
}


void NethackUWP::MainPage::Button_Click_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    splitView->IsPaneOpen = false;
}
