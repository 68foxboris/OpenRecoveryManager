#ifndef RECOVERY_SLOTS_H
#define RECOVERY_SLOTS_H

#include <signal.h>

#include "input.h"
#include "ui.h"

/* multiboot-selector.sh of oe-alliance/MultiBootSelectorPlugin switches the slot. */
#define MULTIBOOT_SELECTOR "/usr/bin/multiboot-selector.sh"

/* 1 when the receiver has slots to start, found like Enigma2 does. */
int slots_multiboot(void);
/* 1 when the next start of the receiver boots the chosen slot. */
int boot_slot(struct ui_context *ui, struct input_context *input,
	volatile sig_atomic_t *stop);

#endif
