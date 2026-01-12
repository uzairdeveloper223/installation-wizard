#include "../all.h"

/** Tracks whether terminal supports extended colors (16+). */
static int use_extended_colors = 0;

int ui_has_extended_colors(void)
{
    return use_extended_colors;
}

void initialize_ui(void)
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
    if (has_colors())
    {
        start_color();
        use_default_colors();

        // Check if terminal supports extended colors (16+).
        use_extended_colors = can_change_color() && (COLORS >= 16);

        if (use_extended_colors)
        {
            // Define custom colors for terminals with full color support.
            init_color(COLOR_WHITE,                900, 900, 880);
            init_color(CUSTOM_COLOR_ROW_ODD_BG,    820, 820, 800);
            init_color(CUSTOM_COLOR_HEADER_BG,     700, 700, 680);
            init_color(CUSTOM_COLOR_ROW_EVEN_BG,   860, 860, 840);
            init_color(CUSTOM_COLOR_BLUE,          118, 565, 1000);
            init_color(CUSTOM_COLOR_ORANGE,        900, 400, 0);
            init_color(CUSTOM_COLOR_RED,           900, 200, 200);
            init_color(CUSTOM_COLOR_PURPLE,        600, 300, 900);

            // Initialize color pairs with custom colors.
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
            init_pair(CUSTOM_COLOR_PAIR_PRIMARY_ODD, CUSTOM_COLOR_PURPLE, CUSTOM_COLOR_ROW_ODD_BG);
            init_pair(CUSTOM_COLOR_PAIR_PRIMARY_EVEN, CUSTOM_COLOR_PURPLE, CUSTOM_COLOR_ROW_EVEN_BG);
        }
        else
        {
            // 8-color fallback for basic terminals (xterm, Linux console, etc).
            init_pair(CUSTOM_COLOR_PAIR_MAIN, COLOR_BLACK, COLOR_WHITE);
            init_pair(CUSTOM_COLOR_PAIR_ROW_ODD, COLOR_BLACK, COLOR_WHITE);
            init_pair(CUSTOM_COLOR_PAIR_DIM, COLOR_BLACK, COLOR_WHITE);
            init_pair(CUSTOM_COLOR_PAIR_HEADER, COLOR_WHITE, COLOR_BLACK);
            init_pair(CUSTOM_COLOR_PAIR_ROW_EVEN, COLOR_BLACK, COLOR_WHITE);
            init_pair(CUSTOM_COLOR_PAIR_SELECTED, COLOR_BLUE, COLOR_WHITE);
            init_pair(CUSTOM_COLOR_PAIR_NOTE_BG, COLOR_BLACK, COLOR_WHITE);
            init_pair(CUSTOM_COLOR_PAIR_NOTE_TEXT, COLOR_BLACK, COLOR_WHITE);
            init_pair(CUSTOM_COLOR_PAIR_INFO_NOTE, COLOR_BLUE, COLOR_WHITE);
            init_pair(CUSTOM_COLOR_PAIR_WARNING_NOTE, COLOR_YELLOW, COLOR_WHITE);
            init_pair(CUSTOM_COLOR_PAIR_ERROR_NOTE, COLOR_RED, COLOR_WHITE);
            init_pair(CUSTOM_COLOR_PAIR_PRIMARY_ODD, COLOR_MAGENTA, COLOR_WHITE);
            init_pair(CUSTOM_COLOR_PAIR_PRIMARY_EVEN, COLOR_MAGENTA, COLOR_WHITE);
        }
    }

    // Refresh to apply initial screen state.
    refresh();
}

void cleanup_ui(void)
{
    endwin();
}
