#pragma once

struct NativeMainPage {
    static constexpr int max_width = 100;
    static constexpr int max_width_offset = max_width + 1;
    static constexpr int max_height = 80;

    static int read_char(int &x, int &y);
    static void write_char(int x, int y, char ch);

    static void write_notification(const char*);
    static void update_statusbar(const char*);
    static void clear_statusbar();

    static void clear_inv();
    static void add_inv_str(const char* str, bool is_header, int attr, char accelerator);

    static char ask_yn_function(const char *question, const char *choices, char def);
    static void complete_yn_function(char c);
};
