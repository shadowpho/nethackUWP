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
#include <future>

#include "../NativeMainPage.h"

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

const std::vector<wchar_t*> DEFAULT_KEYS = {L"i",L".",L"z",L"#engrave",L"o",L"u",L"l",L"d",L"w",L"e",};

MainPage::MainPage()
{
	InitializeComponent();
    if (g_mainpage != nullptr)
        abort();
    g_mainpage = this;
    g_corewindow = Windows::UI::Core::CoreWindow::GetForCurrentThread();
    Notifications = ref new Platform::Collections::Vector<Platform::String^>();
    Inventory_Strings = ref new Platform::Collections::Vector<Platform::String^>();
    Last_Notifications = ref new Platform::Collections::Vector<Platform::String^>();
    Modal_Answers = ref new Platform::Collections::Vector<Platform::String^>();

    this->DataContext = this;
    output_string = std::wstring(NativeMainPage::max_width_offset * NativeMainPage::max_height, L' ');
    for (int x = 1; x <= NativeMainPage::max_height; ++x)
    {
        output_string[x * NativeMainPage::max_width_offset - 1] = '\n';
    }
	for(int i=0; i< MAX_BUTTONS; i++)
	{ 
		Button ^button = ref new Button();
		button->Click += ref new Windows::UI::Xaml::RoutedEventHandler(this, &NethackUWP::MainPage::Quick_Button_Click);
		
		//button->AddHandler(button_Click, Quick_Button_Click, true);
		if (i < DEFAULT_KEYS.size())
			button->Content = ref new Platform::String(DEFAULT_KEYS[i]);
		else
			button->Content=ref new Platform::String( std::to_wstring(i).c_str());
		button->Margin = Thickness(5,0,5,15);
		Action_Button_Stack->Children->Append(button);
		//Action_Button_Stack->Items->Append(button);
		//Action_Button_Stack->Conten
	}

	OutputBox->AddHandler(TappedEvent, ref new TappedEventHandler(this, &NethackUWP::MainPage::OutputBox_Tapped2), true);
	//OutputBox->Tapped += ref new Windows::UI::Xaml::Input::TappedEventHandler(this, &NethackUWP::MainPage::OutputBox_Tapped2,true);
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

        flags.perm_invent = true;

		//resuming = pcmain(argc, argv);
        display_inventory(nullptr, 0);
		moveloop(0);
		//trololololololololololololololololololo
	});
	
}

void NethackUWP::MainPage::OutputBox_Tapped2(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e)
{
	Point pressed = e->GetPosition(OutputBox);
	bool up = false;
	bool down = false;
	bool right = false;
	bool left = false;

	std::string send_str = "";

	if (pressed.X > (OutputBox->RenderSize.Width * 2.0 / 3.0))
		right= true;
	if (pressed.X < (OutputBox->RenderSize.Width / 3.0 ))
		left= true;
	if (pressed.Y >(OutputBox->RenderSize.Height*2.0 / 3.0 ))
		down = true;
	if (pressed.Y <(OutputBox->RenderSize.Height / 3.0 ))
		up = true;

	//XXX DIAGONALS
    if (up && left)
        send_str = "y";
    else if (up && right)
        send_str = "u";
    else if (down && left)
        send_str = "b";
    else if (down && right)
        send_str = "n";
    else if (left)
        send_str = "h";
    else if (up)
        send_str = "k";
    else if (right)
        send_str = "l";
    else if (down)
        send_str = "j";

    if (!send_str.empty())
    {
		lock_guard<mutex> lock(blocked_on_input);
		if (input_string.empty())
			input_string_cv.notify_all();
		input_string.insert(input_string.end(), begin(send_str), end(send_str));
	}

}
void NethackUWP::MainPage::Quick_Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	auto nethack_text = ((Platform::String^)((Button^)sender)->Content);
	{
		lock_guard<mutex> lock(blocked_on_input);
		if (input_string.empty())
			input_string_cv.notify_all();
		input_string.insert(input_string.end(), begin(nethack_text), end(nethack_text));
	}

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

