/**
 * This code is responsible for initializing and cleaning up ncurses.
 */

#include "../all.h"

void initialize_ui(void)
{
    // Set custom palette before ncurses takes over the terminal.
    init_colors_palette();

    // Initialize ncurses with input handling and hidden cursor.
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    set_escdelay(25);

    // Initialize color pairs if terminal supports colors.
    if (has_colors())
    {
        start_color();
        use_default_colors();
        init_colors_pairs();
        bkgd(COLOR_PAIR(COLOR_PAIR_SCREEN_BG));
    }

    // Apply initial screen state.
    refresh();
}

void cleanup_ui(void)
{
    endwin();
    cleanup_colors();
}
