/**
 * This code is responsible for creating, clearing, and destroying modal
 * windows used throughout the installation wizard interface.
 */

#include "../all.h"

WINDOW *create_modal(const char *title)
{
    int screen_height, screen_width;
    getmaxyx(stdscr, screen_height, screen_width);

    // Calculate centered position on screen.
    int start_y = (screen_height - MODAL_HEIGHT) / 2;
    int start_x = (screen_width - MODAL_WIDTH) / 2;

    // Create window with background color.
    WINDOW *window = newwin(MODAL_HEIGHT, MODAL_WIDTH, start_y, start_x);
    wbkgd(window, COLOR_PAIR(COLOR_PAIR_MAIN));

    // Draw full-width title bar with lighter background.
    wattron(window, COLOR_PAIR(COLOR_PAIR_ROW));
    mvwprintw(window, 0, 0, "%*s", MODAL_WIDTH, "");
    int title_x = (MODAL_WIDTH - strlen(title)) / 2;
    mvwprintw(window, 0, title_x, "%s", title);
    wattroff(window, COLOR_PAIR(COLOR_PAIR_ROW));

    wrefresh(window);

    return window;
}

void clear_modal(WINDOW *modal)
{
    // Clear content area with spaces, preserving the title bar.
    for (int row = 1; row < MODAL_HEIGHT; row++)
    {
        mvwprintw(modal, row, 0, "%*s", MODAL_WIDTH, "");
    }
}

void destroy_modal(WINDOW *modal)
{
    delwin(modal);
}
