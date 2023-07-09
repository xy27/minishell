#include "execute.h"
#include "def.h"
#include "externs.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <assert.h>


void forkexec(int i)
{
	pid_t pid = fork();
	if (pid == -1)
		ERR_EXIT("fork");
	if (pid > 0)
	{

	}
	else if(pid == 0)
	{
		if (signal(SIGTTIN, SIG_IGN) == SIG_ERR)
		{
			ERR_EXIT("signal SIGTTIN");
		}

		if (signal(SIGTTOU, SIG_IGN) == SIG_ERR)
		{
			ERR_EXIT("signal SIGTTOU");
		}


		// backgnd=1时，将第一条简单命令的infd重定向至/dev/null
		// 当第一条命令试图从标准输入获取数据的时候立即返回EOF
		// 简化作业控制

		// 输入重定向到文件呢？？？
		// if(cmd[i].infd==0&&backgnd==1)
		// 	cmd[i].infd=open("/dev/null",O_RDONLY);

		// 我没有采用上面的做法，直接让它出错


		// 仅仅将第一个命令作为新的进程组组长是不够的，还需要管道中的其它进程也添加到这个进程组中
		// 有多个子进程时，不一定是第一个子进程先运行
		if (i == 0)
		{
			setpgid(0,0);					// 第一条命令所在进程，作为新进程组组长
			pid_t pid = getpid();

			if (tcsetpgrp(0, pid) < 0)		// (默认)把新进程组设置为前台进程组
			{
				ERR_EXIT("tcsetpgrp");
			}

			if (i != g_cmd_count-1)			// 不是最后一个命令
			{
				int ret = write(g_cmd[i].sync_pipe_write_fd, &pid, sizeof(pid));
				assert(ret == sizeof(pid));
			}
		}
		else
		{
			pid_t pgid;
			int ret = read(g_cmd[i].sync_pipe_read_fd, &pgid, sizeof(pgid));
			assert(ret == sizeof(pgid));

			if (i != g_cmd_count-1)			// 不是最后一个命令
			{
				ret = write(g_cmd[i].sync_pipe_write_fd, &pgid, sizeof(pgid));
				assert(ret == sizeof(pgid));
			}
			if (setpgid(getpid(), pgid) < 0)
			{
				ERR_EXIT("setpgid");
			}
		}

		if (g_cmd[i].infd != 0)				// 输入是否有重定向
		{
			dup2(g_cmd[i].infd, 0);
		}
		if (g_cmd[i].outfd != 1)			// 输出是否有重定向
		{
			dup2(g_cmd[i].outfd, 1);
		}

		for (int j=3; j<FOPEN_MAX; ++j)		// 管道中的命令只需 输入 输出就可以了，前面的操作已经完成输入输出的重定向，后序的描述符可以关闭
			close(j);
		
		// 前台作业能够接受SIGINT,SIGQUIT信号
		// 这两个信号恢复默认操作
		if(!g_background)
		{
			signal(SIGINT, SIG_DFL);
			signal(SIGQUIT, SIG_DFL);
		}
		else
		{
			if (tcsetpgrp(0, getppid()) < 0)	// 把父进程所在进程组设为前台进程组
			{
				ERR_EXIT("tcsetpgrp");
			}
		}
		execvp(g_cmd[i].args[0], g_cmd[i].args);
		fprintf(stderr, "failure\n");
		exit(EXIT_FAILURE);
	}
}

void execute_disk_command()
{
	if (g_cmd_count == 0)
		return;

	if (g_infile[0] != '\0')
	{
		g_cmd[0].infd = open(g_infile, O_RDONLY);
	}
	if (g_outfile[0] != '\0')
	{
		if (g_append)
		{
			g_cmd[g_cmd_count - 1].outfd = open(g_outfile, O_WRONLY | O_CREAT | O_APPEND, 0666);
		}
		else
		{
			g_cmd[g_cmd_count - 1].outfd = open(g_outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
		}
	}

	// 在后台运行时，父进程不会调用wait等待子进程退出
	// 为避免僵死进程，可以忽略SIGCHLD信号
	if (g_background)
	{
		signal(SIGCHLD, SIG_IGN);
	}
	else
	{
		signal(SIGCHLD, SIG_DFL); // SIG_DFL 与SIG_IGN的区别是? SIG_DFL也是忽略该信号，但是显示设置SIG_IGN是不会导致僵死进程的
	}

	int fd;
	int fds[2];
	int fd2s[2];
	for (int i=0; i<g_cmd_count; ++i)
	{
		// 如果不是最后一条命令，则需要创建管道
		// 这些管道对应的文件描述符会复制到子进程中
		if (i < g_cmd_count-1)
		{
			pipe(fds);
			g_cmd[i].outfd = fds[1]; 	// 前一条命令输出到管道
			g_cmd[i + 1].infd = fds[0]; // 后一条命令从管道输入

			pipe(fd2s); 				// 管道之间同步用的，主要是用来设置子进程在一个进程组内
			g_cmd[i].sync_pipe_write_fd = fd2s[1];
			g_cmd[i + 1].sync_pipe_read_fd = fd2s[0];
		}

		forkexec(i);
		
		if ((fd = g_cmd[i].infd) !=0)	// 如果定向到文件或者管道的读端，父进程(minishell进程)不需要继续打开这些文件描述符(子进程已复制)，可以直接关闭
			close(fd);
		if ((fd = g_cmd[i].outfd) !=1)	// 如果定向到文件或者管道的写端，父进程(minishell进程)不需要继续打开这些文件描述符(子进程已复制)，可以直接关闭
			close(fd);

		if ((fd = g_cmd[i].sync_pipe_read_fd) !=0)
			close(fd);
		if ((fd = g_cmd[i].sync_pipe_write_fd) !=1)
			close(fd);		
	}

		// if (signal(SIGTTIN, SIG_IGN) == SIG_ERR)
		// {
		// 	ERR_EXIT("signal SIGTTIN");
		// }

		// if (signal(SIGTTOU, SIG_IGN) == SIG_ERR)
		// {
		// 	ERR_EXIT("signal SIGTTOU");
		// }

	if(!g_background)
	{
		if (signal(SIGTTIN, SIG_IGN) == SIG_ERR)
		{
			ERR_EXIT("signal SIGTTIN");
		}

		if (signal(SIGTTOU, SIG_IGN) == SIG_ERR)
		{
			ERR_EXIT("signal SIGTTOU");
		}

		while (g_cmd_count > 0)
		{
			wait(NULL);
			--g_cmd_count;
		}

		if (tcsetpgrp(0, getpid()) < 0)		// 回收完子进程后，重新设置minishell进程为前台进程
		{
			ERR_EXIT("tcsetpgrp 111");
		}
	}
	else
	{
		if (tcsetpgrp(0, getpid()) <0)		// 没有忽略SIGTTIN，SIGTTOU之类的信号，直接调用会不会出错？
		{
			ERR_EXIT("tcsetpgrp");
		}
	}
}
