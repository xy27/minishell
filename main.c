#include "parse.h"
#include "init.h"
#include "def.h"

char g_cmdline[MAXLINE+1];		// ls -l
char g_avline[MAXLINE+1];		// ls\0-l\0
char *g_lineptr;
char *g_avptr;

char g_infile[MAXLINE+1];		// 存储输入文件名
char g_outfile[MAXLINE+1];		// 存储输出文件名
COMMAND g_cmd[PIPELINE];

int g_cmd_count;				// 命令行中包含的命令个数
int g_background;
int g_append; 					// 输出重定向 >> 追加模式

int main()
{
	setup();		// 安装信号
	shell_loop();	// 进入shell循环
	return 0;
}
