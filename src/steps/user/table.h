#pragma once
#include "../../all.h"

/**
 * Maximum number of user rows visible in the table.
 */
#define MAX_VISIBLE_USERS 3

/** The column width for user number. */
#define USER_COL_WIDTH_NUM    2

/** The column width for username. */
#define USER_COL_WIDTH_NAME   16

/** The column width for password display. */
#define USER_COL_WIDTH_PASS   16

/** The column width for admin status. */
#define USER_COL_WIDTH_ADMIN  6

/**
 * Renders the user table in the modal window.
 *
 * @param modal                 The modal window to draw in.
 * @param store                 The global store containing users.
 * @param selected_user         Index of selected user (-1 for none).
 * @param in_user_select_mode   Whether selection highlighting is active.
 * @param scroll_offset         First visible user index.
 */
void render_user_table(
    WINDOW *modal, Store *store,
    int selected_user, int in_user_select_mode,
    int scroll_offset
);