int NativeMainPage::read_char(int &x, int &y)
{
    unique_lock<mutex> lock(g_mainpage->blocked_on_input);
    while (g_mainpage->input_string.empty() && g_mainpage->input_mouse.empty())
    {
        g_mainpage->input_string_cv.wait(lock);
    }
	if (!g_mainpage->input_string.empty())
	{
		int ret = g_mainpage->input_string.back();
		g_mainpage->input_string.pop_back();

		return ret;
	}
	else //mouse contact
	{
		std::tuple<unsigned short, unsigned short> ret_tuple = g_mainpage->input_mouse.back();
		g_mainpage->input_mouse.pop_back();
		x = std::get<0>(ret_tuple);
		y = std::get<1>(ret_tuple);
		return 0;
	}
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
        g_mainpage->Last_Notifications->Append(pcstr);
        g_mainpage->Last_Notification = pcstr;
        if (g_mainpage->Last_Notifications->Size > 1)
            g_mainpage->notificationsExpander->Visibility = Windows::UI::Xaml::Visibility::Visible;
        g_mainpage->PropertyChanged(g_mainpage, ref new PropertyChangedEventArgs("Last_Notification"));
    }));
}

void NativeMainPage::clear_notifications()
{
    g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([]() {
        g_mainpage->Last_Notification = "";
        g_mainpage->Last_Notifications->Clear();
        g_mainpage->notificationsExpander->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
        g_mainpage->PropertyChanged(g_mainpage, ref new PropertyChangedEventArgs("Last_Notification"));
    }));
}
void NativeMainPage::clear_statusbar()
{
	g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([]() {
        g_mainpage->Status_Line_1 = nullptr;
        g_mainpage->Status_Line_2 = nullptr;
        g_mainpage->PropertyChanged(g_mainpage, ref new PropertyChangedEventArgs("Status_Line_1"));
        g_mainpage->PropertyChanged(g_mainpage, ref new PropertyChangedEventArgs("Status_Line_2"));
    }));
}
void NativeMainPage::clear_inv()
{
    g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([]() {
        g_mainpage->Inventory_Strings->Clear();
    }));
}

void NativeMainPage::add_inv_str(const char* str, bool is_header, int attr, char accelerator)
{
    if (str == 0 || *str == 0)
		return;

	std::wstring strbuf;
    if (attr == 0)
    {
        if (accelerator != 0)
            strbuf.push_back(accelerator);
        else
            strbuf.push_back(' ');
        strbuf.append(L" - ");
    }
	while (*str) { strbuf.push_back(*str); ++str; }
	Platform::String^ pcstr = ref new Platform::String(strbuf.c_str());
	g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([pcstr]() {
		g_mainpage->Inventory_Strings->Append(pcstr);
	}));
}

static auto cstr_to_platstr(const char* str)
{
    if (str == nullptr)
        return (Platform::String^)nullptr;
    std::wstring strbuf;
    while (*str) { strbuf.push_back(*str); ++str; }
    Platform::String^ pcstr = ref new Platform::String(strbuf.c_str(), strbuf.size());
    return pcstr;
}

void NativeMainPage::update_statusbar(const char * str)
{
    if (str == 0 || *str == 0)
        return;

    Platform::String^ pcstr = cstr_to_platstr(str);
    g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([pcstr]() {
        if (g_mainpage->Status_Line_1 == nullptr)
        {
            g_mainpage->Status_Line_1 = pcstr;
            g_mainpage->PropertyChanged(g_mainpage, ref new PropertyChangedEventArgs("Status_Line_1"));
        }
        else
        {
            g_mainpage->Status_Line_2 = pcstr;
            g_mainpage->PropertyChanged(g_mainpage, ref new PropertyChangedEventArgs("Status_Line_2"));
        }
    }));
}

struct NativeMainPageImpl
{
    std::mutex promise_lock;
    std::promise<int> yn_function_promise;

} g_nativepage_impl;

char NativeMainPage::ask_inv_function(const char *question, char def)
{
    {
        std::lock_guard<std::mutex> lock(g_nativepage_impl.promise_lock);
        g_nativepage_impl.yn_function_promise = std::promise<int>();
    }

    Platform::String^ psQuestion = cstr_to_platstr(question);
    g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([psQuestion]() mutable {
        g_mainpage->Modal_Question = psQuestion;
        g_mainpage->Modal_Answers->Clear();

        for (auto&& inv : g_mainpage->Inventory_Strings)
            g_mainpage->Modal_Answers->Append(inv);

        g_mainpage->PropertyChanged(g_mainpage, ref new PropertyChangedEventArgs("Modal_Question"));
        g_mainpage->modalDialog->Visibility = Windows::UI::Xaml::Visibility::Visible;
    }));


    std::future<int> f = g_nativepage_impl.yn_function_promise.get_future();
    auto f_val = f.get();
    if (f_val >= 0)
        return g_mainpage->Inventory_Strings->GetAt(f_val)->Data()[0];
    return def;
}

