#pragma once
#include "../../all.h"

/**
 * Opens a dialog to edit a user account.
 *
 * @param modal The modal window to display the dialog in.
 * @param store The global store containing users.
 *
 * @return 1 if user was updated, 0 if cancelled.
 */
int edit_user_dialog(WINDOW *modal, Store *store);

/**
 * Opens a dialog to add a new user account.
 *
 * @param modal The modal window to display the dialog in.
 * @param store The global store containing users.
 *
 * @return 1 if user was added, 0 if cancelled.
 */
int add_user_dialog(WINDOW *modal, Store *store);

/**
 * Opens a dialog to remove a user account.
 * Note: The first user (default user) cannot be removed.
 *
 * @param modal The modal window to display the dialog in.
 * @param store The global store containing users.
 *
 * @return 1 if user was removed, 0 if cancelled.
 */
int remove_user_dialog(WINDOW *modal, Store *store);
