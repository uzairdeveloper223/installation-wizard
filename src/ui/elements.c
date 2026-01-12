/**
 * This code is responsible for providing reusable UI building blocks such as
 * scrollbars, tables, forms, and styled text helpers. Note that this file is
 * named "elements" rather than "components" because LimeOS uses "components"
 * to refer to extendable pieces of software within the operating system.
 */

#include "../all.h"

static int is_field_valid(const FormField *field)
{
    return field->options != NULL &&
           field->current >= 0 &&
           field->current < field->option_count;
}

void render_scrollbar(
    WINDOW *window, int y, int x, int height, int offset, int visible, int total
)
{
    // Return early if no scrollbar is needed.
    if (total <= visible)
    {
        return;
    }

    // Calculate thumb position.
    int max_scroll = total - visible;
    int thumb_position = (offset * (height - 1)) / max_scroll;

    // Render scrollbar.
    for (int position = 0; position < height; position++)
    {
        if (position == thumb_position)
        {
            wattron(window, A_REVERSE);
            mvwaddch(window, y + position, x, ' ');
            wattroff(window, A_REVERSE);
        }
        else
        {
            mvwaddch(window, y + position, x, ACS_VLINE);
        }
    }
}

void print_bold(WINDOW *window, int y, int x, const char *format, ...)
{
    // Initialize variadic arguments.
    va_list arguments;
    va_start(arguments, format);

    // Print text with bold styling.
    wattron(window, A_BOLD);
    wmove(window, y, x);
    vw_printw(window, format, arguments);
    wattroff(window, A_BOLD);

    va_end(arguments);
}

void print_dim(WINDOW *window, int y, int x, const char *format, ...)
{
    // Initialize variadic arguments.
    va_list arguments;
    va_start(arguments, format);

    // Print text with dimmed styling (use A_DIM on 8-color terminals).
    int attrs = COLOR_PAIR(CUSTOM_COLOR_PAIR_DIM);
    if (!ui_has_extended_colors())
    {
        attrs |= A_DIM;
    }

    wattron(window, attrs);
    wmove(window, y, x);
    vw_printw(window, format, arguments);
    wattroff(window, attrs);

    va_end(arguments);
}

void print_selected(WINDOW *window, int y, int x, const char *format, ...)
{
    // Initialize variadic arguments.
    va_list arguments;
    va_start(arguments, format);

    // Print text with selected styling.
    wattron(window, A_BOLD | COLOR_PAIR(CUSTOM_COLOR_PAIR_SELECTED));
    wmove(window, y, x);
    vw_printw(window, format, arguments);
    wattroff(window, A_BOLD | COLOR_PAIR(CUSTOM_COLOR_PAIR_SELECTED));

    va_end(arguments);
}

