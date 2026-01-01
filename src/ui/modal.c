/**
 * This code is responsible for creating and managing modal dialog windows
 * within the ncurses interface.
 */

#include "../all.h"

WINDOW *create_modal(const char *title)
{
    int screen_height, screen_width;
    getmaxyx(stdscr, screen_height, screen_width);

    // Calculate centered position on screen.
    int start_y = (screen_height - MODAL_HEIGHT) / 2;
    int start_x = (screen_width - MODAL_WIDTH) / 2;

    // Create window with border and background color.
    WINDOW *window = newwin(MODAL_HEIGHT, MODAL_WIDTH, start_y, start_x);
    wbkgd(window, COLOR_PAIR(UI_COLOR_MAIN));
    box(window, 0, 0);

    // Enable arrow key detection on this window.
    keypad(window, TRUE);

    // Draw title centered at top with bracket decoration.
    int title_length = strlen(title);
    int title_x = (MODAL_WIDTH - title_length - 4) / 2;
    mvwprintw(window, 0, title_x, "[ %s ]", title);
    wrefresh(window);

    return window;
}

void clear_modal(WINDOW *modal)
{
    // Clear content area with spaces, preserving the border.
    for (int row = 1; row < MODAL_HEIGHT - 1; row++) {
        mvwprintw(modal, row, 1, "%*s", MODAL_WIDTH - 2, "");
    }
}

void destroy_modal(WINDOW *modal)
{
    delwin(modal);
}
