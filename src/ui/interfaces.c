/**
 * This code is responsible for providing reusable UI building blocks such as
 * scrollbars, tables, forms, and styled text helpers. Note that this file is
 * named "interfaces" rather than "components" because LimeOS uses "components"
 * to refer to extendable pieces of software within the operating system.
 */

#include "../all.h"

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

    // Print text with dimmed styling.
    wattron(window, COLOR_PAIR(UI_COLOR_DIM));
    wmove(window, y, x);
    vw_printw(window, format, arguments);
    wattroff(window, COLOR_PAIR(UI_COLOR_DIM));

    va_end(arguments);
}

void print_warning(WINDOW *window, int y, int x, const char *format, ...)
{
    // Initialize variadic arguments.
    va_list arguments;
    va_start(arguments, format);

    // Print text with warning styling.
    wattron(window, A_BOLD | COLOR_PAIR(UI_COLOR_WARNING));
    wmove(window, y, x);
    vw_printw(window, format, arguments);
    wattroff(window, A_BOLD | COLOR_PAIR(UI_COLOR_WARNING));

    va_end(arguments);
}

void print_selected(WINDOW *window, int y, int x, const char *format, ...)
{
    // Initialize variadic arguments.
    va_list arguments;
    va_start(arguments, format);

    // Print text with selected styling.
    wattron(window, A_BOLD | COLOR_PAIR(UI_COLOR_SELECTED));
    wmove(window, y, x);
    vw_printw(window, format, arguments);
    wattroff(window, A_BOLD | COLOR_PAIR(UI_COLOR_SELECTED));

    va_end(arguments);
}

void render_table(
    WINDOW *window, int y, int x, TableColumn *columns, int column_count,
    TableRow *rows, int row_count,
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
    wattron(window, COLOR_PAIR(UI_COLOR_HEADER));
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
    wattroff(window, COLOR_PAIR(UI_COLOR_HEADER));

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
        int row_color = (row_index % 2 == 0) ? UI_COLOR_ROW_ODD : UI_COLOR_ROW_EVEN;
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

void render_form(
    WINDOW *window, int y, int x, int label_width,
    FormField *fields, int field_count, int focused
)
{
    // Render each field.
    for (int field_index = 0; field_index < field_count; field_index++)
    {
        int row_y = y + (field_index * 2);
        int is_focused = (field_index == focused);

        // Render label.
        mvwprintw(window, row_y, x, "%-*s", label_width, fields[field_index].label);

        // Apply field styling: reverse for focused, dimmed for readonly.
        int value_x = x + label_width + 1;
        if (is_focused && !fields[field_index].readonly)
        {
            wattron(window, A_REVERSE);
        }
        if (fields[field_index].readonly)
        {
            wattron(window, COLOR_PAIR(UI_COLOR_DIM));
        }

        // Render value with arrows for navigable fields.
        if (fields[field_index].options != NULL && fields[field_index].current >= 0 &&
            fields[field_index].current < fields[field_index].option_count)
        {
            if (!fields[field_index].readonly)
            {
                mvwprintw(window, row_y, value_x, "< %s >", fields[field_index].options[fields[field_index].current]);
            }
            else
            {
                mvwprintw(window, row_y, value_x, "  %s", fields[field_index].options[fields[field_index].current]);
            }
        }

        // Disable field styling after rendering.
        if (fields[field_index].readonly)
        {
            wattroff(window, COLOR_PAIR(UI_COLOR_DIM));
        }
        if (is_focused && !fields[field_index].readonly)
        {
            wattroff(window, A_REVERSE);
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