void render_table(
    WINDOW *window, int y, int x, const TableColumn *columns, int column_count,
    const TableRow *rows, int row_count,
    int selected, int scroll_offset, int max_visible
)
{
    // Calculate total table width.
    int table_width = 0;
    for (int column_index = 0; column_index < column_count; column_index++)
    {
        table_width += columns[column_index].width;
        if (column_index < column_count - 1)
        {
            table_width += 1;
        }
    }

    // Reduce width if scrollbar needed.
    int needs_scrollbar = (row_count > max_visible);
    if (needs_scrollbar)
    {
        table_width--;
    }

    // Render header row.
    wattron(window, COLOR_PAIR(CUSTOM_COLOR_PAIR_HEADER));
    int column_x = x;
    for (int column_index = 0; column_index < column_count; column_index++)
    {
        mvwprintw(window, y, column_x, "%-*.*s", columns[column_index].width, columns[column_index].width, columns[column_index].header);
        column_x += columns[column_index].width;
        if (column_index < column_count - 1)
        {
            column_x++;
        }
    }

    // Pad rest of header.
    int remaining = table_width - (column_x - x);
    if (remaining > 0)
    {
        wprintw(window, "%*s", remaining, "");
    }
    wattroff(window, COLOR_PAIR(CUSTOM_COLOR_PAIR_HEADER));

    // Calculate the number of visible rows.
    int visible_count = (row_count < max_visible) ? row_count : max_visible;
    if (visible_count == 0)
    {
        visible_count = max_visible;
    }

    // Render each row.
    for (int visible_index = 0; visible_index < visible_count; visible_index++)
    {
        int row_index = scroll_offset + visible_index;
        int row_color = (row_index % 2 == 0) ? CUSTOM_COLOR_PAIR_ROW_ODD : CUSTOM_COLOR_PAIR_ROW_EVEN;
        int is_selected_row = (row_index == selected);

        // Apply row styling: reverse for selected, alternating colors otherwise.
        if (is_selected_row)
        {
            wattron(window, A_REVERSE);
        }
        else
        {
            wattron(window, COLOR_PAIR(row_color));
        }

        // Render either row data or empty row.
        if (row_index < row_count)
        {
            column_x = x;
            for (int column_index = 0; column_index < column_count && column_index < rows[row_index].cell_count; column_index++)
            {
                if (columns[column_index].align == TABLE_ALIGN_RIGHT)
                {
                    mvwprintw(
                        window,
                        y + 1 + visible_index,
                        column_x,
                        "%*.*s",
                        columns[column_index].width,
                        columns[column_index].width,
                        rows[row_index].cells[column_index]
                    );
                }
                else
                {
                    mvwprintw(
                        window,
                        y + 1 + visible_index,
                        column_x,
                        "%-*.*s",
                        columns[column_index].width,
                        columns[column_index].width,
                        rows[row_index].cells[column_index]
                    );
                }
                column_x += columns[column_index].width;

                // Add space between columns if not last.
                if (column_index < column_count - 1)
                {
                    column_x++;
                }
            }
        }
        else
        {
            mvwprintw(window, y + 1 + visible_index, x, "%*s", table_width, "");
        }

        // Disable row styling after rendering.
        if (is_selected_row)
        {
            wattroff(window, A_REVERSE);
        }
        else
        {
            wattroff(window, COLOR_PAIR(row_color));
        }
    }

    // Render scrollbar if needed.
    if (needs_scrollbar)
    {
        render_scrollbar(
            window, y + 1, x + table_width + 1, max_visible,
            scroll_offset, max_visible, row_count
        );
    }
}

static void render_styled_note(
    WINDOW *window, int y, int x, const char *text, int accent_color
)
{
    // Fill background area with slightly darker color.
    wattron(window, COLOR_PAIR(CUSTOM_COLOR_PAIR_NOTE_BG));
    for (int row = 0; row < NOTE_HEIGHT; row++)
    {
        mvwprintw(window, y + row, x + 1, "%*s", MODAL_WIDTH - NOTE_MARGIN, "");
    }
    wattroff(window, COLOR_PAIR(CUSTOM_COLOR_PAIR_NOTE_BG));

    // Draw accent line on the left.
    wattron(window, COLOR_PAIR(accent_color) | A_REVERSE);
    for (int row = 0; row < NOTE_HEIGHT; row++)
    {
        mvwaddch(window, y + row, x, ' ');
    }
    wattroff(window, COLOR_PAIR(accent_color) | A_REVERSE);

    // Render text on top of background.
    const char *line_start = text;
    int line_num = 0;
    int text_x = x + 2;

    while (*line_start)
    {
        const char *line_end = strchr(line_start, '\n');
        wattron(window, COLOR_PAIR(CUSTOM_COLOR_PAIR_NOTE_TEXT));
        if (line_end)
        {
            int len = line_end - line_start;
            mvwprintw(window, y + line_num, text_x, "%.*s", len, line_start);
            line_start = line_end + 1;
        }
        else
        {
            mvwprintw(window, y + line_num, text_x, "%s", line_start);
            wattroff(window, COLOR_PAIR(CUSTOM_COLOR_PAIR_NOTE_TEXT));
            break;
        }
        wattroff(window, COLOR_PAIR(CUSTOM_COLOR_PAIR_NOTE_TEXT));
        line_num++;
    }
}

