#define _GNU_SOURCE

#include "watch.h"

#include "boxinfo.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#define HANG_SECONDS 120  /* Without a new step while starting. */
#define STABLE_SECONDS 60  /* A crash after this long running is a normal crash, see ORM_STABLE_SECONDS of enigma2. */

static volatile sig_atomic_t watch_stopped;

static void watch_signal(int signal_number)
{
	(void)signal_number;
	watch_stopped = 1;
}

static time_t now_seconds(void)
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return now.tv_sec;
}

static FILE *watch_log;
static struct timespec watch_log_start;

static void log_open(void)
{
	rename(WATCH_LOG, WATCH_LOG_LAST);
	watch_log = fopen(WATCH_LOG, "w");
	if (watch_log)
		setvbuf(watch_log, NULL, _IOLBF, 0);
	clock_gettime(CLOCK_MONOTONIC, &watch_log_start);
}

static void log_line(const char *format, ...) __attribute__((format(printf, 1, 2)));

static void log_line(const char *format, ...)
{
	struct timespec now;
	va_list args;
	if (!watch_log)
		return;
	clock_gettime(CLOCK_MONOTONIC, &now);
	fprintf(watch_log, "%8.3f  ", (double)(now.tv_sec - watch_log_start.tv_sec) +
		(double)(now.tv_nsec - watch_log_start.tv_nsec) / 1e9);
	va_start(args, format);
	vfprintf(watch_log, format, args);
	va_end(args);
	fputc('\n', watch_log);
}

struct watch_state {
	int steps;
	int ready;
	int hang;
	int quit;  /* enigma2 ended on purpose, e.g. restart or shutdown. */
	int quit_code;
	time_t progress;
	time_t ready_time;
	char step[160];
	char crash[32];
};

static void handle_message(struct watch_state *state, const char *message)
{
	if (strncmp(message, "step ", 5) == 0) {
		snprintf(state->step, sizeof(state->step), "%.159s", message + 5);
		state->steps++;
		state->progress = now_seconds();
		log_line("%s", message);
	} else if (strcmp(message, "ready") == 0) {
		state->ready = 1;
		state->ready_time = now_seconds();
		log_line("ready after %d steps", state->steps);
	} else if (strncmp(message, "quit ", 5) == 0) {
		state->quit = 1;
		state->quit_code = atoi(message + 5);
		log_line("%s", message);
	} else if (strncmp(message, "crash ", 6) == 0) {
		snprintf(state->crash, sizeof(state->crash), "%.31s", message + 6);
		log_line("%s", message);
	} else {
		log_line("unknown message: %s", message);
	}
}

static void drain(int fd, struct watch_state *state)
{
	char message[256];
	ssize_t length;
	while ((length = recv(fd, message, sizeof(message) - 1, MSG_DONTWAIT)) > 0) {
		if ((size_t)length >= sizeof(message))
			length = sizeof(message) - 1;
		message[length] = '\0';
		text_cut(message, "\r\n");
		handle_message(state, message);
	}
}

/* The process enigma2 itself; enigma2.sh starts it in a subshell. */
static void kill_enigma2(void)
{
	DIR *proc = opendir("/proc");
	const struct dirent *entry;
	if (!proc)
		return;
	while ((entry = readdir(proc))) {
		char path[64];
		char comm[32] = "";
		FILE *file;
		long pid = strtol(entry->d_name, NULL, 10);
		if (pid <= 0)
			continue;
		snprintf(path, sizeof(path), "/proc/%ld/comm", pid);
		file = fopen(path, "r");
		if (!file)
			continue;
		if (fgets(comm, sizeof(comm), file) && strcmp(comm, "enigma2\n") == 0)
			kill((pid_t)pid, SIGKILL);
		fclose(file);
	}
	closedir(proc);
}

static int write_result(const char *path, const struct watch_state *state)
{
	char temporary[256];
	FILE *file;
	long uptime = state->ready ? (long)(now_seconds() - state->ready_time) : 0;
	const char *reason = "exit";
	int failed;
	/* Code 5 is the Python crash, which reports itself as crash python as well. */
	int clean = state->quit && state->quit_code != 5 && !state->crash[0] && !state->hang;
	if (state->hang)
		reason = "hang";
	else if (state->crash[0])
		reason = "crash";
	else if (clean)
		reason = "quit";
	/* An enigma2 without the start reports never sends a step, it never counts as failed. */
	failed = !clean && (state->steps || state->crash[0]) && (!state->ready || uptime < STABLE_SECONDS);
	snprintf(temporary, sizeof(temporary), "%s.tmp", path);
	file = fopen(temporary, "w");
	if (!file)
		return 0;
	fprintf(file, "failed=%d\nreason=%s\nready=%d\nuptime=%ld\nstep=%s\ncrash=%s\n",
		failed, reason, state->ready, uptime, state->step, state->crash);
	log_line("stopped: failed=%d reason=%s ready=%d uptime=%ld", failed,
		reason, state->ready, uptime);
	if (fclose(file) != 0)
		return 0;
	return rename(temporary, path) == 0;
}

