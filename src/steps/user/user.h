#pragma once
#include "../../all.h"

/**
 * Runs the user configuration step of the installation wizard.
 * Allows users to configure username, hostname, and password.
 *
 * @param modal The modal window to display the step in.
 *
 * @return 1 if user confirmed and proceeded, 0 if user went back.
 */
int run_user_step(WINDOW *modal);
