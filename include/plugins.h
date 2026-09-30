#ifndef RECOVERY_PLUGINS_H
#define RECOVERY_PLUGINS_H

#include <signal.h>

#include "input.h"
#include "ui.h"

/* A disabled plugin, screen, tool, converter or renderer moves below
 * Plugins.disabled, so enigma2 neither loads nor removes it; enabling moves it
 * back. */
#define DISABLED_ROOT "/usr/lib/enigma2/python/Plugins.disabled"

void disable_plugins(struct ui_context *ui, struct input_context *input,
	volatile sig_atomic_t *stop);

#endif
