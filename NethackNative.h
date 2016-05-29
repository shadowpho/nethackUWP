#pragma once

#include <string>
#include <vector>
#include <mutex>

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
    char ch;
    int color;
};

struct point_t
{
    int x;
    int y;
};

struct NethackNative {
    static constexpr int max_width = 100;
    static constexpr int max_width_offset = max_width + 1;
    static constexpr int max_height = 80;

    static void start_nethack();

    static int read_char(int &x, int &y);
    static void put_tile(int x, int y, tile_t& tile);
    static tile_t get_tile(int x, int y);

    static void put_player_position(int x, int y);
    static point_t get_player_position();

    static void write_notification(const char*);
    static void clear_notifications();

    static void update_statusbar(const char*);
    static void clear_statusbar();

    static void display_map();
    static void clear_map();

    static void add_inv_str(const char* str, bool is_header, int attr, char accelerator);
    static void clear_inv();

    static bool ask_menu(const menu_t& m, int& selection_value);
    static char ask_inv_function(const char* question, char def);
    static char ask_direction(char def);
    static char ask_yn_function(const char *question, const char *choices, char def);
};
