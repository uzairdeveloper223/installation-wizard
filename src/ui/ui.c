/**
 * This code is responsible for initializing and cleaning up the ncurses
 * terminal UI library.
 */

#include "../all.h"

void init_ui(void)
{
    // Initialize ncurses screen and disable input buffering.
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    // Reduce Escape key delay to avoid double-press issue.
    set_escdelay(25);

    // Configure color pairs if terminal supports colors.
    if (has_colors()) {
        start_color();
        use_default_colors();

        // Define custom colors if supported.
        if (can_change_color()) {
            init_color(COLOR_WHITE,                900, 900, 880);
            init_color(CUSTOM_COLOR_ROW_ODD_BG,    820, 820, 800);
            init_color(CUSTOM_COLOR_HEADER_BG,     700, 700, 680);
            init_color(CUSTOM_COLOR_ROW_EVEN_BG,   860, 860, 840);
            init_color(CUSTOM_COLOR_BLUE,          118, 565, 1000);
            init_color(CUSTOM_COLOR_ORANGE,        900, 400, 0);
            init_color(CUSTOM_COLOR_RED,           900, 200, 200);
        }

        // Initialize color pairs.
        init_pair(CUSTOM_COLOR_PAIR_MAIN, COLOR_BLACK, COLOR_WHITE);
        init_pair(CUSTOM_COLOR_PAIR_ROW_ODD, COLOR_BLACK, CUSTOM_COLOR_ROW_ODD_BG);
        init_pair(CUSTOM_COLOR_PAIR_DIM, CUSTOM_COLOR_DARK_GRAY, COLOR_WHITE);
        init_pair(CUSTOM_COLOR_PAIR_HEADER, COLOR_BLACK, CUSTOM_COLOR_HEADER_BG);
        init_pair(CUSTOM_COLOR_PAIR_ROW_EVEN, COLOR_BLACK, CUSTOM_COLOR_ROW_EVEN_BG);
        init_pair(CUSTOM_COLOR_PAIR_SELECTED, CUSTOM_COLOR_BLUE, COLOR_WHITE);
        init_pair(CUSTOM_COLOR_PAIR_NOTE_BG, COLOR_BLACK, CUSTOM_COLOR_ROW_EVEN_BG);
        init_pair(CUSTOM_COLOR_PAIR_NOTE_TEXT, COLOR_BLACK, CUSTOM_COLOR_ROW_EVEN_BG);
        init_pair(CUSTOM_COLOR_PAIR_INFO_NOTE, CUSTOM_COLOR_BLUE, CUSTOM_COLOR_ROW_EVEN_BG);
        init_pair(CUSTOM_COLOR_PAIR_WARNING_NOTE, CUSTOM_COLOR_ORANGE, CUSTOM_COLOR_ROW_EVEN_BG);
        init_pair(CUSTOM_COLOR_PAIR_ERROR_NOTE, CUSTOM_COLOR_RED, CUSTOM_COLOR_ROW_EVEN_BG);
    }

    // Refresh to apply initial screen state.
    refresh();
}

void cleanup_ui(void)
{
    endwin();
}
