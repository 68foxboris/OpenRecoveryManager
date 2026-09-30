#ifndef RECOVERY_RESET_H
#define RECOVERY_RESET_H

#include <signal.h>

#include "input.h"
#include "ui.h"

/* Removes the empty restore flags of the last online flash, like "Clean" in the FlashManager. */
void remove_restore_flags(void);
/* Moves the settings of enigma2 aside, so it starts with the wizard. */
void reset_settings(struct ui_context *ui, struct input_context *input,
	volatile sig_atomic_t *stop);

#endif
