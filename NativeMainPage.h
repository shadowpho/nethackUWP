#pragma once

#include <string>
#include <vector>
#include <deque>
#include <mutex>
#include <condition_variable>

struct choice_t
{
    /// Accelerator is a keyboard key that can be used
    /// to select the line.If the accelerator of a selectable
    /// item is 0, the window system is free to select its own
    /// accelerator.
    char accel;

    bool selectable;

    int value;
    /// The value attr is the same as in putstr().
    int attr;
    /// If preselected is true, this choice is to be preselected when the
    /// menu is displayed
    bool preselected;
    /// string for selection or header
    std::string str;
};
struct menu_t
{
    std::string prompt;
    std::vector<choice_t> choices;
};

struct tile_t
{
    wchar_t ch;
    int color;
};



enum class direction_t
{
    north,
    south,
    east,
    west,
    northeast,
    northwest,
    southeast,
    southwest,
    up,
    down,
    self,
    COUNT
};

enum class command_t
{
    // scott plz help
};

namespace input_event
{
    enum class kind_t
    {
        directional,
        command,
        keyboard,
        cancel_menu_button,
        tap,
        select_menu_item,
        extended_command,
        state_transition
    };

    enum class transition_t
    {
        to_extended_command
    };

    struct event_t
    {
        kind_t kind;

        union
        {
            char key;
            int menu_index;
            // X/Y are on a scale from 0.0 to 1.0
            struct
            {
                double x, y;
            };
            direction_t direction;
            transition_t transition;
        };
    };
}


template<class T>
struct ThreadSafeQueue
{
    template<class U>
    void enqueue(U&& in)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty())
            m_cv.notify_one();
        m_queue.emplace_back(std::forward<U>(in));
    }
    void dequeue(T& out)
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        while (m_queue.empty())
            m_cv.wait(lock);

        out = std::move(m_queue.front());
        m_queue.pop_front();
    }
    bool empty()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }
    void clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.clear();
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::deque<T> m_queue;
};

struct NativeMainPage {
    static constexpr int max_width = 100;
    static constexpr int max_width_offset = max_width + 1;
    static constexpr int max_height = 80;

    // Input functions -- blocking.
    static int read_char(int &x, int &y);

    // Output functions -- non-blocking.
    static void write_char(int x, int y, char ch, int color = 15);
    static void write_notification(const char*);
    static void clear_notifications();
    static void update_statusbar(const char*);
    static void clear_statusbar();
    static void clear_map();
    static void display_map();
    static void clear_inv();
    static void add_inv_str(const char* str, bool is_header, int attr, char accelerator);
    static void enqueue_ext_cmd(const char* cmdname);

    // Input/output functions -- blocking.
    static bool ask_menu(const menu_t& m, int& selection_value);
    static char ask_inv_function(const char* question, char def);
    static char ask_direction(char def);
    static char ask_yn_function(const char *question, const char *choices, char def);
    static int ask_extcmdlist();

    // Utility function.
    static void complete_yn_function(int idx);

    // Primary input queue.
    static ThreadSafeQueue<input_event::event_t> event_queue;
};
