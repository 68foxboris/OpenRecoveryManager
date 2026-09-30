#ifndef RECOVERY_LANGUAGE_H
#define RECOVERY_LANGUAGE_H

#include <signal.h>

#include "input.h"
#include "ui.h"

/* The languages ORM has texts for, OK uses one until ORM ends; Enigma2 keeps its own. */
void language_choose(const struct ui_context *ui, struct input_context *input, const volatile sig_atomic_t *stop);
/* 1 when there is another language than English, with its catalog and its locale. */
int language_other(void);

#endif
