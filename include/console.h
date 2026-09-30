#ifndef RECOVERY_CONSOLE_H
#define RECOVERY_CONSOLE_H

#include <signal.h>
#include <stdint.h>
#include <sys/types.h>

#include "input.h"
#include "qrcodegen.h"
#include "ui.h"

/* A command line program of a plugin on a pseudo terminal. It asks every
 * question as "<text> [y/N] (<seconds> s) " and reads a line; ORM shows the
 * text as it is and answers with the choice on the TV, so a changed wording
 * still works. The first https link becomes a QR code, unless a line
 * "QR: <url>" names another one. */
struct console {
	const struct ui_context *ui;
	struct input_context *input;
	const volatile sig_atomic_t *stop;
	pid_t pid;
	int master;
	char buffer[16384];
	size_t used;
	char link[256];
	uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
	int has_qrcode;
	int qrcode_own;  /* The QR code is of the "QR:" line. */
	void (*line)(const struct console *console, const char *line);  /* Every complete line but the link. */
	void (*question)(const struct console *console, const char *question, int seconds);
	void *data;
};

int console_start(struct console *console, const char *path, char *const argv[]);
/* Reads what came within timeout_ms, 1 when the program ended. */
int console_poll(struct console *console, int timeout_ms);
void console_answer(const struct console *console, int yes);
void console_interrupt(const struct console *console);  /* Ctrl+C */
void console_close(struct console *console);
/* Yes or No on the TV with the countdown of the question, answers it and
 * returns the answer; no answer in time is no, like on the command line. */
int console_ask(const struct console *console, const char *title, const char *question, int seconds);
int console_stopped(const struct console *console);

#endif
