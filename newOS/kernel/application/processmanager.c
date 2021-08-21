#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

#include "time.h"
#include "termio.h"
// #include "bits/signum.h"
#include "signal.h"
#include "sys/time.h"


/*****************************************************************************
 *                                processManager
 *****************************************************************************/
//进程管理主函数
void runProcessManage(int fd_stdin)
{
	clear();
	char readbuffer[128];
	showProcessWelcome();
	while (1)
	{
		printf("cherryOS ~ process-manager: $ ");

		int end = read(fd_stdin, readbuffer, 70);
		readbuffer[end] = 0;
		int i = 0, j = 0;
		//获得命令指令
		char cmd[20] = {0};
		while (readbuffer[i] != ' ' && readbuffer[i] != 0)
		{
			cmd[i] = readbuffer[i];
			i++;
		}
		i++;
		//获取命令目标
		char target[20] = {0};
		while (readbuffer[i] != ' ' && readbuffer[i] != 0)
		{
			target[j] = readbuffer[i];
			i++;
			j++;
		}
		//结束进程;
		if (strcmp(cmd, "kill") == 0)
		{
			killProcess(target);
			continue;
		}
		//重启进程
		else if (strcmp(cmd, "restart") == 0)
		{
			restartProcess(target);
			continue;
		}
		//弹出提示
		else if (strcmp(readbuffer, "help") == 0)
		{
			clear();
			showProcessWelcome();
		}
		//打印全部进程
		else if (strcmp(readbuffer, "ps") == 0)
		{
			showProcess();
		}
		//退出进程管理
		else if (strcmp(readbuffer, "quit") == 0)
		{
			clear();

			break;
		}
		else if (!strcmp(readbuffer, "clear"))
		{
			clear();
		}
		//错误命令提示
		else
		{
			printf("Sorry, there no such command in the Process Manager.\n");
			printf("You can input [help] to know more.\n");
			printf("\n");
		}
	}
}

//打印欢迎界面
void showProcessWelcome()
{
	printf("      ====================================================================\n");
	printf("      #                            Welcome to                  ******    #\n");
	printf("      #                     cherryOS ~ Process Manager         **   *    #\n");
	printf("      #                                                        ******    #\n");
	printf("      #                                                        **        #\n");
	printf("      #                                                        **        #\n");
	printf("      #             [COMMAND]                 [FUNCTION]                 #\n");
	printf("      #                                                                  #\n");
	printf("      #               $ ps           |     show all process                  #\n");
	printf("      #           $ kill [id]    |     kill a process                    #\n");
	printf("      #           $ restart [id] |     restart a process                 #\n");
	printf("      #           $ quit         |     quit process management system    #\n");
	printf("      #           $ help         |     show command list of this system  #\n");
	printf("      #           $ clear        |     clear the cmd                     #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #               Powered by doubleZ, budi, flyingfish               #\n");
	printf("      #                       ALL RIGHT REVERSED                         #\n");
	printf("      ====================================================================\n");

	printf("\n\n");
}

//打印所有进程
void showProcess()
{
	int i;
	printf("===============================================================================\n");
	printf("    ProcessID    *    ProcessName    *    ProcessPriority    *    Running?           \n");
	//进程号，进程名，优先级，是否在运行
	printf("-------------------------------------------------------------------------------\n");
	for (i = 0; i < NR_TASKS + NR_PROCS; i++) //逐个遍历
	{
		printf("        %d", proc_table[i].pid);
		printf("                 %5s", proc_table[i].name);
		printf("                   %2d", proc_table[i].priority);
		if (proc_table[i].priority == 0)
		{
			printf("                   no\n");
		}
		else
		{
			printf("                   yes\n");
		}
		//printf("        %d                 %s                   %d                   yes\n", proc_table[i].pid, proc_table[i].name, proc_table[i].priority);
	}
	printf("===============================================================================\n\n");
}

int getMag(int n)
{
	int mag = 1;
	for (int i = 0; i < n; i++)
	{
		mag = mag * 10;
	}
	return mag;
}

//计算进程pid
int getPid(char str[])
{
	int length = 0;
	for (; length < MAX_FILENAME_LEN; length++)
	{
		if (str[length] == '\0')
		{
			break;
		}
	}
	int pid = 0;
	for (int i = 0; i < length; i++)
	{
		if (str[i] - '0' > -1 && str[i] - '9' < 1)
		{
			pid = pid + (str[i] + 1 - '1') * getMag(length - 1 - i);
		}
		else
		{
			pid = -1;
			break;
		}
	}
	return pid;
}

//结束进程
void killProcess(char str[])
{
	int pid = getPid(str);

	//健壮性处理以及结束进程
	if (pid >= NR_TASKS + NR_PROCS || pid < 0)
	{
		printf("The pid exceeded the range\n");
	}
	else if (pid < NR_TASKS)
	{
		printf("System tasks cannot be killed.\n");
	}
	else if (proc_table[pid].priority == 0 || proc_table[pid].p_flags == -1)
	{
		printf("Process not found.\n");
	}
	else if (pid == 4 || pid == 6)
	{
		printf("This process cannot be killed.\n");
	}
	else
	{
		proc_table[pid].priority = 0;
		proc_table[pid].p_flags = -1;
		printf("Aim process is killed.\n");
	}

	showProcess();
}

//重启进程
void restartProcess(char str[])
{
	int pid = getPid(str);

	if (pid >= NR_TASKS + NR_PROCS || pid < 0)
	{
		printf("The pid exceeded the range\n");
	}
	else if (proc_table[pid].p_flags != -1)
	{
		printf("This process is already running.\n");
	}
	else
	{
		proc_table[pid].priority = 1;
		proc_table[pid].p_flags = 1;
		printf("Aim process is running.\n");
	}

	showProcess();
}