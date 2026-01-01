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

        // Define off-white background for a softer appearance.
        if (can_change_color()) {
            init_color(COLOR_WHITE, 900, 900, 880);
            // Define a darker background for table rows (odd).
            init_color(9, 820, 820, 800);
            // Define an even darker background for table header.
            init_color(10, 700, 700, 680);
            // Define a lighter background for table rows (even).
            init_color(11, 860, 860, 840);
            // Define dodger blue for selected indicator.
            init_color(12, 118, 565, 1000);
            // Define orange for warning text.
            init_color(13, 900, 400, 0);
        }

        init_pair(1, COLOR_BLACK, COLOR_WHITE);
        // Color pair for table rows odd (darker background).
        init_pair(2, COLOR_BLACK, 9);
        // Color pair for dimmed/gray text.
        init_pair(3, 8, COLOR_WHITE); 
        // Color pair for table header (darker than rows).
        init_pair(4, COLOR_BLACK, 10);
        // Color pair for table rows even (lighter background).
        init_pair(5, COLOR_BLACK, 11);
        // Color pair for dodger blue selected indicator.
        init_pair(6, 12, COLOR_WHITE);
        // Color pair for warning text (orange).
        init_pair(7, 13, COLOR_WHITE);
    }

    // Refresh to apply initial screen state.
    refresh();
}

void cleanup_ui(void)
{
    endwin();
}

void render_footer(WINDOW *modal, const char **items)
{
    int x = 3;
    for (int item_index = 0; items[item_index] != NULL; item_index++) {
        if (item_index > 0) {
            x += 2; // 2 space gap between items.
        }

        const char *cursor = items[item_index];
        while (*cursor) {
            if (*cursor == '[') {
                // Find closing bracket and render in bold.
                const char *end = strchr(cursor, ']');
                if (end != NULL) {
                    int length = end - cursor + 1;
                    wattron(modal, A_BOLD);
                    mvwprintw(modal, MODAL_HEIGHT - 2, x, "%.*s", length, cursor);
                    wattroff(modal, A_BOLD);
                    x += length;
                    cursor = end + 1;
                } else {
                    // No closing bracket, render rest in gray.
                    wattron(modal, COLOR_PAIR(UI_COLOR_DIM));
                    mvwprintw(modal, MODAL_HEIGHT - 2, x, "%s", cursor);
                    wattroff(modal, COLOR_PAIR(UI_COLOR_DIM));
                    x += strlen(cursor);
                    break;
                }
            } else {
                // Find next '[' or end of string, render in gray.
                const char *next = strchr(cursor, '[');
                int length = next ? (next - cursor) : (int)strlen(cursor);
                wattron(modal, COLOR_PAIR(UI_COLOR_DIM));
                mvwprintw(modal, MODAL_HEIGHT - 2, x, "%.*s", length, cursor);
                wattroff(modal, COLOR_PAIR(UI_COLOR_DIM));
                x += length;
                cursor += length;
            }
        }
    }
}
