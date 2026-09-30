#ifndef RECOVERY_FLASH_H
#define RECOVERY_FLASH_H

#include <signal.h>

#include "input.h"
#include "ui.h"

/* The running slot, like the boot slots of MultiBoot.py; multiboot 0 without slots. */
struct flash_slot {
	int multiboot;
	int ubi;
	char code[8];
	char device[64];  /* /dev/... or ubi0:... */
	char kernel[64];
	char rootsubdir[32];
};

void flash_running_slot(struct flash_slot *slot);
/* Flashes an image of a feed or a local zip into the running slot, like the FlashManager of enigma2. */
void flash_image(struct ui_context *ui, struct input_context *input,
	volatile sig_atomic_t *stop);

#endif
