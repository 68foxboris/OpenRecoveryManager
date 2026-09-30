#ifndef RECOVERY_UPDATE_H
#define RECOVERY_UPDATE_H

#include <signal.h>

#include "input.h"
#include "ui.h"

/* The online update of enigma2 with opkg: feed status, list, one confirmation,
 * the output of opkg as it comes. The restart is left to the menu. */
void update_packages(const struct ui_context *ui, struct input_context *input,
	const volatile sig_atomic_t *stop);

#endif
