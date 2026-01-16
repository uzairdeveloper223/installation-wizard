/**
 * This code is responsible for managing the UI color system.
 *
 * Colors are defined using logical roles with a single RGB palette as the
 * source of truth. Physical color indices are mapped based on terminal
 * capabilities:
 *
 *   - 16-color mode (ncurses): Uses indices 8-15 via init_color()
 *   - 8-color console mode:    Remaps indices 0-7 via escape sequences
 *   - Fallback mode:           Uses standard terminal colors unchanged
 */

#include "../all.h"

typedef enum {
    COLOR_ROLE_BLACK,
    COLOR_ROLE_WHITE,
    COLOR_ROLE_ROW_ODD_BG,
    COLOR_ROLE_HEADER_BG,
    COLOR_ROLE_ROW_EVEN_BG,
    COLOR_ROLE_DIM,
    COLOR_ROLE_BLUE,
    COLOR_ROLE_ORANGE,
    COLOR_ROLE_RED,
    COLOR_ROLE_COUNT
} ColorRole;

typedef struct {
    unsigned char r, g, b;
} RGB;

/** The single source of truth for all color values. */
static const RGB color_palette[COLOR_ROLE_COUNT] = {
    [COLOR_ROLE_BLACK]       = {0x00, 0x00, 0x00},
    [COLOR_ROLE_WHITE]       = {0xE6, 0xE6, 0xE0},
    [COLOR_ROLE_ROW_ODD_BG]  = {0xD1, 0xD1, 0xCC},
    [COLOR_ROLE_HEADER_BG]   = {0xB3, 0xB3, 0xAD},
    [COLOR_ROLE_ROW_EVEN_BG] = {0xDB, 0xDB, 0xD6},
    [COLOR_ROLE_DIM]         = {0x50, 0x50, 0x50},
    [COLOR_ROLE_BLUE]        = {0x00, 0x66, 0xCC},
    [COLOR_ROLE_ORANGE]      = {0xE6, 0x66, 0x00},
    [COLOR_ROLE_RED]         = {0xE6, 0x33, 0x33},
};

/** The 16-color mode index map where each role gets a dedicated index. */
static const int index_map_16[COLOR_ROLE_COUNT] = {
    [COLOR_ROLE_BLACK]       = COLOR_BLACK,
    [COLOR_ROLE_WHITE]       = COLOR_WHITE,
    [COLOR_ROLE_ROW_ODD_BG]  = 9,
    [COLOR_ROLE_HEADER_BG]   = 10,
    [COLOR_ROLE_ROW_EVEN_BG] = 11,
    [COLOR_ROLE_DIM]         = 8,
    [COLOR_ROLE_BLUE]        = 12,
    [COLOR_ROLE_ORANGE]      = 13,
    [COLOR_ROLE_RED]         = 15,
};

/** The 8-color console mode index map where some roles share indices. */
static const int index_map_8[COLOR_ROLE_COUNT] = {
    [COLOR_ROLE_BLACK]       = COLOR_BLACK,
    [COLOR_ROLE_WHITE]       = COLOR_WHITE,
    [COLOR_ROLE_ROW_ODD_BG]  = COLOR_GREEN,
    [COLOR_ROLE_HEADER_BG]   = COLOR_YELLOW,
    [COLOR_ROLE_ROW_EVEN_BG] = COLOR_GREEN,
    [COLOR_ROLE_DIM]         = COLOR_CYAN,
    [COLOR_ROLE_BLUE]        = COLOR_BLUE,
    [COLOR_ROLE_ORANGE]      = COLOR_MAGENTA,
    [COLOR_ROLE_RED]         = COLOR_RED,
};

/** Maps physical console indices to roles for escape sequence generation. */
static const ColorRole index_8_source[8] = {
    COLOR_ROLE_BLACK,
    COLOR_ROLE_RED,
    COLOR_ROLE_ROW_ODD_BG,
    COLOR_ROLE_HEADER_BG,
    COLOR_ROLE_BLUE,
    COLOR_ROLE_ORANGE,
    COLOR_ROLE_DIM,
    COLOR_ROLE_WHITE,
};

/** A structure to define a color pair using logical roles. */
typedef struct {
    ColorRole fg;
    ColorRole bg;
} PairDef;