char NativeMainPage::ask_direction(char def)
{
    static menu_t direction_menu = {
        std::string("In what direction?"),
        std::vector<choice_t>{
            {0, true, 'k', 0, false, "North"},
            {0, true, 'j', 0, false, "South"},
            {0, true, 'l', 0, false, "East"},
            {0, true, 'h', 0, false, "West"},
            {0, true, 'u', 0, false, "Northeast"},
            { 0, true, 'y', 0, false, "Northwest" },
            { 0, true, 'n', 0, false, "Southeast" },
            { 0, true, 'b', 0, false, "Southwest" },
            { 0, true, '<', 0, false, "Up" },
            { 0, true, '>', 0, false, "Down" },
            { 0, true, 's', 0, false, "Yourself" },
    }
    };
    int val = def;
    ask_menu(direction_menu, val);
    return static_cast<char>(val);
}

char NativeMainPage::ask_yn_function(const char *question, const char *choices, char def)
{
    {
        std::lock_guard<std::mutex> lock(g_nativepage_impl.promise_lock);
        g_nativepage_impl.yn_function_promise = std::promise<int>();
    }

    Platform::String^ psQuestion = cstr_to_platstr(question);
    g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([psQuestion, choices]() mutable {
        g_mainpage->Modal_Question = psQuestion;
        g_mainpage->Modal_Answers->Clear();
        if (choices != nullptr)
            while (*choices != 0)
            {
                std::string s;
                s.push_back(*choices);
                g_mainpage->Modal_Answers->Append(cstr_to_platstr(s.c_str()));
                ++choices;
            }

        g_mainpage->PropertyChanged(g_mainpage, ref new PropertyChangedEventArgs("Modal_Question"));

        g_mainpage->modalDialog->Visibility = Windows::UI::Xaml::Visibility::Visible;
    }));

    std::future<int> f = g_nativepage_impl.yn_function_promise.get_future();
    auto f_val = f.get();
    return f_val == -1 ? def : choices[f_val];
}
void NativeMainPage::complete_yn_function(int idx)
{
    std::lock_guard<std::mutex> lock(g_nativepage_impl.promise_lock);
    g_nativepage_impl.yn_function_promise.set_value(idx);
}
bool NativeMainPage::ask_menu(const menu_t& m, int& selection_value)
{
    {
        std::lock_guard<std::mutex> lock(g_nativepage_impl.promise_lock);
        g_nativepage_impl.yn_function_promise = std::promise<int>();
    }

    Platform::String^ psQuestion = cstr_to_platstr(m.prompt.c_str());

    g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([psQuestion, &m]() {
        g_mainpage->Modal_Question = psQuestion;
        g_mainpage->Modal_Answers->Clear();

        for (auto&& s : m.choices)
            g_mainpage->Modal_Answers->Append(cstr_to_platstr(s.str.c_str()));

        g_mainpage->PropertyChanged(g_mainpage, ref new PropertyChangedEventArgs("Modal_Question"));

        g_mainpage->modalDialog->Visibility = Windows::UI::Xaml::Visibility::Visible;
    }));

    std::future<int> f = g_nativepage_impl.yn_function_promise.get_future();
    auto idx = f.get();
    if (idx == -1 || !m.choices[idx].selectable)
        return true;

    selection_value = m.choices[idx].value;
    return false;
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


void NethackUWP::MainPage::listView_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
    if (e->AddedItems->Size == 1)
    {
        auto obj = (Platform::String^)e->AddedItems->GetAt(0);
        unsigned int idx = -1;
        Modal_Answers->IndexOf(obj, &idx);

        NativeMainPage::complete_yn_function(idx);
        g_mainpage->modalDialog->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    }
}


void NethackUWP::MainPage::SymbolIcon_Tapped(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e)
{
    // Dismiss modal dialog
    NativeMainPage::complete_yn_function(-1);
    modalDialog->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}


void NethackUWP::MainPage::Button_Open_History_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    splitView_History->IsPaneOpen = true;
}


void NethackUWP::MainPage::Button_Close_History_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    splitView_History->IsPaneOpen = false;
}


void NethackUWP::MainPage::ExpandNotifications(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e)
{
    notificationsExpander->Visibility = Windows::UI::Xaml::Visibility::Visible;
}


void NethackUWP::MainPage::CollapseNotifications(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e)
{
    notificationsExpander->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}