int watch_run(const char *result_path)
{
	struct sockaddr_un address;
	struct sigaction action;
	struct watch_state state;
	pid_t pid;
	int null;
	int fd;
	memset(&state, 0, sizeof(state));
	state.progress = now_seconds();
	memset(&action, 0, sizeof(action));
	action.sa_handler = watch_signal;
	sigemptyset(&action.sa_mask);
	sigaction(SIGTERM, &action, NULL);
	sigaction(SIGINT, &action, NULL);
	fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (fd < 0) {
		perror("socket");
		return 1;
	}
	memset(&address, 0, sizeof(address));
	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, sizeof(address.sun_path), "%s", WATCH_SOCKET);
	unlink(WATCH_SOCKET);  /* Left by an ORM that did not end cleanly. */
	if (bind(fd, (struct sockaddr *)&address, sizeof(address)) != 0) {
		perror("bind");
		close(fd);
		return 1;
	}
	chmod(WATCH_SOCKET, 0600);
	/* Only now in the background: when enigma2.sh has the pid, the socket is there for the first step. */
	pid = fork();
	if (pid < 0) {
		perror("fork");
		close(fd);
		unlink(WATCH_SOCKET);
		return 1;
	}
	if (pid > 0) {
		printf("%d\n", (int)pid);
		return 0;
	}
	/* Else $(...) in enigma2.sh waits for the end of the watch. */
	null = open("/dev/null", O_RDWR);
	if (null >= 0) {
		dup2(null, STDIN_FILENO);
		dup2(null, STDOUT_FILENO);
		dup2(null, STDERR_FILENO);
		if (null > STDERR_FILENO)
			close(null);
	} else {
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
	}
	log_open();
	log_line("watching %s", WATCH_SOCKET);
	while (!watch_stopped) {
		fd_set read_set;
		struct timeval timeout = {1, 0};
		FD_ZERO(&read_set);
		FD_SET(fd, &read_set);
		if (select(fd + 1, &read_set, NULL, NULL, &timeout) > 0)
			drain(fd, &state);
		if (state.steps && !state.ready && !state.hang && !state.crash[0] &&
			now_seconds() - state.progress >= HANG_SECONDS) {
			state.hang = 1;
			log_line("no new step for %d seconds, enigma2 stopped", HANG_SECONDS);
			kill_enigma2();
		}
	}
	drain(fd, &state);  /* A crash report sent just before enigma2 ended. */
	close(fd);
	unlink(WATCH_SOCKET);
	if (!write_result(result_path, &state))
		return 1;
	if (watch_log)
		fclose(watch_log);
	return 0;
}

static int read_comm(const char *path, char *comm, size_t size)
{
	FILE *file = fopen(path, "r");
	int ok;
	if (!file)
		return 0;
	ok = fgets(comm, (int)size, file) != NULL;
	fclose(file);
	return ok;
}

/* The pid may belong to another process by now, when the watch ended early. */
static int is_watch(pid_t pid)
{
	char path[64];
	char comm[32];
	char own[32];
	snprintf(path, sizeof(path), "/proc/%d/comm", (int)pid);
	return pid > 0 && read_comm(path, comm, sizeof(comm)) &&
		read_comm("/proc/self/comm", own, sizeof(own)) && strcmp(comm, own) == 0;
}

void watch_stop(pid_t pid)
{
	const struct timespec step = {0, 50000000L};
	int i;
	if (!is_watch(pid) || kill(pid, SIGTERM) != 0)
		return;
	for (i = 0; i < 100 && kill(pid, 0) == 0; ++i)  /* It writes the result, at most 5 seconds. */
		nanosleep(&step, NULL);
	if (i == 100 && is_watch(pid))  /* Hangs. */
		kill(pid, SIGKILL);
}

int watch_read_result(const char *path, struct watch_result *result)
{
	char line[256];
	FILE *file = fopen(path, "r");
	memset(result, 0, sizeof(*result));
	if (!file)
		return 0;
	while (fgets(line, sizeof(line), file)) {
		char *value = strchr(line, '=');
		if (!value)
			continue;
		*value++ = '\0';
		text_cut(value, "\n");
		if (strcmp(line, "failed") == 0)
			result->failed = atoi(value);
		else if (strcmp(line, "ready") == 0)
			result->ready = atoi(value);
		else if (strcmp(line, "uptime") == 0)
			result->uptime = atol(value);
		else if (strcmp(line, "reason") == 0)
			snprintf(result->reason, sizeof(result->reason), "%s", value);
		else if (strcmp(line, "step") == 0)
			snprintf(result->step, sizeof(result->step), "%s", value);
		else if (strcmp(line, "crash") == 0)
			snprintf(result->crash, sizeof(result->crash), "%s", value);
	}
	fclose(file);
	return 1;
}

const char *watch_signal_name(const char *crash)
{
	switch (atoi(crash)) {
	case SIGSEGV: return "SIGSEGV";
	case SIGABRT: return "SIGABRT";
	case SIGBUS: return "SIGBUS";
	case SIGILL: return "SIGILL";
	case SIGFPE: return "SIGFPE";
	default: return NULL;
	}
}
