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

constexpr auto tile_width = 10.0f;
constexpr auto tile_height = 14.0f;

MainPage^ NethackUWP::g_mainpage;
Platform::Agile<Windows::UI::Core::CoreWindow> NethackUWP::g_corewindow;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

const std::vector<wchar_t*> DEFAULT_KEYS = { L"#",L".",L"z",L"Z",L"o",L"e",L"l",L"d",L"w",L"W",L"s",L">",L"<",L"P",L"t", };

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
        __fastfail(1);

    g_mainpage = this;
    g_corewindow = Windows::UI::Core::CoreWindow::GetForCurrentThread();
    Notifications = ref new Platform::Collections::Vector<Platform::String^>();
    Inventory_Strings = ref new Platform::Collections::Vector<Platform::String^>();
    Last_Notifications = ref new Platform::Collections::Vector<Platform::String^>();
    Modal_Answers = ref new Platform::Collections::Vector<Platform::String^>();

    for (int y = 0; y < NativeMainPage::max_height; ++y)
        map_data.emplace_back(NativeMainPage::max_width_offset, tile_t{ L' ',0 });

    this->DataContext = this;


    for (unsigned int i = 0; i < MAX_BUTTONS; i++)
    {
        Button ^button = ref new Button();
        button->Click += ref new Windows::UI::Xaml::RoutedEventHandler(this, &NethackUWP::MainPage::Quick_Button_Click);

        //button->AddHandler(button_Click, Quick_Button_Click, true);
        if (i < DEFAULT_KEYS.size())
            button->Content = ref new Platform::String(DEFAULT_KEYS[i]);
        else
            button->Content = ref new Platform::String(std::to_wstring(i).c_str());
        button->Margin = Thickness(5, 0, 5, 15);
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
    if (pressed.Y > (MapCanvas->RenderSize.Height*2.0 / 3.0))
        down = true;
    if (pressed.Y < (MapCanvas->RenderSize.Height / 3.0))
        up = true;

    using namespace input_event;
    event_t e;
    e.kind = kind_t::directional;

    //XXX DIAGONALS
    if (up && left)
        e.direction = direction_t::northwest;
    else if (up && right)
        e.direction = direction_t::northeast;
    else if (down && left)
        e.direction = direction_t::southwest;
    else if (down && right)
        e.direction = direction_t::southeast;
    else if (left)
        e.direction = direction_t::west;
    else if (up)
        e.direction = direction_t::north;
    else if (right)
        e.direction = direction_t::east;
    else if (down)
        e.direction = direction_t::south;
    else
        e.direction = direction_t::self;

    NativeMainPage::event_queue.enqueue(e);
}


void NethackUWP::MainPage::button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
#ifndef NDEBUG
    flags.debug = true;
#endif
    wchar_t* spooky_ghost_folder = (wchar_t*)Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data();

    char* spooky_ghost = new char[1024];
    char* real_spooky_ghost = spooky_ghost;

    while (*spooky_ghost_folder)
        *spooky_ghost++ = (char)*spooky_ghost_folder++;
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
        std::unique_ptr<char[]> delete_real_spooky_ghost(real_spooky_ghost);

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

#ifndef NDEBUG
		flags.debug = true;
#endif
        //resuming = pcmain(argc, argv);
        display_inventory(nullptr, 0);
        moveloop(0);
        //trololololololololololololololololololo
    });

}

void NethackUWP::MainPage::Quick_Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args)
{
    auto nethack_text = ((Platform::String^)((Button^)sender)->Content);
    if (nethack_text->IsEmpty())
        return;

    using namespace input_event;
    event_t e;
    e.kind = kind_t::keyboard;
    e.key = (char)nethack_text->Data()[0];

    NativeMainPage::event_queue.enqueue(e);
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
        if (idx == -1)
            return;

        using namespace input_event;

        event_t e;
        e.kind = kind_t::select_menu_item;
        e.menu_index = idx;
        NativeMainPage::event_queue.enqueue(e);
    }
}


void NethackUWP::MainPage::SymbolIcon_Tapped(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ args)
{
    // Dismiss modal dialog
    using namespace input_event;

    event_t e;
    e.kind = kind_t::cancel_menu_button;
    NativeMainPage::event_queue.enqueue(e);
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

	int actual_x_count = 0;
	int actual_y_count = 0;

	if (out_box_w / tile_width < DESIRED_MIN_W)  // we need to scroll 
		actual_x_count = DESIRED_MIN_W;
	else
		actual_x_count = (int)out_box_w / tile_width;

	if (out_box_h / tile_height < DESIRED_MIN_H)  // we need to scroll 
		actual_y_count = DESIRED_MIN_H;
	else
		actual_y_count = (int)out_box_h / tile_height;

}

