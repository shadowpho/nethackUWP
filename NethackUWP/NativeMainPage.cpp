#include "pch.h"
#include "../NativeMainPage.h"
#include "MainPage.xaml.h"

#include <cassert>

using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Core;
using namespace NethackUWP;

struct ThreadSafeQueue<struct input_event::event_t> NativeMainPage::event_queue;

static Platform::String^ cstr_to_platstr(const char* str)
{
    if (str == nullptr)
        return nullptr;

    // TODO: fix to handle UTF-8 instead of Latin-1
    std::wstring strbuf;
    while (*str) { strbuf.push_back(*str); ++str; }

    Platform::String^ pcstr = ref new Platform::String(strbuf.c_str(), (unsigned int)strbuf.size());
    return pcstr;
}

static char to_keyboard_key(direction_t direction)
{
    static constexpr char dirmap[] = "kjlhuynb<>.";
    auto idx = std::underlying_type_t<direction_t>(direction);
    assert(idx < sizeof(dirmap));

    return dirmap[idx];
}




int NativeMainPage::read_char(int &x, int &y)
{
    using namespace input_event;

    event_t e;
    while (true)
    {
        event_queue.dequeue(e);

        switch (e.kind)
        {
        case kind_t::directional:
            return to_keyboard_key(e.direction);
        case kind_t::keyboard:
            return e.key;
        case kind_t::tap:
            // TODO: add mouse/tap support.
            break;
        }
    }
}

void NativeMainPage::write_char(int x, int y, char ch, int color)
{
    {
        std::lock_guard<std::mutex> lock(g_mainpage->blocked_on_output);
        if (x >= max_width) abort();
        if (y >= max_height) abort();

        g_mainpage->map_data[y][x].ch = ch;
        g_mainpage->map_data[y][x].color = color;
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
    if (str == 0 || *str == 0 || !strcmp(str, ""))
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
    using namespace input_event;

    event_queue.clear();

    Platform::String^ psQuestion = cstr_to_platstr(question);
    g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([psQuestion]() {
        g_mainpage->Modal_Question = psQuestion;
        g_mainpage->Modal_Answers->Clear();

        for (auto&& inv : g_mainpage->Inventory_Strings)
            g_mainpage->Modal_Answers->Append(inv);

        g_mainpage->PropertyChanged(g_mainpage, ref new PropertyChangedEventArgs("Modal_Question"));
        g_mainpage->modalDialog->Visibility = Windows::UI::Xaml::Visibility::Visible;
    }));

    event_t e;
    while (true)
    {
        event_queue.dequeue(e);
        switch (e.kind)
        {
        case kind_t::cancel_menu_button:
            g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([]() {
                g_mainpage->modalDialog->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
            }));
            return def;
        case kind_t::select_menu_item:
            g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([]() {
                g_mainpage->modalDialog->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
            }));
            return (char)g_mainpage->Inventory_Strings->GetAt(e.menu_index)->Data()[0];
        case kind_t::keyboard:
            g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([]() {
                g_mainpage->modalDialog->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
            }));
            return e.key;
        default:
            break;
        }
    }
}

char NativeMainPage::ask_direction(char def)
{
    using namespace input_event;

    event_queue.clear();

    g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([]() {
        g_mainpage->Modal_Question = L"In what direction?";
        g_mainpage->Modal_Answers->Clear();

        g_mainpage->PropertyChanged(g_mainpage, ref new PropertyChangedEventArgs("Modal_Question"));

        g_mainpage->modalDialog->Visibility = Windows::UI::Xaml::Visibility::Visible;
    }));

    event_t e;
    while (true)
    {
        event_queue.dequeue(e);
        switch (e.kind)
        {
        case kind_t::cancel_menu_button:
            g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([]() {
                g_mainpage->modalDialog->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
            }));
            return def;
        case kind_t::directional:
            g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([]() {
                g_mainpage->modalDialog->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
            }));
            return to_keyboard_key(e.direction);
        case kind_t::keyboard:
            g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([]() {
                g_mainpage->modalDialog->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
            }));
            return e.key;
        default:
            break;
        }
    }
}

char NativeMainPage::ask_yn_function(const char *question, const char *choices, char def)
{
    using namespace input_event;

    event_queue.clear();

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

    event_t e;
    while (true)
    {
        event_queue.dequeue(e);
        switch (e.kind)
        {
        case kind_t::cancel_menu_button:
            g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([]() {
                g_mainpage->modalDialog->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
            }));
            return def;
        case kind_t::select_menu_item:
            g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([]() {
                g_mainpage->modalDialog->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
            }));
            return choices[e.menu_index];
        case kind_t::keyboard:
            g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([]() {
                g_mainpage->modalDialog->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
            }));
            return e.key;
        default:
            break;
        }
    }
}
void NativeMainPage::complete_yn_function(int idx)
{
    using namespace input_event;
    event_t e;
    e.kind = kind_t::select_menu_item;
    e.menu_index = idx;

    event_queue.enqueue(e);
}
bool NativeMainPage::ask_menu(const menu_t& m, int& selection_value)
{
    using namespace input_event;

    event_queue.clear();

    Platform::String^ psQuestion = cstr_to_platstr(m.prompt.c_str());

    g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([psQuestion, &m]() {
        g_mainpage->Modal_Question = psQuestion;
        g_mainpage->Modal_Answers->Clear();

        for (auto&& s : m.choices)
            g_mainpage->Modal_Answers->Append(cstr_to_platstr(s.str.c_str()));

        g_mainpage->PropertyChanged(g_mainpage, ref new PropertyChangedEventArgs("Modal_Question"));

        g_mainpage->modalDialog->Visibility = Windows::UI::Xaml::Visibility::Visible;
    }));


    event_t e;
    while (true)
    {
        event_queue.dequeue(e);
        switch (e.kind)
        {
        case kind_t::cancel_menu_button:
            g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([]() {
                g_mainpage->modalDialog->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
            }));
            return true;
        case kind_t::select_menu_item:
            if (e.menu_index == -1)
            {

            }
            if (m.choices[e.menu_index].selectable)
            {
                g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([]() {
                    g_mainpage->modalDialog->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
                }));
                selection_value = m.choices[e.menu_index].value;
                return false;
            }
            break;
        case kind_t::keyboard:
            for (auto&& choice : m.choices)
            {
                if (choice.selectable && choice.accel == e.key)
                {
                    g_corewindow->Dispatcher->RunAsync(CoreDispatcherPriority::Low, ref new DispatchedHandler([]() {
                        g_mainpage->modalDialog->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
                    }));
                    selection_value = choice.value;
                    return false;
                }
            }
            return true;
        default:
            break;
        }
    }
}
