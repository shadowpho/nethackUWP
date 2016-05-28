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
#include <cassert>

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
using namespace Windows::UI::Input;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

const std::vector<wchar_t*> DEFAULT_KEYS = {L"#",L".",L"z",L"Z",L"o",L"e",L"l",L"d",L"w",L"W",};

struct NativeMainPageImpl
{
    std::mutex promise_lock;
    std::promise<int> yn_function_promise;
    bool waiting_for_direction = false;
} g_nativepage_impl;

void MainPage::clear_map()
{
    for (auto&& c : map_data)
    {
        for (auto& r : c)
        {
            r = tile_t{ L' ', 0 };
        }
    }
}

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

    for (int y = 0; y < NativeMainPage::max_height; ++y)
        map_data.emplace_back(NativeMainPage::max_width_offset, tile_t{ L' ',0 });

    this->DataContext = this;


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

	//OutputBox->AddHandler(TappedEvent, ref new TappedEventHandler(this, &NethackUWP::MainPage::OutputBox_Tapped2), true);
	//OutputBox->Tapped += ref new Windows::UI::Xaml::Input::TappedEventHandler(this, &NethackUWP::MainPage::OutputBox_Tapped2,true);
	g_mainpage->SizeChanged += ref new Windows::UI::Xaml::SizeChangedEventHandler(this, &NethackUWP::MainPage::OnSizeChanged);
	g_mainpage->KeyDown += ref new Windows::UI::Xaml::Input::KeyEventHandler(this, &NethackUWP::MainPage::OnKeyDown);

	//XXX hardware button




    MapCanvas->PointerPressed +=
        ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &NethackUWP::MainPage::OnPointerPressed);

    MapCanvas->PointerMoved +=
        ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &NethackUWP::MainPage::OnPointerMoved);

    MapCanvas->PointerReleased +=
        ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &NethackUWP::MainPage::OnPointerReleased);

    gestureRecognizer = ref new GestureRecognizer();

    gestureRecognizer->GestureSettings = GestureSettings::Tap;

    gestureRecognizer->Tapped += ref new Windows::Foundation::TypedEventHandler<
        Windows::UI::Input::GestureRecognizer ^,
        Windows::UI::Input::TappedEventArgs ^>(this, &NethackUWP::MainPage::OnTapped);
}

void NethackUWP::MainPage::OnPointerPressed(
    Platform::Object ^sender,
    Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e)
{
    gestureRecognizer->ProcessDownEvent(e->GetCurrentPoint(MapCanvas));
}

void NethackUWP::MainPage::OnPointerMoved(
    Platform::Object ^sender,
    Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e)
{
    gestureRecognizer->ProcessMoveEvents(e->GetIntermediatePoints(MapCanvas));
}

void NethackUWP::MainPage::OnPointerReleased(
    Platform::Object ^sender,
    Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e)
{
    gestureRecognizer->ProcessUpEvent(e->GetCurrentPoint(MapCanvas));
}

void NethackUWP::MainPage::OnTapped(
    Windows::UI::Input::GestureRecognizer ^sender,
    Windows::UI::Input::TappedEventArgs ^args)
{
    Point pressed = args->Position;
    bool up = false;
    bool down = false;
    bool right = false;
    bool left = false;

    std::string send_str = "";

    if (pressed.X > (MapCanvas->RenderSize.Width * 2.0 / 3.0))
        right = true;
    if (pressed.X < (MapCanvas->RenderSize.Width / 3.0))
        left = true;
    if (pressed.Y >(MapCanvas->RenderSize.Height*2.0 / 3.0))
        down = true;
    if (pressed.Y <(MapCanvas->RenderSize.Height / 3.0))
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
    {
        std::lock_guard<std::mutex> lock(g_nativepage_impl.promise_lock);
        if (g_nativepage_impl.waiting_for_direction) {
            if (send_str.empty())
                g_nativepage_impl.yn_function_promise.set_value('s');
            else
                g_nativepage_impl.yn_function_promise.set_value(send_str.front());
            g_mainpage->modalDialog->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
            return;
        }
    }
    if (!send_str.empty())
    {
        lock_guard<mutex> lock(blocked_on_input);
        if (input_string.empty())
            input_string_cv.notify_all();
        input_string.insert(input_string.end(), begin(send_str), end(send_str));
    }
}


