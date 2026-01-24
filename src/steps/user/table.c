/**
 * This code is responsible for rendering the user configuration table display.
 */

#include "../../all.h"

void render_user_table(
    WINDOW *modal, Store *store,
    int selected_user, int in_user_select_mode,
    int scroll_offset
)
{
    // Display header with hostname.
    mvwprintw(modal, 4, 3, "Hostname: %s", store->hostname);

    // Calculate the table width, reducing by 1 if scrollbar is needed.
    int table_width = MODAL_WIDTH - 6;
    if (store->user_count > MAX_VISIBLE_USERS)
    {
        table_width--;
    }

    // Render column headers with darker background.
    wattron(modal, COLOR_PAIR(COLOR_PAIR_HEADER));
    char header[80];
    snprintf(
        header, sizeof(header),
        " #  %-*s %-*s %-*s %-*s",
        USER_COL_WIDTH_NAME, "Username",
        USER_COL_WIDTH_PASS, "Password",
        USER_COL_WIDTH_ADMIN, "Admin",
        USER_COL_WIDTH_PRIMARY, "Primary"
    );
    mvwprintw(modal, 6, 3, "%-*s", table_width, header);
    wattroff(modal, COLOR_PAIR(COLOR_PAIR_HEADER));

    // Render user rows.
    for (int i = 0; i < MAX_VISIBLE_USERS; i++)
    {
        int user_index = scroll_offset + i;

        // Apply alternating row background color.
        int row_color = (user_index % 2 == 0)
            ? COLOR_PAIR_ROW
            : COLOR_PAIR_ROW;
        wattron(modal, COLOR_PAIR(row_color));

        if (user_index < store->user_count)
        {
            User *user = &store->users[user_index];

            // Build username string (without primary marker).
            char username_display[USER_COL_WIDTH_NAME + 1];
            snprintf(
                username_display, sizeof(username_display),
                "%s", user->username
            );

            // Build masked password string.
            char masked_password[17];
            size_t pass_len = strlen(user->password);
            if (pass_len > 16) pass_len = 16;
            for (size_t j = 0; j < pass_len; j++)
            {
                masked_password[j] = '*';
            }
            masked_password[pass_len] = '\0';

            // Build admin status string.
            const char *admin_string = user->is_admin ? "Yes" : "No";

            // Build primary status string.
            const char *primary_string = (user_index == 0) ? "Yes" : "No";

            // Highlight selected user in selection mode.
            if (in_user_select_mode && user_index == selected_user)
            {
                wattron(modal, A_REVERSE);
            }

            // Render the user row.
            char row[80];
            snprintf(
                row, sizeof(row),
                " %-*d %-*s %-*s %-*s %-*s",
                USER_COL_WIDTH_NUM, user_index + 1,
                USER_COL_WIDTH_NAME, username_display,
                USER_COL_WIDTH_PASS, masked_password,
                USER_COL_WIDTH_ADMIN, admin_string,
                USER_COL_WIDTH_PRIMARY, primary_string
            );
            mvwprintw(modal, 7 + i, 3, "%-*s", table_width, row);

            // Remove highlight after rendering.
            if (in_user_select_mode && user_index == selected_user)
            {
                wattroff(modal, A_REVERSE);
            }
        }
        else
        {
            // Render empty row for unused table slots.
            mvwprintw(modal, 7 + i, 3, "%-*s", table_width, "");
        }

        wattroff(modal, COLOR_PAIR(row_color));
    }

    // Draw scrollbar if there are more users than visible rows.
    render_scrollbar(
        modal, MODAL_TABLE_START_Y, MODAL_WIDTH - 3,
        MAX_VISIBLE_USERS, scroll_offset,
        MAX_VISIBLE_USERS, store->user_count
    );
}
