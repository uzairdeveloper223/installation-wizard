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

#include "store/store.h"
#include "utils/command.h"
#include "utils/disk.h"
#include "utils/hostname.h"
#include "utils/dependencies.h"
#include "operations/log.h"
#include "operations/install.h"
#include "operations/partitions.h"
#include "operations/rootfs.h"
#include "operations/bootloader.h"
#include "operations/locale.h"
#include "operations/cleanup.h"
#include "operations/users.h"
#include "operations/fstab.h"
#include "steps/steps.h"
#include "ui/ui.h"
#include "ui/modal.h"
#include "ui/elements.h"
#include "steps/locale.h"
#include "steps/disk.h"
#include "steps/partition/table.h"
#include "steps/partition/dialogs.h"
#include "steps/partition/partition.h"
#include "steps/user/table.h"
#include "steps/user/dialogs.h"
#include "steps/user/user.h"
#include "steps/confirm/confirm.h"
#include "steps/confirm/progress.h"
