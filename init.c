#include "init.h"
#include "externs.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>

void sigint_handler(int signum);

void setup()
{
	signal(SIGINT, sigint_handler);
	signal(SIGQUIT, SIG_IGN);
}

void sigint_handler(int signum)
{
	fflush(stdout);
	printf("\n[minishell]$");
	fflush(stdout);
}

void init()
{
	memset(g_cmd, 0, sizeof(g_cmd));
	for (int i = 0; i < PIPELINE; ++i)
	{
		g_cmd[i].infd = 0;
		g_cmd[i].outfd = 1;

		g_cmd[i].sync_pipe_read_fd = 0;
		g_cmd[i].sync_pipe_write_fd = 1;
	}

	memset(g_cmdline, 0, sizeof(g_cmdline));
	memset(g_avline, 0, sizeof(g_avline));
	g_lineptr = g_cmdline;
	g_avptr = g_avline;
	memset(g_infile, 0, sizeof(g_infile));
	memset(g_outfile, 0, sizeof(g_outfile));
	g_cmd_count = 0;
	g_background = 0;
	g_append = 0;

	printf("[minishell]$");
	fflush(stdout);
}
