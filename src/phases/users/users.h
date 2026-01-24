#pragma once
#include "../all.h"

/**
 * Configures user accounts and hostname on the target system.
 *
 * @return - `0` - on success.
 * @return - `-1` - if hostname configuration fails.
 * @return - `-2` - if user creation fails.
 * @return - `-3` - if password setting fails.
 * @return - `-4` - if admin group addition fails.
 */
int configure_users(void);
