#pragma once

/*
 * semistatic: expands to 'static' normally, or nothing when -DTESTING is set.
 * This exposes functions for unit testing while keeping them static in 
 * production. Declare test-accessible functions in tests/all.h.
 */
#ifdef TESTING
#define semistatic
#else
#define semistatic static
#endif

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <errno.h> 
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ncurses.h>
#include <sys/wait.h>
#include <dirent.h>
#include <ctype.h>
#include <dlfcn.h>
#include <sys/mount.h>

#include "constants.h"
#include "config.h"
#include "store/store.h"
#include "utils/command.h"
#include "utils/disk.h"
#include "utils/dependencies.h"
#include "utils/system.h"
#include "utils/hostname.h"
#include "utils/install_log.h"
#include "phases/phases.h"
#include "phases/partitions/partitions.h"
#include "phases/rootfs/rootfs.h"
#include "phases/bootloader/bootloader.h"
#include "phases/locale/locale.h"
#include "phases/cleanup/cleanup.h"
#include "phases/users/users.h"
#include "phases/fstab/fstab.h"
#include "phases/components/components.h"
#include "steps/steps.h"
#include "ui/ui.h"
#include "ui/modal.h"
#include "ui/elements.h"
#include "ui/colors.h"
#include "steps/locale/locale.h"
#include "steps/disk/disk.h"
#include "steps/partition/table.h"
#include "steps/partition/dialogs.h"
#include "steps/partition/partition.h"
#include "steps/user/table.h"
#include "steps/user/dialogs.h"
#include "steps/user/user.h"
#include "steps/confirm/confirm.h"
#include "steps/confirm/progress.h"