static const char
/* normal, shift, control */
keypad[][3] =
{
    {'0','0','0'},
    { 'b', 'B', C('b') },    /* NumberPad1 */
    { 'j', 'J', C('j') },    /* 2 */
    { 'n', 'N', C('n') },    /* 3 */
    { 'h', 'H', C('h') },    /* 4 */
    { 'g', 'G', 'g' },       /* 5 */
    { 'l', 'L', C('l') },    /* 6 */
    { 'y', 'Y', C('y') },    /* 7 */
    { 'k', 'K', C('k') },    /* 8 */
    { 'u', 'U', C('u') },    /* 9 */
    {'*','*','*'},
    { '+', 'P', C('p') },    /* + */
    {'\n', '\n', '\n' },    //ENTER??? 
    { 'm', C('p'), C('p') }, /* - */
    {'.', '.','.'}, //dot
    {'//','//','//'} //divide

    //{ 'i', 'I', C('i') },    /* Ins */
    //{ '.', ':', ':' }        /* Del */
},
numpad[][3] = {   //XXX if numpad option is DISABLED.
    { '7', M('7'), '7' },    /* 7 */
    { '8', M('8'), '8' },    /* 8 */
    { '9', M('9'), '9' },    /* 9 */
    { 'm', C('p'), C('p') }, /* - */
    { '4', M('4'), '4' },    /* 4 */
    { '5', M('5'), '5' },    /* 5 */
    { '6', M('6'), '6' },    /* 6 */
    { '+', 'P', C('p') },    /* + */
    { '1', M('1'), '1' },    /* 1 */
    { '2', M('2'), '2' },    /* 2 */
    { '3', M('3'), '3' },    /* 3 */
    { '0', M('0'), '0' },    /* Ins */
    { '.', ':', ':' }        /* Del */
};

//we care about the following keys.
// a-z
// 1-9 for selection
// numlock
//space to dismiss messages
//up/down/left/right
//escape\back

//. and ,
// \ ?- = ~ ; '
//> <
//XXX
//then we care about these MODIFIERS:
//shift for capital letters//numbers modifier
//ctrl for ^kick commands

static direction_t key_to_direction(char direction)
{
    direction_t return_direction;
    switch (direction)
    {
    case 'y': return_direction = direction_t::northwest; break;
    case 'k': return_direction = direction_t::north; break;
    case 'u': return_direction = direction_t::northeast; break;
    case 'h': return_direction = direction_t::west; break;
    case 'l': return_direction = direction_t::east; break;
    case 'b': return_direction = direction_t::southwest; break;
    case 'j': return_direction = direction_t::south; break;
    case 'n': return_direction = direction_t::southeast; break;
    case '.': return_direction = direction_t::self; break;
    case 's': return_direction = direction_t::self; break;
    case '<': return_direction = direction_t::up; break;
    case '>': return_direction = direction_t::down; break;
    default:
        __fastfail(555);
   }
    return return_direction;
}

