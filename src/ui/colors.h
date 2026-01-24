#pragma once

/** The color pair for main UI elements. */
#define COLOR_PAIR_MAIN         1

/** The color pair for table rows. */
#define COLOR_PAIR_ROW          2

/** The color pair for dimmed text. */
#define COLOR_PAIR_DIM          3

/** The color pair for table headers. */
#define COLOR_PAIR_HEADER       4

/** The color pair for selected items. */
#define COLOR_PAIR_SELECTED     5

/** The color pair for note boxes. */
#define COLOR_PAIR_NOTE         6

/** The color pair for info notes. */
#define COLOR_PAIR_INFO         7

/** The color pair for warning notes. */
#define COLOR_PAIR_WARNING      8

/** The color pair for error notes. */
#define COLOR_PAIR_ERROR        9

/** The color pair for the screen background. */
#define COLOR_PAIR_SCREEN_BG    10

/**
 * Sets the console color palette via escape sequences.
 *
 * Must be called before ncurses initialization. Redefines the 8 standard
 * color slots with custom RGB values for a consistent look in Linux TTY.
 */
void init_colors_palette(void);

/** Initializes ncurses color pairs. */
void init_colors_pairs(void);

/** Restores the original console palette on exit. */
void cleanup_colors(void);
