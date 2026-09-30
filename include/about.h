#ifndef RECOVERY_ABOUT_H
#define RECOVERY_ABOUT_H

#include <signal.h>

#include "input.h"
#include "ui.h"

/* The components of ORM with their licenses, OK shows the full text of one. */
void about(struct ui_context *ui, struct input_context *input, volatile sig_atomic_t *stop);

#endif
