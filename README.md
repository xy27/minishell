## minishell - 简单的shell

### 实现的功能

	简单命令解析
	输入输出重定向
	管道行解析
	简单内置命令
	信号处理 进程组处理
 ### 示例
	cmd [ < filename ] [ | cmd ]…[ or filename ] [ & ]
	cat < infile | sort | uniq | wc -l > outfile
	
	bash minishell 以及管道行中的命令属于不同的进程组
	cat sort uniq wc -l 在前台进程组，cat为进程组组长