void render_note(WINDOW *window, int y, int x, const char *text)
{
    render_styled_note(window, y, x, text, CUSTOM_COLOR_PAIR_NOTE_TEXT);
}

void render_info(WINDOW *window, int y, int x, const char *text)
{
    render_styled_note(window, y, x, text, CUSTOM_COLOR_PAIR_INFO_NOTE);
}

void render_warning(WINDOW *window, int y, int x, const char *text)
{
    render_styled_note(window, y, x, text, CUSTOM_COLOR_PAIR_WARNING_NOTE);
}

void render_error(WINDOW *window, int y, int x, const char *text)
{
    render_styled_note(window, y, x, text, CUSTOM_COLOR_PAIR_ERROR_NOTE);
}

void render_form(
    WINDOW *window, int y, int x, int label_width,
    const FormField *fields, int field_count, int focused
)
{
    // Render each field, with description below focused field.
    for (int field_index = 0; field_index < field_count; field_index++)
    {
        // Calculate row position, accounting for description below focused.
        int row_y = y + field_index;
        if (field_index > focused)
        {
            row_y += FORM_DESCRIPTION_SHIFT;
        }

        const FormField *field = &fields[field_index];
        int is_focused = (field_index == focused);

        // Render label.
        mvwprintw(window, row_y, x, "%-*s", label_width, field->label);

        // Apply field styling: reverse for focused, dimmed for readonly.
        int value_x = x + label_width + 1;
        if (is_focused && !field->readonly)
        {
            wattron(window, A_REVERSE);
        }
        if (field->readonly)
        {
            int dim_attrs = COLOR_PAIR(CUSTOM_COLOR_PAIR_DIM);
            if (!ui_has_extended_colors())
            {
                dim_attrs |= A_DIM;
            }
            wattron(window, dim_attrs);
        }

        // Render value with arrows for navigable fields.
        if (is_field_valid(field))
        {
            const char *value = field->options[field->current];
            if (!field->readonly)
            {
                mvwprintw(window, row_y, value_x, "< %s >", value);
            }
            else
            {
                mvwprintw(window, row_y, value_x, "  %s", value);
            }
        }

        // Disable field styling after rendering.
        if (field->readonly)
        {
            int dim_attrs = COLOR_PAIR(CUSTOM_COLOR_PAIR_DIM);
            if (!ui_has_extended_colors())
            {
                dim_attrs |= A_DIM;
            }
            wattroff(window, dim_attrs);
        }
        if (is_focused && !field->readonly)
        {
            wattroff(window, A_REVERSE);
        }

        // Render description below focused field (with gap above).
        if (is_focused && field->description != NULL)
        {
            if (field->warning)
            {
                render_warning(window, row_y + 2, x, field->description);
            }
            else
            {
                render_info(window, row_y + 2, x, field->description);
            }
        }
    }
}

FormResult handle_form_key(
    int key, FormField *fields, int field_count, int *out_focused
)
{
    // Handle key input.
    switch (key)
    {
        case KEY_UP:
            // Move focus to previous non-readonly field.
            do
            {
                if (*out_focused > 0)
                {
                    (*out_focused)--;
                }
            } while (*out_focused > 0 && fields[*out_focused].readonly);
            break;

        case KEY_DOWN:
            // Move focus to next non-readonly field.
            do
            {
                if (*out_focused < field_count - 1)
                {
                    (*out_focused)++;
                }
            } while (*out_focused < field_count - 1 && fields[*out_focused].readonly);
            break;

        case KEY_LEFT:
            // Decrement current option value.
            if (!fields[*out_focused].readonly && fields[*out_focused].current > 0)
            {
                fields[*out_focused].current--;
            }
            break;

        case KEY_RIGHT:
            // Increment current option value.
            if (!fields[*out_focused].readonly &&
                fields[*out_focused].current < fields[*out_focused].option_count - 1)
            {
                fields[*out_focused].current++;
            }
            break;

        case '\n':
        case KEY_ENTER:
            return FORM_SUBMIT;

        case 27:
            return FORM_CANCEL;
    }

    return FORM_CONTINUE;
}

