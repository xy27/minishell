#ifndef _DEF_H_
#define _DEF_H_

#include <stdio.h>
#include <stdlib.h>

#define ERR_EXIT(m)			\
	do						\
	{						\
		perror(m);			\
		exit(EXIT_FAILURE);	\
	} while(0)



#define MAXLINE 1024			// 输入行的最大长度
#define MAXARG 20				// 每个简单命令的参数最多个数
#define PIPELINE 5				// 一个管道行中简单命令的最多个数
#define MAXNAME 100				// IO重定向文件名的最大长度

typedef struct command
{
	char *args[MAXARG+1];		// 指向命令以及其参数 的指针数组  ls -l
	int infd;					// 该条命令的输入fd
	int outfd;					// 该条命令的输出fd

	// 在将管道中的命令添加到一个进程组时，该管道用于同步，以及传递进程组ID
	int sync_pipe_read_fd;
	int sync_pipe_write_fd;
} COMMAND;

#endif /* _DEF_H_*/
