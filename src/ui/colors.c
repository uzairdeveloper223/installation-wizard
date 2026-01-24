/**
 * This code is responsible for initializing the color system.
 *
 * Uses escape sequences to redefine the 8 standard color slots with custom
 * RGB values, providing a consistent look in Linux TTY environments.
 */

#include "../all.h"

/** The custom RGB values for the 8 color slots. */
static const unsigned char palette[8][3] = {
    [COLOR_BLACK]   = {0x1C, 0x1C, 0x1C},  // Dark gray (screen background)
    [COLOR_RED]     = {0xE6, 0x33, 0x33},  // Red (errors)
    [COLOR_GREEN]   = {0xD6, 0xD6, 0xD0},  // Light gray (row background)
    [COLOR_YELLOW]  = {0xB3, 0xB3, 0xAD},  // Medium gray (header background)
    [COLOR_BLUE]    = {0x1E, 0x90, 0xFF},  // Blue (selection, info)
    [COLOR_MAGENTA] = {0xE6, 0x66, 0x00},  // Orange (warnings)
    [COLOR_CYAN]    = {0x50, 0x50, 0x50},  // Dark gray (dimmed text)
    [COLOR_WHITE]   = {0xE6, 0xE6, 0xE0},  // Off-white (main text/background)
};

/** Tracks whether the palette was modified for cleanup. */
static int palette_modified = 0;

/** Tracks whether we're on a Linux console. */
static int on_linux_console = 0;

void init_colors_palette(void)
{
    const char *term = getenv("TERM");
    if (term == NULL)
    {
        return;
    }

    // Set custom RGB values for color slots 0-7.
    if (strcmp(term, "linux") == 0)
    {
        // Linux console uses \033]P escape sequence.
        on_linux_console = 1;
        for (int i = 0; i < 8; i++)
        {
            printf(
                "\033]P%X%02X%02X%02X",
                i, palette[i][0], palette[i][1], palette[i][2]
            );
        }
    }
    else
    {
        // xterm-compatible terminals use OSC 4 escape sequence.
        for (int i = 0; i < 8; i++)
        {
            printf(
                "\033]4;%d;rgb:%02x/%02x/%02x\007",
                i, palette[i][0], palette[i][1], palette[i][2]
            );
        }
    }

    fflush(stdout);
    palette_modified = 1;
}

void init_colors_pairs(void)
{
    init_pair(COLOR_PAIR_MAIN,      COLOR_BLACK,   COLOR_WHITE);
    init_pair(COLOR_PAIR_ROW,       COLOR_BLACK,   COLOR_GREEN);
    init_pair(COLOR_PAIR_DIM,       COLOR_CYAN,    COLOR_WHITE);
    init_pair(COLOR_PAIR_HEADER,    COLOR_BLACK,   COLOR_YELLOW);
    init_pair(COLOR_PAIR_SELECTED,  COLOR_BLUE,    COLOR_WHITE);
    init_pair(COLOR_PAIR_NOTE,      COLOR_BLACK,   COLOR_GREEN);
    init_pair(COLOR_PAIR_INFO,      COLOR_BLUE,    COLOR_GREEN);
    init_pair(COLOR_PAIR_WARNING,   COLOR_MAGENTA, COLOR_GREEN);
    init_pair(COLOR_PAIR_ERROR,     COLOR_RED,     COLOR_GREEN);
    init_pair(COLOR_PAIR_SCREEN_BG, COLOR_WHITE,   COLOR_BLACK);
}

void cleanup_colors(void)
{
    if (palette_modified)
    {
        if (on_linux_console)
        {
            // Linux console: restore original palette.
            printf("\033]R");
        }
        else
        {
            // xterm-compatible: reset all colors to defaults.
            printf("\033]104\007");
        }
        fflush(stdout);
    }
}
