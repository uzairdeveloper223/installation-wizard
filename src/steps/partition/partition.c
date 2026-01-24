/**
 * This code is responsible for running the partition management step of the
 * installation wizard, allowing users to add, edit, and remove partitions.
 */

#include "../../all.h"

int run_partition_step(WINDOW *modal, int step_index)
{
    // Get store and use cached disk size for partition operations.
    Store *store = get_store();
    unsigned long long disk_size = store->disk_size;

    // Define available actions for the partition step.
    StepOption actions[] = {
        {"add", "Add"},
        {"edit", "Edit"},
        {"remove", "Remove"},
        {"autofill", "Autofill"},
        {"done", "Done"}
    };
    int action_count = 5;
    int action_selected = 0;
    int scroll_offset = 0;

    // Run main partition step loop.
    while (1)
    {
        // Adjust scroll offset and get max scroll value.
        int max_scroll = adjust_scroll_offset(
            &scroll_offset, store->partition_count, MAX_VISIBLE_PARTITIONS
        );

        // Clear modal and render step header.
        clear_modal(modal);
        wattron(modal, A_BOLD);
        mvwprintw(modal, 2, 3, "Step %d: %s", step_index + 1, wizard_steps[step_index].display_name);
        wattroff(modal, A_BOLD);

        // Render the partition table.
        render_partition_table(modal, store, disk_size, -1, 0, scroll_offset);

        // Render action menu above footer.
        render_action_menu(modal, MODAL_HEIGHT - 4, 3, actions, action_count, action_selected);

        // Render footer and refresh display.
        const char *footer[] = {
            "[Left][Right] Navigate", "[Enter] Select", "[Esc] Back", NULL
        };
        render_footer(modal, footer);
        wrefresh(modal);

        // Handle user input.
        int key = getch();
        switch (key)
        {
            case KEY_UP:
                // Scroll partition table up.
                if (scroll_offset > 0) scroll_offset--;
                break;

            case KEY_DOWN:
                // Scroll partition table down.
                if (scroll_offset < max_scroll) scroll_offset++;
                break;

            case KEY_LEFT:
                // Move action selection left.
                if (action_selected > 0) action_selected--;
                break;

            case KEY_RIGHT:
                // Move action selection right.
                if (action_selected < action_count - 1) action_selected++;
                break;

            case '\n':
                // Execute selected action.
                if (strcmp(actions[action_selected].value, "add") == 0)
                {
                    add_partition_dialog(modal, store, disk_size);
                }
                else if (strcmp(actions[action_selected].value, "edit") == 0)
                {
                    edit_partition_dialog(modal, store, disk_size);
                }
                else if (strcmp(actions[action_selected].value, "remove") == 0)
                {
                    remove_partition_dialog(modal, store, disk_size);
                }
                else if (strcmp(actions[action_selected].value, "autofill") == 0)
                {
                    autofill_partitions(store, disk_size);
                }
                else if (strcmp(actions[action_selected].value, "done") == 0)
                {
                    return 1;
                }
                break;

            case 27:
                // Return to previous step when user presses Escape.
                return 0;
        }
    }
}