void NethackUWP::MainPage::OnKeyDown(Platform::Object ^sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs ^e)
{
    using Windows::System::VirtualKey;
    using Windows::UI::Core::CoreVirtualKeyStates;
    using Windows::UI::Xaml::FocusState;
    using namespace input_event;


    if (InputBox->FocusState != FocusState::Unfocused)
        return;

    auto g_corewindow = Windows::UI::Core::CoreWindow::GetForCurrentThread();
    //#define M(c) (0x80 | (c))
    bool alt_is_pressed = (CoreVirtualKeyStates::Down == g_corewindow->GetKeyState(Windows::System::VirtualKey::Menu));
    //+32
    bool shift_is_pressed = (CoreVirtualKeyStates::Down == g_corewindow->GetKeyState(Windows::System::VirtualKey::Shift));
    //#define C(c) (0x1f & (c))
    bool ctrl_is_pressed = (CoreVirtualKeyStates::Down == g_corewindow->GetKeyState(Windows::System::VirtualKey::Control));


    e->Handled = false;
    //auto keys = e->OriginalSource;
    auto keys2 = e->Key;
    auto keys3 = e->KeyStatus;
    char key_value = (char)(int)keys2;

    bool we_care_about_this_key = false;
    bool modifiers_handled = false;
    bool directionals = false;

    if (keys2 >= VirtualKey::A && keys2 <= VirtualKey::Z) //a-z
    {
        key_value = key_value + 32; //lower case a-z
        we_care_about_this_key = true;
    }
    if (keys2 >= VirtualKey::Number0 && keys2 <= VirtualKey::Number9)
        we_care_about_this_key = true;


    if ((keys2 >= VirtualKey::NumberPad0) && (keys2 <= VirtualKey::Divide))
    {
        int index_array = (shift_is_pressed * 1) | (ctrl_is_pressed * 2);
        key_value=keypad[key_value - (char)VirtualKey::NumberPad0][index_array];
        we_care_about_this_key = true;
        modifiers_handled = true;
        if (keys2 >= VirtualKey::NumberPad0  && keys2 <= VirtualKey::NumberPad9)
            if (keys2 != VirtualKey::NumberPad5)
                if ((alt_is_pressed || shift_is_pressed || ctrl_is_pressed) == false)
                    directionals = true;
    }           
    
    if (keys2 == VirtualKey::Escape || keys2 == VirtualKey::Back || keys2 == VirtualKey::GoBack || keys2 == VirtualKey::Space)
    {
        event_t ET;
        ET.kind = kind_t::cancel_menu_button;
         NativeMainPage::event_queue.enqueue(ET);
        we_care_about_this_key = true;
        return;
    }

    if (keys2 >= VirtualKey::Left && keys2 <= VirtualKey::Down)
    {
        we_care_about_this_key = true;
        directionals = true;
        switch (keys2)
        {
        case VirtualKey::Left: key_value = 'h'; break;
        case VirtualKey::Right: key_value = 'l'; break;
        case VirtualKey::Up: key_value = 'k'; break;
        case VirtualKey::Down: key_value = 'j'; break;
        default:
            __fastfail(5);
            
        }
    }

    if ((int)keys2 >= VK_OEM_1 && (int)keys2 <= VK_OEM_7) // don't handkle 
    {
        we_care_about_this_key = true;
        modifiers_handled = true;

        constexpr const   static char keys_no_shift[] = ";+,-./`[\\]'";
        constexpr const   static char keys_shift[] = ":=<_>?~{|}\"";
        int iter_arr = (int)keys2 - VK_OEM_1;
        assert(iter_arr >= 0 && iter_arr <= 11);
        if (shift_is_pressed)
            key_value = keys_no_shift[iter_arr];
        else
            key_value = keys_shift[iter_arr];
        
    }

    if (we_care_about_this_key == false)
    {
        return;
    }
 

    if (modifiers_handled == false)
    {
        if (alt_is_pressed)
            key_value |= 0x80;
        if (shift_is_pressed)
            key_value -= 32;
        if (ctrl_is_pressed)
            key_value &= 0x1f;
    }

    event_t ET;
    if (directionals)
    {
        ET.kind = kind_t::directional;
        ET.direction = key_to_direction(key_value);

    }
    else
    {
        ET.kind = kind_t::keyboard;
        ET.key = key_value;
    }
    NativeMainPage::event_queue.enqueue(ET);
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

    //int start_x = (sender->ActualWidth / tile_size) / 2;
    //int start_y = (sender->ActualHeight / tile_size) / 2;

    //float screen_x_start = (sender->ActualWidth / 2.0f) - (15 - start_x) * tile_size;
    //float screen_y_start = (sender->ActualHeight / 2.0f) - (15 - start_y) * tile_size;

    int start_x = u.ux - sender->ActualWidth / tile_width / 2;
    int start_y = u.uy - sender->ActualHeight / tile_height / 2;
    float screen_x_start = 0; //(sender->ActualWidth / 2.0f) - (u.ux - start_x) * tile_width;
    float screen_y_start = 0; //(sender->ActualHeight / 2.0f) - (u.uy - start_y) * tile_height;

    auto text_format = ref new CanvasTextFormat();
    text_format->FontFamily = "Consolas";
    text_format->FontSize = 16.0f;
    text_format->HorizontalAlignment = CanvasHorizontalAlignment::Center;
    text_format->VerticalAlignment = CanvasVerticalAlignment::Center;

    std::lock_guard<std::mutex> lock(blocked_on_output);

    int y = start_y;
    for (float screenY = screen_y_start; screenY < sender->ActualHeight; screenY += tile_height, ++y)
    {
        if (y < 0)
            continue;
        if (y >= map_data.size())
            break;

        auto& row_data = map_data[y];

        int x = start_x;
        for (float screenX = screen_x_start; screenX < sender->ActualWidth; screenX += tile_width, ++x)
        {
            if (x < 0)
                continue;
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

                args->DrawingSession->DrawText(
                    ref new Platform::String(&tile.ch, 1),
                    { screenX, screenY, tile_width, tile_height },
                    NetHackColorToColor(tile.color),
                    text_format);
            }
        }
    }
}