/** The color pair definitions using logical roles. */
static const PairDef color_pair_defs[] = {
    [0]                              = {0, 0},
    [CUSTOM_COLOR_PAIR_MAIN]         = {COLOR_ROLE_BLACK,  COLOR_ROLE_WHITE},
    [CUSTOM_COLOR_PAIR_ROW_ODD]      = {COLOR_ROLE_BLACK,  COLOR_ROLE_ROW_ODD_BG},
    [CUSTOM_COLOR_PAIR_DIM]          = {COLOR_ROLE_DIM,    COLOR_ROLE_WHITE},
    [CUSTOM_COLOR_PAIR_HEADER]       = {COLOR_ROLE_BLACK,  COLOR_ROLE_HEADER_BG},
    [CUSTOM_COLOR_PAIR_ROW_EVEN]     = {COLOR_ROLE_BLACK,  COLOR_ROLE_ROW_EVEN_BG},
    [CUSTOM_COLOR_PAIR_SELECTED]     = {COLOR_ROLE_BLUE,   COLOR_ROLE_WHITE},
    [CUSTOM_COLOR_PAIR_NOTE_BG]      = {COLOR_ROLE_BLACK,  COLOR_ROLE_ROW_EVEN_BG},
    [CUSTOM_COLOR_PAIR_NOTE_TEXT]    = {COLOR_ROLE_BLACK,  COLOR_ROLE_ROW_EVEN_BG},
    [CUSTOM_COLOR_PAIR_INFO_NOTE]    = {COLOR_ROLE_BLUE,   COLOR_ROLE_ROW_EVEN_BG},
    [CUSTOM_COLOR_PAIR_WARNING_NOTE] = {COLOR_ROLE_ORANGE, COLOR_ROLE_ROW_EVEN_BG},
    [CUSTOM_COLOR_PAIR_ERROR_NOTE]   = {COLOR_ROLE_RED,    COLOR_ROLE_ROW_EVEN_BG},
};

#define COLOR_PAIR_COUNT (sizeof(color_pair_defs) / sizeof(color_pair_defs[0]))

/** Indicates custom palette is active (affects UI rendering decisions). */
static int use_extended_colors = 0;

/** Tracks console palette modification for restoration on cleanup. */
static int console_palette_modified = 0;

/** Pointer to the active index map based on terminal capabilities. */
static const int *active_index_map = NULL;

static int is_linux_console(void)
{
    const char *term = getenv("TERM");
    return term != NULL && strcmp(term, "linux") == 0;
}

static void set_console_palette(void)
{
    // Emit escape sequences to redefine palette indices 1-7.
    for (int i = 1; i <= 7; i++)
    {
        RGB c = color_palette[index_8_source[i]];
        printf("\033]P%X%02X%02X%02X", i, c.r, c.g, c.b);
    }

    fflush(stdout);
    console_palette_modified = 1;
}

static void restore_console_palette(void)
{
    printf("\033]R");
    fflush(stdout);
}

static void set_ncurses_palette(void)
{
    // Define custom colors using ncurses init_color (0-1000 scale).
    for (int role = 0; role < COLOR_ROLE_COUNT; role++)
    {
        int idx = index_map_16[role];
        RGB c = color_palette[role];
        init_color(idx, c.r * 1000 / 255, c.g * 1000 / 255, c.b * 1000 / 255);
    }
}

static void init_pairs_from_map(void)
{
    // Initialize all color pairs using the active index map.
    for (size_t pair = 1; pair < COLOR_PAIR_COUNT; pair++)
    {
        int fg = active_index_map[color_pair_defs[pair].fg];
        int bg = active_index_map[color_pair_defs[pair].bg];
        init_pair(pair, fg, bg);
    }
}

static void init_fallback_pairs(void)
{
    init_pair(CUSTOM_COLOR_PAIR_MAIN,         COLOR_BLACK,  COLOR_WHITE);
    init_pair(CUSTOM_COLOR_PAIR_ROW_ODD,      COLOR_BLACK,  COLOR_WHITE);
    init_pair(CUSTOM_COLOR_PAIR_DIM,          COLOR_BLACK,  COLOR_WHITE);
    init_pair(CUSTOM_COLOR_PAIR_HEADER,       COLOR_WHITE,  COLOR_BLACK);
    init_pair(CUSTOM_COLOR_PAIR_ROW_EVEN,     COLOR_BLACK,  COLOR_WHITE);
    init_pair(CUSTOM_COLOR_PAIR_SELECTED,     COLOR_BLUE,   COLOR_WHITE);
    init_pair(CUSTOM_COLOR_PAIR_NOTE_BG,      COLOR_BLACK,  COLOR_WHITE);
    init_pair(CUSTOM_COLOR_PAIR_NOTE_TEXT,    COLOR_BLACK,  COLOR_WHITE);
    init_pair(CUSTOM_COLOR_PAIR_INFO_NOTE,    COLOR_BLUE,   COLOR_WHITE);
    init_pair(CUSTOM_COLOR_PAIR_WARNING_NOTE, COLOR_YELLOW, COLOR_WHITE);
    init_pair(CUSTOM_COLOR_PAIR_ERROR_NOTE,   COLOR_RED,    COLOR_WHITE);
}

int colors_has_extended(void)
{
    return use_extended_colors;
}

void colors_init_console_palette(void)
{
    if (is_linux_console())
    {
        set_console_palette();
    }
}

void colors_init_pairs(void)
{
    if (console_palette_modified)
    {
        // Use 8-color map with escape-sequence palette.
        active_index_map = index_map_8;
        use_extended_colors = 1;
        init_pairs_from_map();
    }
    else if (can_change_color() && COLORS >= 16)
    {
        // Apply custom palette via init_color.
        set_ncurses_palette();
        active_index_map = index_map_16;
        use_extended_colors = 1;
        init_pairs_from_map();
    }
    else
    {
        // Fall back to basic terminal colors.
        use_extended_colors = 0;
        init_fallback_pairs();
    }
}

void colors_cleanup(void)
{
    if (console_palette_modified)
    {
        restore_console_palette();
    }
}
