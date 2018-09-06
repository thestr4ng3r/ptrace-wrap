
#include <stdio.h>
#include <ptrace_wrap.h>
#include <zconf.h>
#include <stdlib.h>
#include <wait.h>
#include <errno.h>
#include <string.h>

ptrace_wrap_instance inst;
pid_t child_pid;

void handle_sigint(int v) {
	kill (child_pid, SIGKILL);
}

int main(int argc, const char *argv[]) {
	if (argc != 2) {
		printf("usage: %s [executable]\n", argv[0]);
		return 1;
	}
	const char *file = argv[1];

	pid_t pid = fork();
	if (pid < 0) {
		perror ("fork");
		return 1;
	}
	if (pid == 0) {
		execl (file, file, NULL);
		perror ("execl");
		exit (1);
	}
	child_pid = pid;

	if (ptrace_wrap_instance_start (&inst) != 0) {
		fprintf (stderr, "ptrace_wrap_instance_start failed.\n");
		return 1;
	}

	int _errno;
	long r = ptrace_wrap (&inst, PTRACE_ATTACH, pid, NULL, NULL, &_errno);
	if (r < 0) {
		printf ("ptrace err %s\n", strerror(_errno));
	}

	int wstatus;
	if (waitpid(pid, &wstatus, 0) < 0) {
		perror ("waitpid");
	}

	if (WIFSTOPPED (wstatus)) {
		printf ("stopped\n");
	}

	while (1) {
		r = ptrace_wrap (&inst, PTRACE_CONT, pid, NULL, NULL, &_errno);
		if (r < 0) {
			printf ("ptrace err %s\n", strerror(_errno));
			break;
		}

		signal (SIGINT, handle_sigint);
		if (waitpid(pid, &wstatus, 0) < 0) {
			perror ("waitpid");
			break;
		}
		signal (SIGINT, SIG_IGN);

		if (WIFEXITED (wstatus)) {
			break;
		}

		if (WIFSTOPPED (wstatus)) {
			printf ("stopped\n");
		}
	}

	ptrace_wrap_instance_stop (&inst);

	return 0;
}
