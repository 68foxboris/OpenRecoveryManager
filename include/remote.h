#ifndef RECOVERY_REMOTE_H
#define RECOVERY_REMOTE_H

#include <signal.h>
#include <stddef.h>

#include "input.h"
#include "ui.h"

/* The command line of the RemoteSupport plugin, run on a pseudo terminal. */
#define REMOTESUPPORT "/usr/bin/remotesupport"

/* 0 once neither the installed plugin, its copy in RAM nor one installed from the feed started. */
int remote_support_available(void);
/* A session keeps running when its screen is left with BACK. */
int remote_support_running(void);
/* "Users: <names>\nTerminals: <n>" of a running session. */
void remote_support_summary(char *text, size_t size);
/* 1 while a session runs, with its users and open terminals. */
int remote_support_counts(int *users, int *open);

void remote_support(struct ui_context *ui, struct input_context *input,
	volatile sig_atomic_t *stop);

#endif
