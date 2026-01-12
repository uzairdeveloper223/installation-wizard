/**
 * This code is responsible for providing dialog interfaces for editing
 * the hostname and user accounts during the installation process.
 */

#include "../../all.h"

#define TEXT_FIELD_USERNAME  0
#define TEXT_FIELD_PASSWORD  1
#define TEXT_FIELD_ADMIN     2
#define TEXT_FIELD_COUNT     3

/** Maximum display width for text input fields. */
#define TEXT_INPUT_WIDTH 24

/** A type representing a text input field for forms. */
typedef struct
{
    const char *label;
    char *buffer;
    size_t buffer_size;
    size_t cursor;
    int is_password;
    int is_spinner;
    const char **spinner_options;
    int spinner_count;
    int spinner_current;
    const char *description;
    int description_is_warning;
} TextField;

static void render_text_form(
    WINDOW *window, int y, int x, int label_width,
    TextField *fields, int field_count, int focused
)
{
    // Render each field in the form.
    for (int i = 0; i < field_count; i++)
    {
        // Calculate row position accounting for description shift.
        int row_y = y + i;
        if (i > focused)
        {
            row_y += FORM_DESCRIPTION_SHIFT;
        }

        // Get field reference and focus state.
        TextField *field = &fields[i];
        int is_focused = (i == focused);

        // Render label.
        mvwprintw(window, row_y, x, "%-*s", label_width, field->label);

        int value_x = x + label_width + 1;

        if (field->is_spinner)
        {
            // Render spinner field.
            if (is_focused)
            {
                wattron(window, A_REVERSE);
            }
            const char *value = field->spinner_options[field->spinner_current];
            mvwprintw(window, row_y, value_x, "< %-*s >", TEXT_INPUT_WIDTH - 4, value);
            if (is_focused)
            {
                wattroff(window, A_REVERSE);
            }
        }
        else
        {
            // Render text input field.
            if (is_focused)
            {
                wattron(window, A_REVERSE);
            }

            // Build display string.
            char display[TEXT_INPUT_WIDTH + 1];
            size_t len = strlen(field->buffer);
            if (field->is_password)
            {
                // Show asterisks for password.
                size_t display_len = (len > TEXT_INPUT_WIDTH) ? TEXT_INPUT_WIDTH : len;
                for (size_t j = 0; j < display_len; j++)
                {
                    display[j] = '*';
                }
                display[display_len] = '\0';
            }
            else
            {
                // Show actual text.
                size_t display_len = (len > TEXT_INPUT_WIDTH) ? TEXT_INPUT_WIDTH : len;
                strncpy(display, field->buffer, display_len);
                display[display_len] = '\0';
            }

            mvwprintw(window, row_y, value_x, "%-*s", TEXT_INPUT_WIDTH, display);

            // Show cursor position when focused.
            if (is_focused)
            {
                size_t cursor_pos = field->cursor;
                if (cursor_pos > TEXT_INPUT_WIDTH - 1)
                {
                    cursor_pos = TEXT_INPUT_WIDTH - 1;
                }
                mvwchgat(window, row_y, value_x + cursor_pos, 1, A_UNDERLINE, 0, NULL);
            }

            if (is_focused)
            {
                wattroff(window, A_REVERSE);
            }
        }

        // Render description below focused field.
        if (is_focused && field->description != NULL)
        {
            if (field->description_is_warning)
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

static int handle_text_input(int key, TextField *field)
{
    // Get current buffer length.
    size_t len = strlen(field->buffer);

    if (key == KEY_BACKSPACE || key == 127 || key == 8)
    {
        // Delete character before cursor.
        if (field->cursor > 0)
        {
            memmove(
                field->buffer + field->cursor - 1,
                field->buffer + field->cursor,
                len - field->cursor + 1
            );
            field->cursor--;
        }
        return 1;
    }
    else if (key == KEY_DC)
    {
        // Delete character at cursor.
        if (field->cursor < len)
        {
            memmove(
                field->buffer + field->cursor,
                field->buffer + field->cursor + 1,
                len - field->cursor
            );
        }
        return 1;
    }
    else if (key == KEY_LEFT)
    {
        // Move cursor left.
        if (field->cursor > 0)
        {
            field->cursor--;
        }
        return 1;
    }
    else if (key == KEY_RIGHT)
    {
        // Move cursor right.
        if (field->cursor < len)
        {
            field->cursor++;
        }
        return 1;
    }
    else if (key == KEY_HOME || key == 1) // Ctrl+A
    {
        field->cursor = 0;
        return 1;
    }
    else if (key == KEY_END || key == 5) // Ctrl+E
    {
        field->cursor = len;
        return 1;
    }
    else if (isprint(key) && len < field->buffer_size - 1)
    {
        // Insert printable character at cursor.
        memmove(
            field->buffer + field->cursor + 1,
            field->buffer + field->cursor,
            len - field->cursor + 1
        );
        field->buffer[field->cursor] = (char)key;
        field->cursor++;
        return 1;
    }

    return 0;
}

static int is_valid_username(const char *username)
{
    // Return invalid if username is empty.
    if (username[0] == '\0')
    {
        return 0;
    }

    // Must start with a lowercase letter.
    if (!islower(username[0]))
    {
        return 0;
    }

    // Can only contain lowercase letters, digits, underscores, hyphens.
    for (size_t i = 0; username[i] != '\0'; i++)
    {
        char c = username[i];
        if (!islower(c) && !isdigit(c) && c != '_' && c != '-')
        {
            return 0;
        }
    }

    return 1;
}

static int run_user_form(
    WINDOW *modal, const char *title, User *user, int is_new_user,
    int is_primary_user
)
{
    // Copy current user data to local buffers.
    char username_buffer[STORE_MAX_USERNAME_LEN];
    char password_buffer[STORE_MAX_PASSWORD_LEN];
    strncpy(username_buffer, user->username, sizeof(username_buffer) - 1);
    username_buffer[sizeof(username_buffer) - 1] = '\0';
    strncpy(password_buffer, user->password, sizeof(password_buffer) - 1);
    password_buffer[sizeof(password_buffer) - 1] = '\0';

    // Set up admin spinner options.
    const char *admin_options[] = { "No", "Yes" };
    int admin_current = user->is_admin ? 1 : 0;

    // Primary user must always be admin.
    if (is_primary_user)
    {
        admin_current = 1;
    }

    // Configure form fields.
    TextField fields[TEXT_FIELD_COUNT] = {
        {
            .label = "Username",
            .buffer = username_buffer,
            .buffer_size = sizeof(username_buffer),
            .cursor = strlen(username_buffer),
            .is_password = 0,
            .is_spinner = 0,
            .description = "Login name for the user account.\n"
                           "Lowercase letters, digits, underscores, hyphens."
        },
        {
            .label = "Password",
            .buffer = password_buffer,
            .buffer_size = sizeof(password_buffer),
            .cursor = strlen(password_buffer),
            .is_password = 1,
            .is_spinner = 0,
            .description = "Password for the user account.\n"
                           "Choose a secure password."
        },
        {
            .label = "Admin",
            .buffer = NULL,
            .buffer_size = 0,
            .cursor = 0,
            .is_password = 0,
            .is_spinner = 1,
            .spinner_options = admin_options,
            .spinner_count = 2,
            .spinner_current = admin_current,
            .description = is_primary_user
                ? "The primary user must have admin privileges.\n"
                  "This setting cannot be changed."
                : "Whether this user has administrator privileges.\n"
                  "Admins can install software and change settings.",
            .description_is_warning = is_primary_user
        }
    };

    int focused = 0;

    while (1)
    {
        clear_modal(modal);
        wattron(modal, A_BOLD);
        mvwprintw(modal, 2, 3, "%s", title);
        wattroff(modal, A_BOLD);

        render_text_form(modal, 4, 3, 11, fields, TEXT_FIELD_COUNT, focused);

        // Show validation messages.
        int valid_username = is_valid_username(username_buffer);
        int valid_password = (strlen(password_buffer) > 0);

        const char *footer_action = is_new_user ? "Add" : "Save";
        char action_string[32];
        snprintf(action_string, sizeof(action_string), "[Enter] %s", footer_action);
        const char *footer[] = {
            "[Arrows] Navigate", action_string, "[Esc] Cancel", NULL
        };
        render_footer(modal, footer);
        wrefresh(modal);

        int key = getch();

        if (key == '\n' || key == KEY_ENTER)
        {
            if (valid_username && valid_password)
            {
                strncpy(user->username, username_buffer, sizeof(user->username) - 1);
                user->username[sizeof(user->username) - 1] = '\0';

                strncpy(user->password, password_buffer, sizeof(user->password) - 1);
                user->password[sizeof(user->password) - 1] = '\0';

                user->is_admin = (fields[TEXT_FIELD_ADMIN].spinner_current == 1);
                return 1;
            }
        }
        else if (key == 27)
        {
            return 0;
        }
        else if (key == KEY_UP)
        {
            if (focused > 0)
            {
                focused--;
            }
        }
        else if (key == KEY_DOWN)
        {
            if (focused < TEXT_FIELD_COUNT - 1)
            {
                focused++;
            }
        }
        else if (fields[focused].is_spinner)
        {
            // Skip spinner input for primary user's admin field.
            if (is_primary_user && focused == TEXT_FIELD_ADMIN)
            {
                continue;
            }

            // Handle spinner navigation.
            if (key == KEY_LEFT)
            {
                if (fields[focused].spinner_current > 0)
                {
                    fields[focused].spinner_current--;
                }
            }
            else if (key == KEY_RIGHT)
            {
                if (fields[focused].spinner_current < fields[focused].spinner_count - 1)
                {
                    fields[focused].spinner_current++;
                }
            }
        }
        else
        {
            // Handle text input.
            handle_text_input(key, &fields[focused]);
        }
    }
}

static int select_user(
    WINDOW *modal, Store *store, const char *title, int allow_first
)
{
    // Initialize selection index based on whether first user is selectable.
    int selected = allow_first ? 0 : 1;
    int scroll_offset = 0;

    // Ensure valid starting selection.
    if (!allow_first && store->user_count <= 1)
    {
        // No users to select (only the default user).
        return -1;
    }

    while (1)
    {
        clear_modal(modal);
        wattron(modal, A_BOLD);
        mvwprintw(modal, 2, 3, "%s", title);
        wattroff(modal, A_BOLD);

        render_user_table(modal, store, selected, 1, scroll_offset);

        const char *footer[] = {
            "[Up][Down] Navigate", "[Enter] Select", "[Esc] Cancel", NULL
        };
        render_footer(modal, footer);
        wrefresh(modal);

        int key = getch();
        int min_select = allow_first ? 0 : 1;

        if (key == KEY_UP && selected > min_select)
        {
            selected--;
            if (selected < scroll_offset)
            {
                scroll_offset = selected;
            }
        }
        else if (key == KEY_DOWN && selected < store->user_count - 1)
        {
            selected++;
            if (selected >= scroll_offset + MAX_VISIBLE_USERS)
            {
                scroll_offset = selected - MAX_VISIBLE_USERS + 1;
            }
        }
        else if (key == '\n')
        {
            return selected;
        }
        else if (key == 27)
        {
            return -1;
        }
    }
}

int edit_user_dialog(WINDOW *modal, Store *store)
{
    // Return early if there are no users to edit.
    if (store->user_count == 0)
    {
        return 0;
    }

    // Let user select which user to edit.
    int selected = select_user(modal, store, "Edit User - Select", 1);
    if (selected < 0)
    {
        return 0;
    }

    // Build dialog title with user number.
    char title[32];
    snprintf(title, sizeof(title), "Edit User %d", selected + 1);

    // Pass is_primary_user flag for the first user.
    return run_user_form(modal, title, &store->users[selected], 0, selected == 0);
}

int add_user_dialog(WINDOW *modal, Store *store)
{
    // Check if maximum user count has been reached.
    if (store->user_count >= STORE_MAX_USERS)
    {
        show_error_dialog(modal, "Add User",
            "Maximum user limit reached.\n"
            "Remove a user before adding a new one.");
        return 0;
    }

    // Create new user with defaults.
    User new_user = {0};
    snprintf(new_user.username, sizeof(new_user.username), "newuser");
    snprintf(new_user.password, sizeof(new_user.password), "password");
    new_user.is_admin = 0;

    // Run the user form and return if cancelled.
    // New users are never the primary user.
    if (!run_user_form(modal, "Add User", &new_user, 1, 0))
    {
        return 0;
    }

    // Add the new user to the store.
    store->users[store->user_count++] = new_user;
    return 1;
}

int remove_user_dialog(WINDOW *modal, Store *store)
{
    // Prevent removal of the default user.
    if (store->user_count <= 1)
    {
        show_error_dialog(modal, "Remove User",
            "Cannot remove the default user.\n"
            "At least one user must exist.");
        return 0;
    }

    // Let user select which user to remove.
    int selected = select_user(modal, store, "Remove User - Select", 0);
    if (selected < 0)
    {
        return 0;
    }

    // Shift remaining users down.
    for (int i = selected; i < store->user_count - 1; i++)
    {
        store->users[i] = store->users[i + 1];
    }
    store->user_count--;

    return 1;
}
