#ifndef RECOVERY_WATCH_H
#define RECOVERY_WATCH_H

#include <stddef.h>
#include <sys/types.h>

/* enigma2 reports its start here: "step <name>", "ready", "quit <code>" when it ends on
 * purpose and "crash <signal>" or "crash python" from its crash handler. */
#define WATCH_SOCKET "/var/run/enigma2-orm.socket"
/* Every message with its time, for debugging; the one of the previous start is .last. */
#define WATCH_LOG "/tmp/orm.log"
#define WATCH_LOG_LAST WATCH_LOG ".last"
#define WATCH_RESULT "/tmp/orm.result"  /* Of the last start, enigma2.sh passes it to --crash. */

struct watch_result {
	int failed;
	int ready;
	long uptime;
	char reason[16];
	char step[160];
	char crash[32];
};

/* Listens, then goes into the background and prints its pid. */
int watch_run(const char *result_path);
/* Ends the watch of pid and waits until it wrote the result. */
void watch_stop(pid_t pid);
int watch_read_result(const char *path, struct watch_result *result);
/* SIGSEGV for "11" of crash=, NULL when unknown. */
const char *watch_signal_name(const char *crash);

#endif
