#pragma once
#include "../../all.h"

/**
 * Opens a dialog to edit a user account.
 *
 * @param modal The modal window to display the dialog in.
 * @param store The global store containing users.
 *
 * @return - `0` - Indicates user was updated.
 * @return - `-1` - Indicates no users to edit.
 * @return - `-2` - Indicates user cancelled selection.
 * @return - `-3` - Indicates user cancelled form.
 */
int edit_user_dialog(WINDOW *modal, Store *store);

/**
 * Opens a dialog to add a new user account.
 *
 * @param modal The modal window to display the dialog in.
 * @param store The global store containing users.
 *
 * @return - `0` - Indicates user was added.
 * @return - `-1` - Indicates maximum user limit reached.
 * @return - `-2` - Indicates user cancelled form.
 */
int add_user_dialog(WINDOW *modal, Store *store);

/**
 * Opens a dialog to remove a user account.
 * Note: The first user (default user) cannot be removed.
 *
 * @param modal The modal window to display the dialog in.
 * @param store The global store containing users.
 *
 * @return - `0` - Indicates user was removed.
 * @return - `-1` - Indicates cannot remove primary user.
 * @return - `-2` - Indicates user cancelled selection.
 */
int remove_user_dialog(WINDOW *modal, Store *store);
