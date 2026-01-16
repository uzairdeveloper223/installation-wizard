#pragma once

/** The color pair for main UI elements. */
#define CUSTOM_COLOR_PAIR_MAIN         1

/** The color pair for odd table rows. */
#define CUSTOM_COLOR_PAIR_ROW_ODD      2

/** The color pair for dimmed text. */
#define CUSTOM_COLOR_PAIR_DIM          3

/** The color pair for table headers. */
#define CUSTOM_COLOR_PAIR_HEADER       4

/** The color pair for even table rows. */
#define CUSTOM_COLOR_PAIR_ROW_EVEN     5

/** The color pair for selected items. */
#define CUSTOM_COLOR_PAIR_SELECTED     6

/** The color pair for note box background. */
#define CUSTOM_COLOR_PAIR_NOTE_BG      7

/** The color pair for note box text. */
#define CUSTOM_COLOR_PAIR_NOTE_TEXT    8

/** The color pair for info note accent. */
#define CUSTOM_COLOR_PAIR_INFO_NOTE    9

/** The color pair for warning note accent. */
#define CUSTOM_COLOR_PAIR_WARNING_NOTE 10

/** The color pair for error note accent. */
#define CUSTOM_COLOR_PAIR_ERROR_NOTE   11

/** Returns non-zero if custom palette is active. */
int colors_has_extended(void);

/**
 * Initializes color palette for the Linux console.
 *
 * Must be called before ncurses initialization. Only has effect when
 * running on the Linux console (TERM=linux).
 */
void colors_init_console_palette(void);

/**
 * Initializes color pairs based on terminal capabilities.
 *
 * Must be called after ncurses initialization and start_color().
 */
void colors_init_pairs(void);

/** Restores console palette if it was modified. */
void colors_cleanup(void);
