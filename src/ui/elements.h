#pragma once
#include "../all.h"

/** Height of note/info/warning/error boxes in rows. */
#define NOTE_HEIGHT 2

/** Margin from modal edge for note boxes. */
#define NOTE_MARGIN 8

/**
 * Row shift to accommodate description below focused form field.
 * Accounts for 2-line note + gap above + gap below.
 */
#define FORM_DESCRIPTION_SHIFT 4

/** A type representing column alignment options. */
typedef enum
{
    TABLE_ALIGN_LEFT,
    TABLE_ALIGN_RIGHT,
    TABLE_ALIGN_CENTER
} TableAlign;

/** A type representing a single table column definition. */
typedef struct
{
    const char *header;
    int width;
    TableAlign align;
} TableColumn;

/** A type representing a table row with cells. */
typedef struct
{
    char cells[6][64];
    int cell_count;
} TableRow;

/** A type representing a form field for spinner-type input. */
typedef struct
{
    const char *label;
    const char **options;
    int option_count;
    int current;
    int readonly;
    const char *description;
    int warning;
} FormField;

/** A type representing the result of form input handling. */
typedef enum
{
    FORM_CONTINUE,
    FORM_SUBMIT,
    FORM_CANCEL
} FormResult;

/**
 * Renders a vertical scrollbar with track and thumb indicator.
 *
 * @param window The ncurses window to draw in.
 * @param y Y coordinate of the top of the scrollbar track.
 * @param x X coordinate of the scrollbar.
 * @param height Height of the scrollbar track in characters.
 * @param offset Current scroll offset (first visible item index).
 * @param visible Number of visible items.
 * @param total Total number of items.
 */
void render_scrollbar(
    WINDOW *window, int y, int x, int height, int offset, int visible, int total
);

/**
 * Prints bold text at the specified position.
 *
 * @param window The ncurses window to draw in.
 * @param y Y coordinate.
 * @param x X coordinate.
 * @param format Format string.
 */
void print_bold(WINDOW *window, int y, int x, const char *format, ...);

/**
 * Prints dimmed (gray) text at the specified position.
 *
 * @param window The ncurses window to draw in.
 * @param y Y coordinate.
 * @param x X coordinate.
 * @param format Format string.
 */
void print_dim(WINDOW *window, int y, int x, const char *format, ...);

/**
 * Prints text with the selected indicator style (bold dodger blue).
 *
 * @param window The ncurses window to draw in.
 * @param y Y coordinate.
 * @param x X coordinate.
 * @param format Format string.
 */
void print_selected(WINDOW *window, int y, int x, const char *format, ...);

/**
 * Renders a data table with header, scrolling rows, and optional scrollbar.
 *
 * @param window The ncurses window to draw in.
 * @param y Top Y coordinate for header row.
 * @param x Left edge X coordinate.
 * @param columns Array of column definitions.
 * @param column_count Number of columns.
 * @param rows Array of data rows.
 * @param row_count Total number of data rows.
 * @param selected Index of selected row (-1 for none).
 * @param scroll_offset Index of first visible row.
 * @param max_visible Maximum visible rows (excluding header).
 */
void render_table(
    WINDOW *window, int y, int x, const TableColumn *columns, int column_count,
    const TableRow *rows, int row_count, int selected, int scroll_offset,
    int max_visible
);

/**
 * Renders a form with multiple fields.
 *
 * @param window The ncurses window to draw in.
 * @param y Top Y coordinate.
 * @param x Left edge X coordinate.
 * @param label_width Width reserved for labels.
 * @param fields Array of form fields.
 * @param field_count Number of fields.
 * @param focused Index of currently focused field.
 */
void render_form(
    WINDOW *window, int y, int x, int label_width, const FormField *fields,
    int field_count, int focused
);

/**
 * Handles keyboard input for form navigation.
 *
 * @param key The key code from wgetch().
 * @param fields Array of form fields.
 * @param field_count Number of fields.
 * @param out_focused Pointer to focused field index (will be modified).
 *
 * @return Form result indicating action to take.
 */
FormResult handle_form_key(
    int key, FormField *fields, int field_count, int *out_focused
);

/**
 * Renders a styled note box with accent line and background (gray).
 *
 * @param window The ncurses window to draw in.
 * @param y Top Y coordinate.
 * @param x Left edge X coordinate.
 * @param text The note text (can contain newlines for multi-line).
 */
void render_note(WINDOW *window, int y, int x, const char *text);

/**
 * Renders a styled info box with accent line and background (blue).
 *
 * @param window The ncurses window to draw in.
 * @param y Top Y coordinate.
 * @param x Left edge X coordinate.
 * @param text The info text (can contain newlines for multi-line).
 */
void render_info(WINDOW *window, int y, int x, const char *text);

/**
 * Renders a styled warning box with accent line and background (orange).
 *
 * @param window The ncurses window to draw in.
 * @param y Top Y coordinate.
 * @param x Left edge X coordinate.
 * @param text The warning text (can contain newlines for multi-line).
 */
void render_warning(WINDOW *window, int y, int x, const char *text);

/**
 * Renders a styled error box with accent line and background (red).
 *
 * @param window The ncurses window to draw in.
 * @param y Top Y coordinate.
 * @param x Left edge X coordinate.
 * @param text The error text (can contain newlines for multi-line).
 */
void render_error(WINDOW *window, int y, int x, const char *text);

/**
 * Renders footer items with darker background, separated by spaces.
 *
 * @param modal The modal window to render the footer in.
 * @param items NULL-terminated array of strings.
 */
void render_footer(WINDOW *modal, const char **items);

/**
 * Renders a horizontal action menu with selection highlighting.
 *
 * @param window The ncurses window to draw in.
 * @param y Y coordinate for the menu.
 * @param x Starting X coordinate.
 * @param actions Array of action options.
 * @param count Number of actions.
 * @param selected Index of selected action.
 */
void render_action_menu(
    WINDOW *window, int y, int x,
    const StepOption *actions, int count, int selected
);

/**
 * Adjusts scroll offset to keep it within valid bounds.
 *
 * @param out_scroll_offset Pointer to scroll offset (will be modified).
 * @param item_count Total number of items.
 * @param max_visible Maximum visible items.
 *
 * @return The computed max_scroll value.
 */
int adjust_scroll_offset(int *out_scroll_offset, int item_count, int max_visible);

/**
 * Shows an error dialog with a title and message, waiting for Enter.
 *
 * @param modal The modal window to display in.
 * @param title The dialog title.
 * @param message The error message (can contain newlines).
 */
void show_error_dialog(WINDOW *modal, const char *title, const char *message);
