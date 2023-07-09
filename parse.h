#ifndef _PARSE_H_
#define _PARSE_H_

void shell_loop();
int read_command();
int parse_command();
void execute_command();
int check(const char *str);

#endif /*_PARSE_H_*/
