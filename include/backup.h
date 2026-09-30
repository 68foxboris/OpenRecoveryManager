#ifndef RECOVERY_BACKUP_H
#define RECOVERY_BACKUP_H

#include <signal.h>

#include "input.h"
#include "ui.h"

/* Saves the running image as a zip for ofgwrite, like the ImageBackup of enigma2. */
void image_backup(struct ui_context *ui, struct input_context *input, volatile sig_atomic_t *stop);

#endif
