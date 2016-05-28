
#include "pch.h"

#include "..\NethackNative.h"

#define boolean boolean2
#define terminate terminate2

extern "C" {
    #include "hack.h"
    #include "dlb.h"
}

#undef terminate
#undef boolean

#include <thread>

struct
{
    std::mutex lock;
    tile_t tiles[NethackNative::max_width][NethackNative::max_height];
} g_map;

struct
{
    std::mutex lock;
    point_t position;
} g_player;

void NethackNative::start_nethack()
{
    static std::thread nethack_thread([]()
    {
        sys_early_init();
        choose_windows("mswin");

        initoptions();
        dlb_init();
        init_nhwindows(0, 0);
        vision_init();
        display_gamewindows();
        newgame();

        flags.perm_invent = true;
        flags.debug = true;

        display_inventory(nullptr, 0);
        moveloop(0);
    });
}

int NethackNative::read_char(int & x, int & y)
{
    return 0;
}

void NethackNative::put_tile(int x, int y, tile_t& tile)
{
    if (x < 0 || x > max_width || y < 0 || y > max_height)
        return;
    
    std::lock_guard<std::mutex> lock(g_map.lock);
    g_map.tiles[x][y] = tile;

    if (tile.ch == '@')
    {
        std::lock_guard<std::mutex> lock(g_player.lock);
        g_player.position.x = x;
        g_player.position.y = y;
    }
}

tile_t NethackNative::get_tile(int x, int y)
{
    if (x < 0 || x > max_width || y < 0 || y > max_height)
        return { ' ', 0 };

    std::lock_guard<std::mutex> lock(g_map.lock);
    return g_map.tiles[x][y];
}

point_t NethackNative::get_player_position()
{
    std::lock_guard<std::mutex> lock(g_player.lock);
    return g_player.position;
}

void NethackNative::write_notification(const char *)
{
    //
}

void NethackNative::clear_notifications()
{
    //
}

void NethackNative::update_statusbar(const char *)
{
    //
}

void NethackNative::clear_statusbar()
{
    //
}

void NethackNative::clear_map()
{
    //
}

void NethackNative::display_map()
{
    //
}

void NethackNative::clear_inv()
{
    //
}

void NethackNative::add_inv_str(const char * str, bool is_header, int attr, char accelerator)
{
    //
}

bool NethackNative::ask_menu(const menu_t & m, int & selection_value)
{
    return false;
}

char NethackNative::ask_inv_function(const char * question, char def)
{
    return 0;
}

char NethackNative::ask_direction(char def)
{
    return 0;
}

char NethackNative::ask_yn_function(const char * question, const char * choices, char def)
{
    return 0;
}
