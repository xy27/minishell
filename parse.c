#include "parse.h"
#include "externs.h"
#include "init.h"
#include "execute.h"
#include "builtin.h"
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>


void get_command(int i);
void get_name(char *name);
void print_command();

void shell_loop()
{
	while (1)
	{
		init();						// 初始化环境
		if (read_command() == -1)	// 获取命令
		{
			break;
		}
		parse_command();			// 解析命令
		// print_command(); 			// 打印命令，用于调试
		execute_command();			// 执行命令*/
	}
	printf("\nexit\n");
}

// 读取命令
// 成功返回0，失败或者读取到文件结束符(EOF)返回-1
int read_command()
{
	if (fgets(g_cmdline, MAXLINE, stdin) == NULL)
	{
		return -1;
	}
	return 0;
}


// 解析命令
// 成功返回解析到的命令个数，失败返回-1
int parse_command()
{
	if (check("\n"))
		return 0;

	if (builtin())			// 先判断是内部命令并执行它
		return 0;

	get_command(0);			// 1.解析第一条简单命

	if (check("<"))			// 2.判定是否有输入重定向符
	{
		get_name(g_infile);
	}

	int i;
	for (i=1; i<PIPELINE; ++i)		// 3.判定是否有管道
	{
		if (check("|"))
			get_command(i);
		else
			break;
	}

	if (check(">"))					// 4.判定是否有输出重定向符
	{
		if (check(">"))
			g_append = 1;
		get_name(g_outfile);
	}

	if (check("&"))					// 5.判定是否后台作业
		g_background = 1;

	if (check("\n"))				// 6.判定命令结束
	{
		g_cmd_count = i;
		return g_cmd_count;
	}
	else
	{
		fprintf(stderr,"Command line syntax error\n");
		return -1;
	}
}

void execute_command()
{
	execute_disk_command();
}

// 解析简单命令至g_cmd[i]中
// 提取g_cmdline中的命令参数到g_avline数组中
// 并且将COMMAND结构中的args[]中的每个指针指向这些字符
void get_command(int i)
{
	int j = 0;
	int inword; 	// ??? 一个flag

	while (*g_lineptr != '\0')
	{
		// 去除空格
		while (*g_lineptr == ' ' || *g_lineptr == '\t')
			++g_lineptr;

		g_cmd[i].args[j] = g_avptr;				// 将第i条命令第j个参数指向avptr

		// 提取参数
		while ( *g_lineptr != '\0'
		     && *g_lineptr != ' '
		     && *g_lineptr != '\t'
		     && *g_lineptr != '>'
		     && *g_lineptr != '<'
		     && *g_lineptr != '|'
		     && *g_lineptr != '&'
		     && *g_lineptr != '\n')
		{
			*g_avptr++ = *g_lineptr++;			// 命令及参数提取至avptr指针所指向的数组avline
			inword = 1;
		}
		*g_avptr++ = '\0';

		switch (*g_lineptr)
		{
			case ' ':			// 后面可能还有命令相关的参数，重定向符号，管道符号，该命令(输入的一串命令中的某一个)还需继续解析
			case '\t':
				inword = 0;
				++j;
				break;
			case '<':			// 该命令所有参数已近解析完毕，后面可能有重定向文件，其它命令，或者是最后一条命令
			case '>':
			case '|':
			case '&':
			case '\n':
				if (inword == 0)	// 该命令已解析完毕，重置最后一个args指针为NULL
					g_cmd[i].args[j] = NULL;
				return;
			default:
				return;
		}
	}
}

// 将g_lineptr中的字符串与str进行匹配
// 成功返回1，g_lineptr移过所匹配的字符串
// 失败返回0，g_lineptr保持不变
int check(const char *str)
{
	while (*g_lineptr == ' ' || *g_lineptr == '\t')
		++g_lineptr;
	char *p = g_lineptr;

	while (*str != '\0' && *str == *p)
	{
		++str;
		++p;
	}
	if (*str=='\0')
	{
		g_lineptr = p;
		return 1;
	}
	return 0;
}

void get_name(char *name)
{
	while (*g_lineptr == ' ' || *g_lineptr == '\t')
		++g_lineptr;

	while ( *g_lineptr != '\0'
	     && *g_lineptr != ' '
	     && *g_lineptr != '\t'
	     && *g_lineptr != '>'
	     && *g_lineptr != '<'
	     && *g_lineptr != '|'
	     && *g_lineptr != '&'
	     && *g_lineptr != '\n')
	{
		*name++ = *g_lineptr++;
	}
	*name = '\0';
}

// for debug
void print_command()
{
	printf("cmd_count=%d\n", g_cmd_count);
	if (g_infile[0] != '\0')
		printf("g_infile=[%s]\n", g_infile);
	if (g_outfile[0] != '\0')
		printf("outfile=[%s]\n", g_outfile);

	for(int i=0; i<g_cmd_count; ++i)
	{
		int j = 0;
		while (g_cmd[i].args[j] != NULL)
		{
			printf("[%s]", g_cmd[i].args[j]);
			++j;
		}
		printf("\n");
	}
}
