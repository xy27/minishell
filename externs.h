#ifndef _EXTERNS_H_
#define _EXTERNS_H_

#include "def.h"

extern char g_cmdline[MAXLINE+1];
extern char g_avline[MAXLINE+1];

extern char *g_lineptr;
extern char *g_avptr;

extern char g_infile[MAXLINE+1];
extern char g_outfile[MAXLINE+1];

extern COMMAND g_cmd[PIPELINE];

extern int g_cmd_count;
extern int g_background;
extern int g_append;

#endif /* _EXTERNS_H_*/