void render_footer(WINDOW *modal, const char **items)
{
    // Compute dim attributes (use A_DIM on 8-color terminals).
    int dim_attrs = COLOR_PAIR(CUSTOM_COLOR_PAIR_DIM);
    if (!ui_has_extended_colors())
    {
        dim_attrs |= A_DIM;
    }

    // Iterate through footer items and render each one.
    int x = 3;
    for (int item_index = 0; items[item_index] != NULL; item_index++)
    {
        if (item_index > 0)
        {
            x += 2; // Add 2 space gap between items.
        }

        const char *cursor = items[item_index];
        while (*cursor)
        {
            if (*cursor == '[')
            {
                // Find closing bracket and render in bold.
                const char *end = strchr(cursor, ']');
                if (end != NULL)
                {
                    int length = end - cursor + 1;
                    wattron(modal, A_BOLD);
                    mvwprintw(modal, MODAL_HEIGHT - 2, x, "%.*s", length, cursor);
                    wattroff(modal, A_BOLD);
                    x += length;
                    cursor = end + 1;
                }
                else
                {
                    // Render rest in gray when no closing bracket found.
                    wattron(modal, dim_attrs);
                    mvwprintw(modal, MODAL_HEIGHT - 2, x, "%s", cursor);
                    wattroff(modal, dim_attrs);
                    x += strlen(cursor);
                    break;
                }
            }
            else
            {
                // Find next '[' or end of string, render in gray.
                const char *next = strchr(cursor, '[');
                int length = next ? (next - cursor) : (int)strlen(cursor);
                wattron(modal, dim_attrs);
                mvwprintw(modal, MODAL_HEIGHT - 2, x, "%.*s", length, cursor);
                wattroff(modal, dim_attrs);
                x += length;
                cursor += length;
            }
        }
    }
}

void render_action_menu(
    WINDOW *window, int y, int x,
    const StepOption *actions, int count, int selected
)
{
    // Render each action item in the menu.
    for (int i = 0; i < count; i++)
    {
        // Apply highlight styling if this action is selected.
        if (i == selected)
        {
            wattron(window, A_REVERSE);
        }

        // Print the action label with padding.
        mvwprintw(window, y, x, " %s ", actions[i].label);

        // Remove highlight styling after printing.
        if (i == selected)
        {
            wattroff(window, A_REVERSE);
        }

        // Advance x position for next action.
        x += strlen(actions[i].label) + 3;
    }
}

int adjust_scroll_offset(int *out_scroll_offset, int item_count, int max_visible)
{
    // Adjust scroll offset if items were removed.
    if (*out_scroll_offset > 0 && *out_scroll_offset >= item_count)
    {
        if (item_count > 0)
        {
            *out_scroll_offset = item_count - 1;
        }
        else
        {
            *out_scroll_offset = 0;
        }
    }

    // Calculate maximum scroll offset.
    int max_scroll = 0;
    if (item_count > max_visible)
    {
        max_scroll = item_count - max_visible;
    }
    if (*out_scroll_offset > max_scroll)
    {
        *out_scroll_offset = max_scroll;
    }

    return max_scroll;
}

void show_error_dialog(WINDOW *modal, const char *title, const char *message)
{
    // Clear modal and render dialog title.
    clear_modal(modal);
    wattron(modal, A_BOLD);
    mvwprintw(modal, 2, 3, "%s", title);
    wattroff(modal, A_BOLD);

    // Render error message and footer.
    render_error(modal, 5, 3, message);
    const char *footer[] = { "[Enter] OK", NULL };
    render_footer(modal, footer);
    wrefresh(modal);

    // Wait for Enter key.
    while (getch() != '\n');
}
