#ifndef RECOVERY_PROCESS_H
#define RECOVERY_PROCESS_H

#include <stddef.h>

typedef void (*process_line_cb)(const char *line, void *opaque);
typedef void (*process_tick_cb)(void *opaque);

int process_run(char *const argv[], const char *stdin_text,
	process_line_cb callback, void *opaque);
int process_run_with_updates(char *const argv[], const char *stdin_text,
	process_line_cb callback, process_tick_cb tick, unsigned int tick_ms,
	void *opaque);
/* Ends the command process_run_with_updates() runs, with everything it started; from its tick. */
void process_cancel(void);
int process_capture(char *const argv[], char *output, size_t output_size);
int process_find(const char *name, char *path, size_t path_size);

#endif
