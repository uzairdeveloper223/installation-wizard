/**
 * This code is responsible for displaying installation steps and handling
 * user input navigation between steps.
 */

#include "../all.h"

#define MAX_VISIBLE_OPTIONS MODAL_MAX_VISIBLE

void display_step(WINDOW *modal, int step_number, Step *step)
{
    // Clear previous step content before drawing.
    clear_modal(modal);

    // Display step header in bold with primary color.
    wattron(modal, A_BOLD | COLOR_PAIR(UI_COLOR_MAIN));
    mvwprintw(modal, 2, 3, "Step %d: %s", step_number, step->name);
    wattroff(modal, A_BOLD);

    // Render each line of step content below header.
    for (int i = 0; i < step->content_lines; i++)
    {
        mvwprintw(modal, 4 + i, 3, "%s", step->content[i]);
    }

    // Display navigation footer at fixed bottom position if provided.
    if (step->footer != NULL)
    {
        mvwprintw(modal, MODAL_HEIGHT - 2, 3, "%s", step->footer);
    }

    // Refresh window to show updated content.
    wrefresh(modal);
}

int await_step_input(WINDOW *modal)
{
    // Wait for Enter or 'q' key, ignoring all other input.
    int key;
    while ((key = wgetch(modal)) != '\n' && key != 'q')
    {
        // Continue waiting for valid input.
    }

    // Return true if user pressed Enter to proceed.
    return key == '\n';
}

void render_step_options(
    WINDOW *modal, StepOption *options, int count, int selected,
    int start_y, int scroll_offset, int max_visible
)
{
    // Calculate number of options to display.
    int visible_count = count < max_visible ? count : max_visible;

    // Render each visible option.
    for (int i = 0; i < visible_count; i++)
    {
        int option_index = scroll_offset + i;

        // Apply reverse video highlighting if option is selected.
        if (option_index == selected)
        {
            wattron(modal, A_REVERSE);
        }

        // Check if label contains " *" suffix indicating previous selection.
        const char *label = options[option_index].label;
        const char *selected_tag = strstr(label, " *");

        if (selected_tag != NULL && selected_tag[2] == '\0')
        {
            // Render label without the " *" suffix.
            int label_len = selected_tag - label;
            mvwprintw(
                modal,                                  // Modal window.
                start_y + i,                            // Y position.
                3,                                      // X position.
                "  %s %.*s",                            // Format string.
                option_index == selected ? ">" : " ",
                label_len, label
            );

            // Remove highlight before rendering the styled indicator.
            if (option_index == selected)
            {
                wattroff(modal, A_REVERSE);
            }

            // Render " *" indicator in bold blue.
            wattron(modal, COLOR_PAIR(UI_COLOR_SELECTED) | A_BOLD);
            wprintw(modal, " *");
            wattroff(modal, COLOR_PAIR(UI_COLOR_SELECTED) | A_BOLD);
        }
        else
        {
            // Render option with selection indicator.
            mvwprintw(
                modal, start_y + i, 3,
                "  %s %s",
                option_index == selected ? ">" : " ", label
            );

            // Remove highlight after rendering.
            if (option_index == selected)
            {
                wattroff(modal, A_REVERSE);
            }
        }
    }

    // Draw scrollbar on the right edge.
    render_scrollbar(modal, start_y, MODAL_SCROLLBAR_X, visible_count,
                     scroll_offset, max_visible, count);
}

int run_selection_step(
    WINDOW *modal, const char *title, int step_number,
    const char *description, StepOption *options, int count,
    int *out_selected, int allow_back
)
{
    // Initialize selection state from input parameter.
    int current = *out_selected;
    int scroll_offset = 0;
    int max_visible = MAX_VISIBLE_OPTIONS;

    // Adjust initial scroll position to show selected item.
    if (current >= max_visible)
    {
        scroll_offset = current - max_visible + 1;
    }

    // Main input loop.
    while (1)
    {
        // Clear modal and render step header.
        clear_modal(modal);
        wattron(modal, A_BOLD | COLOR_PAIR(UI_COLOR_MAIN));
        mvwprintw(modal, 2, 3, "Step %d: %s", step_number, title);
        wattroff(modal, A_BOLD);

        // Display description text above options.
        mvwprintw(modal, 4, 3, "%s", description);

        // Render the selectable options list.
        render_step_options(
            modal, options, count, current, 6,
            scroll_offset, max_visible
        );

        // Render appropriate footer based on back navigation setting.
        if (allow_back)
        {
            const char *footer[] = {
                "[Up][Down] Navigate", "[Enter] Select", "[Esc] Back", NULL
            };
            render_footer(modal, footer);
        }
        else
        {
            const char *footer[] = {
                "[Up][Down] Navigate", "[Enter] Select", NULL
            };
            render_footer(modal, footer);
        }
        wrefresh(modal);

        // Handle user input.
        int key = wgetch(modal);
        switch (key)
        {
            case KEY_UP:
                // Move selection up if not at first item.
                if (current > 0)
                {
                    current--;
                    // Adjust scroll if selection moved above visible area.
                    if (current < scroll_offset)
                    {
                        scroll_offset = current;
                    }
                }
                break;

            case KEY_DOWN:
                // Move selection down if not at last item.
                if (current < count - 1)
                {
                    current++;
                    // Adjust scroll if selection moved below visible area.
                    if (current >= scroll_offset + max_visible)
                    {
                        scroll_offset = current - max_visible + 1;
                    }
                }
                break;

            case '\n':
                // User confirmed selection.
                *out_selected = current;
                return 1;

            case 27:
                // User pressed Escape to go back.
                if (allow_back)
                {
                    return 0;
                }
                break;
        }
    }
}
