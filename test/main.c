/*
 * This file is part of ptrace-wrap.
 *
 * ptrace-wrap is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ptrace-wrap is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ptrace-wrap.  If not, see <http://www.gnu.org/licenses/>.
 */

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

void child_callback(void *user) {
	const char *file = user;
	ptrace (PTRACE_TRACEME, 0, NULL, NULL);
	execl (file, file, NULL);
	perror ("execl");
	exit (1);
}

void *print_pid(void *user) {
	const char *t = user;
	printf("%s has pid %lu\n", t, pthread_self ());
}

int main(int argc, const char *argv[]) {
	if (argc != 2) {
		printf("usage: %s [executable or pid]\n", argv[0]);
		return 1;
	}
	const char *tracee = argv[1];

	if (ptrace_wrap_instance_start (&inst) != 0) {
		fprintf (stderr, "ptrace_wrap_instance_start failed.\n");
		return 1;
	}

	print_pid ("main thread");
	ptrace_wrap_func (&inst, print_pid, "ptrace thread");

	long r;

	pid_t pid = (pid_t) atol (tracee);
	if (pid) {
		r = ptrace_wrap (&inst, PTRACE_ATTACH, pid, NULL, NULL);
		if (r < 0) {
			perror ("ptrace");
		}
	} else {
		pid = ptrace_wrap_fork (&inst, child_callback, (void *)tracee);
		if (pid < 0) {
			perror ("fork");
			return 1;
		}
	}
	child_pid = pid;

	int wstatus;
	if (waitpid(pid, &wstatus, 0) < 0) {
		perror ("waitpid");
	}

	if (WIFSTOPPED (wstatus)) {
		printf ("stopped\n");
	}

	while (1) {
		r = ptrace_wrap (&inst, PTRACE_CONT, pid, NULL, NULL);
		if (r < 0) {
			perror ("ptrace");
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