void NethackUWP::MainPage::button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	flags.debug = true;
	wchar_t* spooky_ghost_folder = (wchar_t*) Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data();

	char* spooky_ghost = new char[1024];
	char* real_spooky_ghost = spooky_ghost;

	while (*spooky_ghost_folder)
		*spooky_ghost++ = *spooky_ghost_folder++;
	*spooky_ghost++ = '\\';
	*spooky_ghost++ = 'G';
	*spooky_ghost++ = 0;

	fqn_prefix[LEVELPREFIX] = real_spooky_ghost;
	for (int i = 0; i < 7; i++)
	{
		fqn_prefix[i] = real_spooky_ghost;
	}


	if (game_is_running == true) return;
	game_is_running = true;
    static thread nethack_thread([real_spooky_ghost]()
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

		flags.debug = true;
		//resuming = pcmain(argc, argv);
        display_inventory(nullptr, 0);
		moveloop(0);
		//trololololololololololololololololololo
		delete[] real_spooky_ghost;
	});
	
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

        g_mainpage->map_data[y][x].ch = ch;
        g_mainpage->map_data[y][x].color = CLR_WHITE;
    }
}
void NativeMainPage::display_map()
{
    g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([]() {
        g_mainpage->MapCanvas->Invalidate();
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

void NativeMainPage::clear_map()
{
    g_mainpage->clear_map();
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
        return (char)g_mainpage->Inventory_Strings->GetAt(f_val)->Data()[0];
    return def;
}

char NativeMainPage::ask_direction(char def)
{
    {
        std::lock_guard<std::mutex> lock(g_nativepage_impl.promise_lock);
        g_nativepage_impl.yn_function_promise = std::promise<int>();
        g_nativepage_impl.waiting_for_direction = true;
    }

    g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([]() {
        g_mainpage->Modal_Question = L"In what direction?";
        g_mainpage->Modal_Answers->Clear();

        g_mainpage->PropertyChanged(g_mainpage, ref new PropertyChangedEventArgs("Modal_Question"));

        g_mainpage->modalDialog->Visibility = Windows::UI::Xaml::Visibility::Visible;
    }));

    std::future<int> f = g_nativepage_impl.yn_function_promise.get_future();
    auto x = f.get();
    {
        std::lock_guard<std::mutex> lock(g_nativepage_impl.promise_lock);
        g_nativepage_impl.waiting_for_direction = false;
    }
    if (x == -1)
        return def;
    return static_cast<char>(x);
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
        modalDialog->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
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
    notificationsExpander->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    splitView_History->IsPaneOpen = true;
}


void NethackUWP::MainPage::Button_Close_History_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    splitView_History->IsPaneOpen = false;
}


void NethackUWP::MainPage::OnSizeChanged(Platform::Object ^sender, Windows::UI::Xaml::SizeChangedEventArgs ^e)
{
	float total_size_w = e->NewSize.Width;
	float total_size_h = e->NewSize.Height;
	float out_box_w = MapCanvas->RenderSize.Width;
	float out_box_h = MapCanvas->RenderSize.Height;

	const int DESIRED_MIN_W = 80;
	const int DESIRED_MIN_H = 40;
	const int MINIMUM_FONT = 15; //DPI is hard, alright? Scott can fix. Or I can fix after seeing how it works

	float maximum_font = (out_box_w / DESIRED_MIN_W);

	if (out_box_h / DESIRED_MIN_H < maximum_font) maximum_font = (out_box_h / DESIRED_MIN_H);

    if (maximum_font < MINIMUM_FONT) maximum_font = MINIMUM_FONT;

	//OutputBox->FontSize = maximum_font-1;
	//XXX - ENABLE SCROLLBAR IF CANT REACH 80/40.
	
	


}


//we care about the following keys.
// a-z
// 1-9 for selection
//XXX
// numlock
//space to dismiss messages
//up/down/left/right
//escape\back
//. and ,
// \ ?- = ~ ; '

//then we care about these MODIFIERS:
//shift for capital letters//numbers modifier
//ctrl for ^kick commands

void NethackUWP::MainPage::OnKeyDown(Platform::Object ^sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs ^e)
{
	using Windows::System::VirtualKey;

	e->Handled = true;
	//auto keys = e->OriginalSource;
	auto keys2 = e->Key;
	auto keys3 = e->KeyStatus;
	char key_value = (char) (int) keys2;
	
	bool we_care_about_this_key = false;

		
	if (key_value >= 'A' && key_value <= 'Z') //a-z
	{
		we_care_about_this_key = true;
	}
	if (key_value >= '0' && key_value <= '9')
		we_care_about_this_key = true;

	/* XXX
	if ((keys2 >= VirtualKey::NumberPad0) && (keys2 <= VirtualKey::NumberPad9))
		we_care_about_this_key = true;
	if (keys2 == VirtualKey::Space || keys2 == VirtualKey::Escape || keys2 == VirtualKey::Back)
		we_care_about_this_key = true;
	if (keys2 >= VirtualKey::Left && keys2 <= VirtualKey::Down)
		we_care_about_this_key = true;
*/
	if (we_care_about_this_key == false)
	{
		return;
	}
		key_value = key_value + 32; //lower case a-z

		auto g_corewindow = Windows::UI::Core::CoreWindow::GetForCurrentThread();
//#define M(c) (0x80 | (c))
		bool alt_is_pressed = (CoreVirtualKeyStates::Down == g_corewindow->GetKeyState(Windows::System::VirtualKey::Menu));
//+32
		bool shift_is_pressed = (CoreVirtualKeyStates::Down == g_corewindow->GetKeyState(Windows::System::VirtualKey::Shift));
//#define C(c) (0x1f & (c))
		bool ctrl_is_pressed = (CoreVirtualKeyStates::Down == g_corewindow->GetKeyState(Windows::System::VirtualKey::Control));

		if (alt_is_pressed)
			key_value |= 0x80;
		if (shift_is_pressed)
			key_value -= 32;
		if (ctrl_is_pressed)
			key_value &= 0x1f;

	
		lock_guard<mutex> lock(blocked_on_input);
		if (input_string.empty())
			input_string_cv.notify_all();
		input_string.push_back(key_value);
	

}


void NethackUWP::MainPage::ExpandNotifications(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e)
{
    notificationsExpander->Visibility = Windows::UI::Xaml::Visibility::Visible;
}


void NethackUWP::MainPage::CollapseNotifications(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e)
{
    notificationsExpander->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}

void NethackUWP::MainPage::MapCanvas_Draw(Microsoft::Graphics::Canvas::UI::Xaml::CanvasControl^ sender, Microsoft::Graphics::Canvas::UI::Xaml::CanvasDrawEventArgs^ args)
{
    using namespace Microsoft::Graphics::Canvas::Text;
    using namespace Windows::UI;

    auto tile_size = 15.0f;

    //int start_x = (sender->ActualWidth / tile_size) / 2;
    //int start_y = (sender->ActualHeight / tile_size) / 2;

    //float screen_x_start = (sender->ActualWidth / 2.0f) - (15 - start_x) * tile_size;
    //float screen_y_start = (sender->ActualHeight / 2.0f) - (15 - start_y) * tile_size;

    int start_x = 0;
    int start_y = 0;
    float screen_x_start = 0;
    float screen_y_start = 0;

    auto text_format = ref new CanvasTextFormat();
    text_format->FontFamily = "Consolas";
    text_format->FontSize = 12.0f;
    text_format->HorizontalAlignment = CanvasHorizontalAlignment::Center;
    text_format->VerticalAlignment = CanvasVerticalAlignment::Center;

    std::lock_guard<std::mutex> lock(blocked_on_output);

    int y = start_y;
    for (float screenY = screen_y_start; screenY < sender->ActualHeight; screenY += tile_size)
    {
        if (y >= map_data.size())
            break;

        auto& row_data = map_data[y];

        int x = start_x;
        for (float screenX = screen_x_start; screenX < sender->ActualWidth; screenX += tile_size)
        {
            if (x >= row_data.size())
                break;
            tile_t tile = row_data[x];

            if (tile.ch != L'\0' && tile.ch != L' ')
            {
                static const auto NetHackColorToColor = [](int color) {
                    switch (color)
                    {
                    case 1: // CLR_RED
                        return Colors::Red;
                    case 2: // CLR_GREEN
                        return Colors::Green;
                    case 3: // CLR_BROWN
                        return Colors::Brown;
                    case 4: // CLR_BLUE
                        return Colors::Blue;
                    case 5: // CLR_MAGENTA
                        return Colors::Magenta;
                    case 6: // CLR_CYAN
                        return Colors::Cyan;
                    case 7: // CLR_GRAY
                        return Colors::Gray;
                    case 9: // CLR_ORANGE
                        return Colors::Orange;
                    case 10: // CLR_BRIGHT_GREEN
                        return Colors::LightGreen;
                    case 11: // CLR_YELLOW
                        return Colors::Yellow;
                    case 12: // CLR_BRIGHT_BLUE
                        return Colors::LightBlue;
                    case 13: // CLR_BRIGHT_MAGENTA
                        return Colors::LightPink;
                    case 14: // CLR_BRIGHT_CYAN
                        return Colors::LightCyan;
                    case 15: // CLR_WHITE
                        return Colors::White;
                    default:
                        return Colors::DarkGray;
                    }
                };

                std::string tile_string_ascii(1, tile.ch);
                std::wstring tile_string_wide(tile_string_ascii.begin(), tile_string_ascii.end());

                args->DrawingSession->DrawText(
                    ref new Platform::String(&tile.ch, 1),
                    { screenX, screenY, tile_size, tile_size },
                    NetHackColorToColor(tile.color),
                    text_format);
            }
            x++;
        }
        y++;
    }
}
