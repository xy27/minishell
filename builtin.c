#include "builtin.h"
#include "parse.h"
#include <stdio.h>
#include <stdlib.h>

typedef void (*CMD_HANDLER)(void);

typedef struct builtin_cmd
{
	char *name;
	CMD_HANDLER handler;
} BUILTIN_CMD;



void do_exit();
void do_cd();
void do_type();

BUILTIN_CMD builtins[] =
{
	{"exit", do_exit},
	{"cd", do_cd},
	{"type", do_type},
	{NULL, NULL}
};

// 内部命令解析
// 返回1表示为内部命令，0表示不是内部命令
int builtin()
{
	int i=0;
	int found=0;
	while (builtins[i].name != NULL)
	{
		if (check(builtins[i].name))
		{
			builtins[i].handler();
			found = 1;
			break;
		}
		++i;
	}
	return found;
}

void do_exit()
{
	printf("exit\n");
	exit(EXIT_SUCCESS);
}

void do_cd()
{
	printf("do_cd...\n");
}

void do_type()
{
	printf("do_type...\n");
}
