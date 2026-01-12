/**
 * This code is responsible for running the user configuration step of the
 * installation wizard, allowing users to configure hostname, username,
 * and password settings.
 */

#include "../../all.h"

#define USER_STEP_NUM 2

int run_user_step(WINDOW *modal)
{
    // Get the global store.
    Store *store = get_store();

    // Initialize default user if not already set.
    if (store->user_count == 0)
    {
        snprintf(store->users[0].username, sizeof(store->users[0].username), "user");
        snprintf(store->users[0].password, sizeof(store->users[0].password), "password");
        store->users[0].is_admin = 1;
        store->user_count = 1;
        generate_hostname(store->users[0].username, store->hostname, sizeof(store->hostname));
    }

    // Define available actions for the user step.
    StepOption actions[] = {
        {"edit", "Edit"},
        {"add", "Add"},
        {"remove", "Remove"},
        {"done", "Done"}
    };
    int action_count = 4;
    int action_selected = 0;
    int scroll_offset = 0;

    while (1)
    {
        // Adjust scroll offset and get max scroll value.
        int max_scroll = adjust_scroll_offset(
            &scroll_offset, store->user_count, MAX_VISIBLE_USERS
        );

        // Clear modal and render step header.
        clear_modal(modal);
        wattron(modal, A_BOLD);
        mvwprintw(modal, 2, 3, "Step %d: Users", USER_STEP_NUM);
        wattroff(modal, A_BOLD);

        // Render the user table.
        render_user_table(modal, store, -1, 0, scroll_offset);

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
                if (scroll_offset > 0) scroll_offset--;
                break;

            case KEY_DOWN:
                if (scroll_offset < max_scroll) scroll_offset++;
                break;

            case KEY_LEFT:
                if (action_selected > 0) action_selected--;
                break;

            case KEY_RIGHT:
                if (action_selected < action_count - 1) action_selected++;
                break;

            case '\n':
                if (strcmp(actions[action_selected].value, "edit") == 0)
                {
                    // Capture old primary username to detect changes.
                    char old_primary_username[STORE_MAX_USERNAME_LEN];
                    strncpy(old_primary_username, store->users[0].username, sizeof(old_primary_username) - 1);
                    old_primary_username[sizeof(old_primary_username) - 1] = '\0';

                    edit_user_dialog(modal, store);

                    // Regenerate hostname if primary user's username changed.
                    if (strcmp(old_primary_username, store->users[0].username) != 0)
                    {
                        generate_hostname(store->users[0].username, store->hostname, sizeof(store->hostname));
                    }
                }
                else if (strcmp(actions[action_selected].value, "add") == 0)
                {
                    add_user_dialog(modal, store);
                }
                else if (strcmp(actions[action_selected].value, "remove") == 0)
                {
                    // Capture old primary username to detect changes.
                    char old_primary_username[STORE_MAX_USERNAME_LEN];
                    strncpy(old_primary_username, store->users[0].username, sizeof(old_primary_username) - 1);
                    old_primary_username[sizeof(old_primary_username) - 1] = '\0';

                    remove_user_dialog(modal, store);

                    // Regenerate hostname if primary user changed (due to removal shifting users).
                    if (strcmp(old_primary_username, store->users[0].username) != 0)
                    {
                        generate_hostname(store->users[0].username, store->hostname, sizeof(store->hostname));
                    }
                }
                else if (strcmp(actions[action_selected].value, "done") == 0)
                {
                    return 1;
                }
                break;

            case 27:
                return 0;
        }
    }
}
