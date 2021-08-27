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
#include "signal.h"
#include "sys/time.h"


/*****************************************************************************
 *                                processManager
 *****************************************************************************/
void runProcessManage(int fd_stdin)
{
	clear();
	char readbuffer[128];
	showPs();
	while (1)
	{
		printf("UpUpOS ~ process_manager: $ ");

		int nn = read(fd_stdin, readbuffer, 70);
		readbuffer[nn] = 0;
		int i = 0, j = 0;
		int pid;
		char cmd[20] = {0};
		while (readbuffer[i] != ' ' && readbuffer[i] != 0)
		{
			cmd[i] = readbuffer[i];
			i++;
		}
		i++;
		char target[20] = {0};
		while (readbuffer[i] != ' ' && readbuffer[i] != 0)
		{
			target[j] = readbuffer[i];
			i++;
			j++;
		}
		target[j+1]=0;
		atoi(target, &pid);
		if (strcmp(cmd, "kill") == 0)
		{
			killProcess(pid);
			continue;
		}
		//重启进程
		else if (strcmp(cmd, "start") == 0)
		{
			startProcess(pid);
			continue;
		}
		//弹出提示
		else if (strcmp(readbuffer, "help") == 0)
		{
			clear();
			showHelp();
		}
		//打印全部进程
		else if (strcmp(readbuffer, "ps") == 0)
		{
			clear();
			showPs();
		}
		//退出进程管理
		else if (strcmp(readbuffer, "quit") == 0||strcmp(readbuffer, "q") == 0)
		{
			clear();
			break;
		}
		else if (!strcmp(readbuffer, "release"))
		{
			release();
		}
		else
		{
			printf("Input command error.\n");
			printf("input [help] to learn more.\n");
			printf("\n");
		}
	}
}


void showPs()
{
	printf("\n      #==================================================================#\n");
	printf("      #                          Welcome to UpUpOS                       #\n");
	printf("      #            --------------- Process Manager ---------------       #\n");
	printf("      #                                                                  #\n");
	printf("      #   ProcessID          Name          Priority          Running     #\n");
	for (int i = 0; i < NR_TASKS + NR_PROCS; i++)
	{
		printf("      #     %d", proc_table[i].pid);
		printf("                 %5s", proc_table[i].name);
		printf("             %2d", proc_table[i].priority);
		if (proc_table[i].priority == 0)
		{
			printf("              no       #\n");
		}
		else
		{
			printf("              yes      #\n");
		}
	}
	printf("      #==================================================================#\n");
	printf("      #====|   ps  |  kill  |  start  |  quit  |  help  |  release  |====#\n");
	printf("      #==================================================================#\n\n");
}


void showHelp()
{
		printf("      #==================================================================#\n");
		printf("      #                                         Welcome to               #\n");
		printf("      #  ***     ***                               UpUpOS                #\n");
		printf("      #  ***     ***                                                     #\n");
		printf("      #  ***     ***  *********           <Process Manager help>         #\n");
		printf("      #  ***     ***  **       **     You can manage the process here    #\n");
		printf("      #   *********   **       **                                        #\n");
		printf("      #               **       **                                        #\n");
		printf("      #               **********   [OPTION LIST]                         #\n");
		printf("      #               **            ps -> show all process               #\n");
		printf("      #  ***     ***  **            kill [id] -> kill a process          #\n");
		printf("      #  ***     ***                start [id] -> start a process        #\n");
		printf("      #  ***     ***                quit -> quit system                  #\n");
		printf("      #  ***     *** *********      help -> show command list            #\n");
		printf("      #   *********  **       **    release -> kill all free process     #\n");
		printf("      #              **       **                                         #\n");
		printf("      #              **       **                                         #\n");
		printf("      #              **********                                          #\n");
		printf("      #              **                                                  #\n");
		printf("      #              **                                                  #\n");
		printf("      #==================================================================#\n");

}

//结束进程
void killProcess(int pid)
{
	if (pid >= NR_TASKS + NR_PROCS || pid < 0)
	{
		printf("Process does not exist\n");
		return;
	}
	else if (pid < NR_TASKS+1)
	{
		printf("System tasks cannot be killed.\n");
		return;
	}
	else if (proc_table[pid].priority == 0 || proc_table[pid].p_flags == -1)
	{
		printf("Process is already killed.\n");
		return;
	}
	else
	{
		proc_table[pid].priority = 0;
		proc_table[pid].p_flags = -1;
		clear();
		printf("\n                             ----Process is killed----                    ");
	}
	showPs();
}

//重启进程
void startProcess(int pid)
{
	if (pid >= NR_TASKS + NR_PROCS || pid < 0)
	{
		printf("Process does not exist\n");
		return;
	}
	else if (proc_table[pid].p_flags != -1)
	{
		printf("Process is already running.\n");
		return;
	}
	else
	{
		proc_table[pid].priority = 1;
		proc_table[pid].p_flags = 1;
		clear();
        printf("\n                            ----Process is running----                    ");
	}
	showPs();
}
//释放空间
void release()
{
	for(int pid=0;pid<NR_TASKS + NR_PROCS;pid++)
	{
	if (pid < NR_TASKS+1)
	{
		continue;
	}
	else if (proc_table[pid].priority == 0 || proc_table[pid].p_flags == -1)
	{
		continue;
	}
	else if(pid!=8)
	{
		proc_table[pid].priority = 0;
		proc_table[pid].p_flags = -1;
	}
	}
	clear();
	printf("\n                              ----Release finish----                      ");
	showPs();
}
