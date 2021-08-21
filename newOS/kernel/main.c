
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
							main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

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

#define DELAY_TIME 6000 //延迟时间
#define NULL ((void *)0)

// my code here

/*======================================================================*
							文件系统
 *======================================================================*/
#define MAX_FILE_PER_LAYER 10
#define MAX_FILE_NAME_LENGTH 20
#define MAX_CONTENT_ 50
#define MAX_FILE_NUM 100

//文件ID计数器
int fileIDCount = 0;
int currentFileID = 0;

struct fileBlock
{
	int fileID;
	char fileName[MAX_FILE_NAME_LENGTH];
	int fileType; //0 for txt, 1 for folder
	char content[MAX_CONTENT_];
	int fatherID;
	int children[MAX_FILE_PER_LAYER];
	int childrenNumber;
};
struct fileBlock blocks[MAX_FILE_NUM];
int IDLog[MAX_FILE_NUM];

//文件管理主函数
void runFileManage(int fd_stdin)
{
	char rdbuf[128];
	char cmd[8];
	char filename[120];
	char buf[1024];
	int m, n;
	char _name[MAX_FILE_NAME_LENGTH];
	FSInit();
	int len = ReadDisk();
	ShowMessage();

	while (1)
	{
		for (int i = 0; i <= 127; i++)
			rdbuf[i] = '\0';
		for (int i = 0; i < MAX_FILE_NAME_LENGTH; i++)
			_name[i] = '\0';
		printf("\n/%s:", blocks[currentFileID].fileName);

		int r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;
		assert(fd_stdin == 0);

		char target[10];
		for (int i = 0; i <= 1 && i < r; i++)
		{
			target[i] = rdbuf[i];
		}
		if (rdbuf[0] == 't' && rdbuf[1] == 'o' && rdbuf[2] == 'u' && rdbuf[3] == 'c' && rdbuf[4] == 'h')
		{
			if (rdbuf[5] != ' ')
			{
				printf("You should add the filename, like \"create XXX\".\n");
				printf("Please input [help] to know more.\n");
				continue;
			}
			for (int i = 0; i < MAX_FILE_NAME_LENGTH && i < r - 3; i++)
			{
				_name[i] = rdbuf[i + 6];
			}
			CreateFIle(_name, 0);
		}
		else if (rdbuf[0] == 'm' && rdbuf[1] == 'k' && rdbuf[2] == 'd' && rdbuf[3] == 'i' && rdbuf[4] == 'r')
		{
			if (rdbuf[5] != ' ')
			{
				printf("You should add the dirname, like \"mkdir XXX\".\n");
				printf("Please input [help] to know more.\n");
				continue;
			}
			char N[MAX_FILE_NAME_LENGTH];
			for (int i = 0; i < MAX_FILE_NAME_LENGTH && i < r - 3; i++)
			{
				_name[i] = rdbuf[i + 6];
			}
			CreateFIle(_name, 1);
		}
		else if (strcmp(rdbuf, "ls") == 0)
		{
			showFileList();
		}
		else if (strcmp(target, "cd") == 0)
		{
			if (rdbuf[2] == ' ' && rdbuf[3] == '.' && rdbuf[4] == '.')
			{
				ReturnFile(currentFileID);
				continue;
			}
			else if (rdbuf[2] != ' ')
			{
				printf("You should add the dirname, like \"cd XXX\".\n");
				printf("Please input [help] to know more.\n");

				continue;
			}
			for (int i = 0; i < MAX_FILE_NAME_LENGTH && i < r - 3; i++)
			{
				_name[i] = rdbuf[i + 3];
			}
			printf("name: %s\n", _name);
			int ID = SearchFile(_name);
			if (ID >= 0)
			{
				if (blocks[ID].fileType == 1)
				{
					currentFileID = ID;
					continue;
				}
				else if (blocks[ID].fileType == 0)
				{
					while (1)
					{
						printf("input the character representing the method you want to operate:"
							   "\nu --- update"
							   "\nd --- detail of the content"
							   "\nq --- quit\n");
						int r = read(fd_stdin, rdbuf, 70);
						rdbuf[r] = 0;
						if (strcmp(rdbuf, "u") == 0)
						{
							printf("input the text you want to write:\n");
							int r = read(fd_stdin, blocks[ID].content, MAX_CONTENT_);
							blocks[ID].content[r] = 0;
						}
						else if (strcmp(rdbuf, "d") == 0)
						{
							printf("--------------------------------------------"
								   "\n%s\n-------------------------------------\n",
								   blocks[ID].content);
						}
						else if (strcmp(rdbuf, "q") == 0)
						{
							printf("would you like to save the change? y/n");
							int r = read(fd_stdin, rdbuf, 70);
							rdbuf[r] = 0;
							if (strcmp(rdbuf, "y") == 0)
							{
								printf("save changes!");
							}
							else
							{
								printf("quit without changing");
							}
							break;
						}
					}
				}
			}
			else
				printf("No such file!");
		}
		else if (strcmp(target, "rm") == 0)
		{
			if (rdbuf[2] != ' ')
			{
				printf("You should add the filename or dirname, like \"rm XXX\".\n");
				printf("Please input [help] to know more.\n");
				continue;
			}
			for (int i = 0; i < MAX_FILE_NAME_LENGTH && i < r - 3; i++)
			{
				_name[i] = rdbuf[i + 3];
			}
			int ID = SearchFile(_name);
			if (ID >= 0)
			{
				printf("Delete successfully!\n");
				DeleteFile(ID);
				for (int i = 0; i < blocks[currentFileID].childrenNumber; i++)
				{
					if (ID == blocks[currentFileID].children[i])
					{
						for (int j = i + 1; j < blocks[currentFileID].childrenNumber; j++)
						{
							blocks[currentFileID].children[i] = blocks[currentFileID].children[j];
						}
						blocks[currentFileID].childrenNumber--;
						break;
					}
				}
			}
			else
				printf("No such file!\n");
		}
		else if (strcmp(target, "sv") == 0)
		{
			WriteDisk(1000);
			printf("Save to disk successfully!\n");
		}
		else if (strcmp(rdbuf, "help") == 0)
		{
			printf("\n");
			ShowMessage();
		}
		else if (strcmp(rdbuf, "quit") == 0)
		{
			clear();
			break;
		}
		else if (!strcmp(rdbuf, "clear"))
		{
			clear();
		}
		else
		{
			printf("Sorry, there no such command in the File Manager.\n");
			printf("You can input [help] to know more.\n");
		}
	}
}

void initFileBlock(int fileID, char *fileName, int fileType)
{
	blocks[fileID].fileID = fileID;
	strcpy(blocks[fileID].fileName, fileName);
	blocks[fileID].fileType = fileType;
	blocks[fileID].fatherID = currentFileID;
	blocks[fileID].childrenNumber = 0;
}

void toStr3(char *temp, int i)
{
	if (i / 100 < 0)
		temp[0] = (char)48;
	else
		temp[0] = (char)(i / 100 + 48);
	if ((i % 100) / 10 < 0)
		temp[1] = '0';
	else
		temp[1] = (char)((i % 100) / 10 + 48);
	temp[2] = (char)(i % 10 + 48);
}

void WriteDisk(int len)
{
	char temp[MAX_FILE_NUM * 150 + 103];
	int i = 0;
	temp[i] = '^';
	i++;
	toStr3(temp + i, fileIDCount);
	i = i + 3;
	temp[i] = '^';
	i++;
	for (int j = 0; j < MAX_FILE_NUM; j++)
	{
		if (IDLog[j] == 1)
		{
			toStr3(temp + i, blocks[j].fileID);
			i = i + 3;
			temp[i] = '^';
			i++;
			for (int h = 0; h < strlen(blocks[j].fileName); h++)
			{
				temp[i + h] = blocks[j].fileName[h];
				if (blocks[j].fileName[h] == '^')
					temp[i + h] = (char)1;
			}
			i = i + strlen(blocks[j].fileName);
			temp[i] = '^';
			i++;
			temp[i] = (char)(blocks[j].fileType + 48);
			i++;
			temp[i] = '^';
			i++;
			for (int h = 0; h < strlen(blocks[j].content); h++)
			{
				temp[i + h] = blocks[j].content[h];
				if (blocks[j].content[h] == '^')
					temp[i + h] = (char)1;
			}
			i = i + strlen(blocks[j].content);
			temp[i] = '^';
			i++;
			toStr3(temp + i, blocks[j].fatherID);
			i = i + 3;
			temp[i] = '^';
			i++;
			for (int m = 0; m < MAX_FILE_PER_LAYER; m++)
			{
				toStr3(temp + i, blocks[j].children[m]);
				i = i + 3;
			}
			temp[i] = '^';
			i++;
			toStr3(temp + i, blocks[j].childrenNumber);
			i = i + 3;
			temp[i] = '^';
			i++;
		}
	}
	int fd = 0;
	int n1 = 0;
	fd = open("ss", O_RDWR);
	assert(fd != -1);
	n1 = write(fd, temp, strlen(temp));
	assert(n1 == strlen(temp));
	close(fd);
}

int toInt(char *temp)
{
	int result = 0;
	for (int i = 0; i < 3; i++)
		result = result * 10 + (int)temp[i] - 48;
	return result;
}

int ReadDisk()
{
	char bufr[1000];
	int fd = 0;
	int n1 = 0;
	fd = open("ss", O_RDWR);
	assert(fd != -1);
	n1 = read(fd, bufr, 1000);
	assert(n1 == 1000);
	bufr[n1] = 0;
	int r = 1;
	fileIDCount = toInt(bufr + r);
	r = r + 4;
	for (int i = 0; i < fileIDCount; i++)
	{
		int ID = toInt(bufr + r);
		IDLog[ID] = 1;
		blocks[ID].fileID = ID;
		r = r + 4;
		for (int i = 0; i < MAX_FILE_NAME_LENGTH; i++)
		{
			if (bufr[r] == '^')
				break;
			else if (bufr[r] == (char)1)
				blocks[ID].fileName[i] = '^';
			else
				blocks[ID].fileName[i] = bufr[r];
			r++;
		}
		r++;
		blocks[ID].fileType = (int)bufr[r] - 48;
		r = r + 2;
		for (int j = 0; j < MAX_CONTENT_; j++)
		{
			if (bufr[r] == '^')
				break;
			else if (bufr[r] == (char)1)
				blocks[ID].content[j] = '^';
			else
				blocks[ID].content[j] = bufr[r];
			r++;
		}
		r++;
		blocks[ID].fatherID = toInt(bufr + r);
		r = r + 4;
		for (int j = 0; j < MAX_FILE_PER_LAYER; j++)
		{
			blocks[ID].children[j] = toInt(bufr + r);
			r = r + 3;
		}
		r++;
		blocks[ID].childrenNumber = toInt(bufr + r);
		r = r + 4;
	}
	return n1;
}

void FSInit()
{

	for (int i = 0; i < MAX_FILE_NUM; i++)
	{
		blocks[i].childrenNumber = 0;
		blocks[i].fileID = -2;
		IDLog[i] = '\0';
	}
	IDLog[0] = 1;
	blocks[0].fileID = 0;
	strcpy(blocks[0].fileName, "home");
	strcpy(blocks[0].content, "welcome to use file system!");
	blocks[0].fileType = 2;
	blocks[0].fatherID = 0;
	blocks[0].childrenNumber = 0;
	currentFileID = 0;
	fileIDCount = 1;
}

int CreateFIle(char *fileName, int fileType)
{
	if (blocks[currentFileID].childrenNumber == MAX_FILE_PER_LAYER)
	{
		printf("Sorry you cannot add more files in this layer.\n");
		return 0;
	}
	else
	{
		for (int i = 0; i < blocks[currentFileID].childrenNumber; i++)
		{
			if (strcmp(blocks[blocks[currentFileID].children[i]].fileName, fileName) == 0)
			{
				if (fileType)
				{
					printf("You have a folder of same name!\n");
				}
				else
				{
					printf("You have a file of same name!\n");
				}
				return 0;
			}
		}
		fileIDCount++;
		int target = 0;
		for (int i = 0; i < MAX_FILE_NUM; i++)
		{
			if (IDLog[i] == 0)
			{
				target = i;
				break;
			}
		}
		initFileBlock(target, fileName, fileType);
		blocks[currentFileID].children[blocks[currentFileID].childrenNumber] = target;
		blocks[currentFileID].childrenNumber++;
		if (fileType)
		{
			printf("Create directory %s successful!\n", fileName);
		}
		else
		{
			printf("Create file %s successful!\n", fileName);
		}
		IDLog[target] = 1;
		return 1;
	}
}

void showFileList()
{
	printf("The elements in %s.\n", blocks[currentFileID].fileName); //通过currentFileID获取当前路径s

	printf("-----------------------------------------\n");
	printf("  filename |    type   | id  \n");
	for (int i = 0; i < blocks[currentFileID].childrenNumber; i++)
	{ //遍历每个孩子
		printf("%10s", blocks[blocks[currentFileID].children[i]].fileName);
		if (blocks[blocks[currentFileID].children[i]].fileType == 0)
		{
			printf(" | .txt file |");
		}
		else
		{
			printf(" |   folder  |");
		}
		printf("%3d\n", blocks[blocks[currentFileID].children[i]].fileID);
	}
	printf("-----------------------------------------\n");
}

int SearchFile(char *name)
{
	for (int i = 0; i < blocks[currentFileID].childrenNumber; i++)
	{
		if (strcmp(name, blocks[blocks[currentFileID].children[i]].fileName) == 0)
		{
			return blocks[currentFileID].children[i];
		}
	}
	return -2;
}

void ReturnFile(int ID)
{
	currentFileID = blocks[ID].fatherID;
}

void DeleteFile(int ID)
{
	if (blocks[ID].childrenNumber > 0)
	{
		for (int i = 0; i < blocks[ID].childrenNumber; i++)
		{
			DeleteFile(blocks[blocks[ID].children[i]].fileID);
		}
	}
	IDLog[ID] = 0;
	blocks[ID].fileID = -2;
	blocks[ID].childrenNumber = 0;
	for (int i = 0; i < MAX_CONTENT_; i++)
		blocks[ID].content[i] = '\0';
	for (int i = 0; i < MAX_FILE_NAME_LENGTH; i++)
		blocks[ID].fileName[i] = '\0';
	blocks[ID].fileType = -1;
	for (int i = 0; i < MAX_FILE_PER_LAYER; i++)
		blocks[ID].children[i] = -1;
	blocks[ID].fatherID = -2;
	fileIDCount--;
}

void ShowMessage()
{
	printf("      ====================================================================\n");
	printf("      #                            Welcome to                  ******    #\n");
	printf("      #                     cherryOS ~ File Manager            **        #\n");
	printf("      #                                                        ******    #\n");
	printf("      #                                                        **        #\n");
	printf("      #         [COMMAND]                 [FUNCTION]           **        #\n");
	printf("      #                                                                  #\n");
	printf("      #     $ touch [filename]  |   create a new .txt file               #\n");
	printf("      #     $ mkdir [dirname]   |   create a new folder                  #\n");
	printf("      #     $ ls                |   list the elements in this level      #\n");
	printf("      #     $ cd [dirname]      |   switch work path to this directory   #\n");
	printf("      #     $ cd ..             |   return to the superior directory     #\n");
	printf("      #     $ rm [name]         |   delete a file or directory           #\n");
	printf("      #     $ help              |   show command list of this system     #\n");
	printf("      #     $ clear             |   clear the cmd                        #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #               Powered by doubleZ, budi, flyingfish               #\n");
	printf("      #                       ALL RIGHT REVERSED                         #\n");
	printf("      ====================================================================\n");

	printf("\n\n");
}

/*======================================================================*
							kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	struct task *p_task;
	struct proc *p_proc = proc_table;
	char *p_task_stack = task_stack + STACK_SIZE_TOTAL;
	u16 selector_ldt = SELECTOR_LDT_FIRST;
	u8 privilege;
	u8 rpl;
	int eflags;
	int i, j;
	int prio;
	for (i = 0; i < NR_TASKS + NR_PROCS; i++)
	{
		if (i < NR_TASKS)
		{ /* 任务 */
			p_task = task_table + i;
			privilege = PRIVILEGE_TASK;
			rpl = RPL_TASK;
			eflags = 0x1202; /* IF=1, IOPL=1, bit 2 is always 1 */
			prio = 15;
		}
		else
		{ /* 用户进程 */
			p_task = user_proc_table + (i - NR_TASKS);
			privilege = PRIVILEGE_USER;
			rpl = RPL_USER;
			eflags = 0x202; /* IF=1, bit 2 is always 1 */
			prio = 5;
		}

		strcpy(p_proc->name, p_task->name); /* name of the process */
		p_proc->pid = i;					/* pid */

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
			   sizeof(struct descriptor));
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
			   sizeof(struct descriptor));
		p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;
		p_proc->regs.cs = (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ds = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.es = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = eflags;

		/* p_proc->nr_tty		= 0; */

		p_proc->p_flags = 0;
		p_proc->p_msg = 0;
		p_proc->p_recvfrom = NO_TASK;
		p_proc->p_sendto = NO_TASK;
		p_proc->has_int_msg = 0;
		p_proc->q_sending = 0;
		p_proc->next_sending = 0;

		for (j = 0; j < NR_FILES; j++)
			p_proc->filp[j] = 0;

		p_proc->ticks = p_proc->priority = prio;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	k_reenter = 0;
	ticks = 0;

	p_proc_ready = proc_table;

	init_clock();
	init_keyboard();

	restart();

	while (1)
	{
	}
}

/*****************************************************************************
 *                                get_ticks
 *****************************************************************************/
PUBLIC int get_ticks()
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = GET_TICKS;
	send_recv(BOTH, TASK_SYS, &msg);
	return msg.RETVAL;
}

/*======================================================================*
							   TestA
 *======================================================================*/
void TestA()
{
	int fd;
	int i, n;

	char tty_name[] = "/dev_tty0";

	char rdbuf[128];
	char command3[100], command4[100], command5[100];

	int fd_stdin = open(tty_name, O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

	//	char filename[MAX_FILENAME_LEN+1] = "zsp01";
	const char bufw[80] = {0};

	clear();

	/*================================= 这里是开机动画 ===============================*/
	Booting();

	Loading();

	clear();
	/*================================= 这里是显示在上面的系统信息 ===============================*/

	CommandList();

	while (1)
	{
		printf("master@cherryOS: $ ");

		memset(command3, 0, sizeof(command3));
		memset(command4, 0, sizeof(command4));
		memset(command5, 0, sizeof(command5));

		int r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;
		mystrncpy(command3, rdbuf, 3);
		mystrncpy(command4, rdbuf, 4);
		mystrncpy(command5, rdbuf, 5);

		if (!strcmp(command4, "help"))
		{
			clear();
			CommandList();
		}
		else if (!strcmp(command5, "clear"))
		{
			clear();
		}
		else if (!strcmp(command3, "man"))
		{
			if (strlen(rdbuf) > 4)
			{
				manMain(rdbuf + 4);
			}
			else
			{
				char *str = "NULL";
				manMain(str);
			}

			continue;
		}
		else if (!strcmp(command4, "game"))
		{
			if (strlen(rdbuf) > 5)
			{
				gameMain(rdbuf + 5, fd_stdin, fd_stdout);
			}
			else
			{
				char *str = "NULL";
				gameMain(str, fd_stdin, fd_stdout);
			}
			continue;
		}
		else if (!strcmp(command4, "math"))
		{
			if (strlen(rdbuf) > 5)
			{
				mathMain(rdbuf + 5);
			}
			else
			{
				char *str = "NULL";
				mathMain(str);
			}

			continue;
		}
		else if (!strcmp(command3, "cal"))
		{
			if (strlen(rdbuf) > 4)
			{
				calMain(rdbuf + 4);
			}
			else
			{
				char *str = "NULL";
				calMain(str);
			}
			continue;
		}
		else if (!strcmp(rdbuf, "process"))
		{
			clear();
			runProcessManage(fd_stdin);
		}
		else if (!strcmp(rdbuf, "file"))
		{
			clear();
			runFileManage(fd_stdin);
		}
		else if (!strcmp(rdbuf, ""))
		{
			continue;
		}
		else
		{
			clear();
			/*================================= 这里是命令不存在的提示信息 ===============================*/
			NotFound();
		}
	}
}

/*======================================================================*
							   TestB
 *======================================================================*/
void TestB()
{
	char tty_name[] = "/dev_tty1";
	int fd_stdin = open(tty_name, O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

	char rdbuf[128];
	char cmd[8];
	char filename[120];
	char buf[1024];
	int m, n;
	char _name[MAX_FILE_NAME_LENGTH];
	FSInit();
	int len = ReadDisk();
	ShowMessage();

	while (1)
	{
		for (int i = 0; i <= 127; i++)
			rdbuf[i] = '\0';
		for (int i = 0; i < MAX_FILE_NAME_LENGTH; i++)
			_name[i] = '\0';
		printf("\n/%s:", blocks[currentFileID].fileName);

		int r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;
		assert(fd_stdin == 0);
		char target[10];
		for (int i = 0; i <= 1 && i < r; i++)
		{
			target[i] = rdbuf[i];
		}
		if (rdbuf[0] == 't' && rdbuf[1] == 'o' && rdbuf[2] == 'u' && rdbuf[3] == 'c' && rdbuf[4] == 'h')
		{
			if (rdbuf[5] != ' ')
			{
				printf("You should add the filename, like \"create XXX\".\n");
				printf("Please input [help] to know more.\n");
				continue;
			}
			for (int i = 0; i < MAX_FILE_NAME_LENGTH && i < r - 3; i++)
			{
				_name[i] = rdbuf[i + 6];
			}
			CreateFIle(_name, 0);
		}
		else if (rdbuf[0] == 'm' && rdbuf[1] == 'k' && rdbuf[2] == 'd' && rdbuf[3] == 'i' && rdbuf[4] == 'r')
		{
			if (rdbuf[5] != ' ')
			{
				printf("You should add the dirname, like \"mkdir XXX\".\n");
				printf("Please input [help] to know more.\n");
				continue;
			}
			char N[MAX_FILE_NAME_LENGTH];
			for (int i = 0; i < MAX_FILE_NAME_LENGTH && i < r - 3; i++)
			{
				_name[i] = rdbuf[i + 6];
			}
			CreateFIle(_name, 1);
		}
		else if (strcmp(target, "ls") == 0)
		{
			showFileList();
		}
		else if (strcmp(target, "cd") == 0)
		{
			if (rdbuf[2] == ' ' && rdbuf[3] == '.' && rdbuf[4] == '.')
			{
				ReturnFile(currentFileID);
				continue;
			}
			else if (rdbuf[2] != ' ')
			{
				printf("You should add the dirname, like \"cd XXX\".\n");
				printf("Please input [help] to know more.\n");

				continue;
			}
			for (int i = 0; i < MAX_FILE_NAME_LENGTH && i < r - 3; i++)
			{
				_name[i] = rdbuf[i + 3];
			}
			printf("name: %s\n", _name);
			int ID = SearchFile(_name);
			if (ID >= 0)
			{
				if (blocks[ID].fileType == 1)
				{
					currentFileID = ID;
					continue;
				}
				else if (blocks[ID].fileType == 0)
				{
					while (1)
					{
						printf("input the character representing the method you want to operate:"
							   "\nu --- update"
							   "\nd --- detail of the content"
							   "\nq --- quit\n");
						int r = read(fd_stdin, rdbuf, 70);
						rdbuf[r] = 0;
						if (strcmp(rdbuf, "u") == 0)
						{
							printf("input the text you want to write:\n");
							int r = read(fd_stdin, blocks[ID].content, MAX_CONTENT_);
							blocks[ID].content[r] = 0;
						}
						else if (strcmp(rdbuf, "d") == 0)
						{
							printf("--------------------------------------------"
								   "\n%s\n-------------------------------------\n",
								   blocks[ID].content);
						}
						else if (strcmp(rdbuf, "q") == 0)
						{
							printf("would you like to save the change? y/n");
							int r = read(fd_stdin, rdbuf, 70);
							rdbuf[r] = 0;
							if (strcmp(rdbuf, "y") == 0)
							{
								printf("save changes!");
							}
							else
							{
								printf("quit without changing");
							}
							break;
						}
					}
				}
			}
			else
				printf("No such file!");
		}
		else if (strcmp(target, "rm") == 0)
		{
			if (rdbuf[2] != ' ')
			{
				printf("You should add the filename or dirname, like \"rm XXX\".\n");
				printf("Please input [help] to know more.\n");
				continue;
			}
			for (int i = 0; i < MAX_FILE_NAME_LENGTH && i < r - 3; i++)
			{
				_name[i] = rdbuf[i + 3];
			}
			int ID = SearchFile(_name);
			if (ID >= 0)
			{
				printf("Delete successfully!\n");
				DeleteFile(ID);
				for (int i = 0; i < blocks[currentFileID].childrenNumber; i++)
				{
					if (ID == blocks[currentFileID].children[i])
					{
						for (int j = i + 1; j < blocks[currentFileID].childrenNumber; j++)
						{
							blocks[currentFileID].children[i] = blocks[currentFileID].children[j];
						}
						blocks[currentFileID].childrenNumber--;
						break;
					}
				}
			}
			else
				printf("No such file!\n");
		}
		else if (strcmp(target, "sv") == 0)
		{
			WriteDisk(1000);
			printf("Save to disk successfully!\n");
		}
		else if (strcmp(rdbuf, "help") == 0)
		{
			printf("\n");
			ShowMessage(fd_stdin);
		}
		else if (strcmp(rdbuf, "quit") == 0)
		{
			printf("You cannot quit File Manager in this mode.\n");
		}
		else if (!strcmp(rdbuf, "clear"))
		{
			for (int i = 0; i < 30; ++i)
			{
				printf("\n");
			}
		}
		else
		{
			printf("Sorry, there no such command in the File Manager.\n");
			printf("You can input [help] to know more.\n");
		}
	}
}

/*****************************************************************************
 *                                TestC
 *****************************************************************************/
void TestC()
{
	spin("TestC");
}

/*****************************************************************************
 *                                panic
 *****************************************************************************/
PUBLIC void panic(const char *fmt, ...)
{
	int i;
	char buf[256];

	/* 4 is the size of fmt in the stack */
	va_list arg = (va_list)((char *)&fmt + 4);

	i = vsprintf(buf, fmt, arg);

	printl("%c !!panic!! %s", MAG_CH_PANIC, buf);

	/* should never arrive here */
	__asm__ __volatile__("ud2");
}

void clear()
{
	clear_screen(0, console_table[current_console].cursor);
	console_table[current_console].crtc_start = 0;
	console_table[current_console].cursor = 0;
}

void mystrncpy(char *dest, char *src, int len)
{
	assert(dest != NULL && src != NULL);

	char *temp = dest;
	int i = 0;
	while (i++ < len && (*temp++ = *src++) != '\0')
		;

	if (*(temp) != '\0')
	{
		*temp = '\0';
	}
}

/*开机动画*/
void Booting()
{
	emptyWindow();

	gradualStart();
	emptyWindow();

	gradualBoot();
}

void emptyWindow()
{
	printf("      ====================================================================\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      ====================================================================\n");

	milli_delay(DELAY_TIME);
	clear();
}
void gradualStart()
{
	printf("      ====================================================================\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                       *************                              #\n");
	printf("      #                       *************                              #\n");
	printf("      #                            ***                                   #\n");
	printf("      #                             ***                                  #\n");
	printf("      #                     ********  ***                                #\n");
	printf("      #                ************    ***                               #\n");
	printf("      #              ************       ***                              #\n");
	printf("      #             ***********          ***                             #\n");
	printf("      #             *******            *** ***                           #\n");
	printf("      #                              ***    ***                          #\n");
	printf("      #                             ***      ***                         #\n");
	printf("      #                            ***        ***                        #\n");
	printf("      #                       *********        ******                    #\n");
	printf("      #                     *************    **********                  #\n");
	printf("      #                    ***************   **********                  #\n");
	printf("      #                    ***************   **********                  #\n");
	printf("      #                     *************      ******                    #\n");
	printf("      #                      ***********                                 #\n");
	printf("      #                        *******                                   #\n");
	printf("      #                                                                  #\n");
	printf("      ====================================================================\n");

	milli_delay(DELAY_TIME);
	clear();

	printf("      ====================================================================\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                       *  ** *  **                                #\n");
	printf("      #                        **   * **                                 #\n");
	printf("      #                              *                                   #\n");
	printf("      #                             *                                    #\n");
	printf("      #                     ***  ***  * *                                #\n");
	printf("      #                   *****  **     **                               #\n");
	printf("      #              ****   *****       *                                #\n");
	printf("      #             **   ******          **                              #\n");
	printf("      #             ****  *              *                               #\n");
	printf("      #                              **     * *                          #\n");
	printf("      #                             * *        *                         #\n");
	printf("      #                            *          *                          #\n");
	printf("      #                         **  ***        *  ***                    #\n");
	printf("      #                     ****    *****    ****    **                  #\n");
	printf("      #                    *  *******   **   *  **  ***                  #\n");
	printf("      #                    **** **   *****   ** ***  **                  #\n");
	printf("      #                     ******  *****      ******                    #\n");
	printf("      #                      ***     ***                                 #\n");
	printf("      #                        *******                                   #\n");
	printf("      #                                                                  #\n");
	printf("      ====================================================================\n");

	milli_delay(DELAY_TIME);
	clear();

	printf("      ====================================================================\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                       *     *                                    #\n");
	printf("      #                        *    *  *                                 #\n");
	printf("      #                              *                                   #\n");
	printf("      #                             *                                    #\n");
	printf("      #                     *         * *                                #\n");
	printf("      #                           *                                      #\n");
	printf("      #                     *  **       *                                #\n");
	printf("      #             **   *               **                              #\n");
	printf("      #                *  *              *                               #\n");
	printf("      #                              **     * *                          #\n");
	printf("      #                               *        *                         #\n");
	printf("      #                            *          *                          #\n");
	printf("      #                         **    *             *                    #\n");
	printf("      #                        *       **    *       **                  #\n");
	printf("      #                    *        *   **   *  **    *                  #\n");
	printf("      #                       * **   *       **      **                  #\n");
	printf("      #                         **  *   *       *  **                    #\n");
	printf("      #                      ***       *                                 #\n");
	printf("      #                        **    *                                   #\n");
	printf("      #                                                                  #\n");
	printf("      ====================================================================\n");

	milli_delay(DELAY_TIME);
	clear();
}
void gradualBoot()
{
	printf("      ====================================================================\n");
	printf("      #                                                                  #\n");
	printf("      #                            *   *                                 #\n");
	printf("      #                              *                                   #\n");
	printf("      #                                *                                 #\n");
	printf("      #                            *    *                                #\n");
	printf("      #                                                                  #\n");
	printf("      #                     *  *      *                                  #\n");
	printf("      #                               *     *                            #\n");
	printf("      #                                     *  *                         #\n");
	printf("      #                          *    * *       *                        #\n");
	printf("      #                        *        *    * *                         #\n");
	printf("      #                          *    *                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      ====================================================================\n");

	milli_delay(DELAY_TIME);
	clear();

	printf("      ====================================================================\n");
	printf("      #                                                                  #\n");
	printf("      #                            *   **                                #\n");
	printf("      #                              **                                  #\n");
	printf("      #                           **   *                                 #\n");
	printf("      #                       ***  *    *                                #\n");
	printf("      #                        ***      **                               #\n");
	printf("      #                     *  *      *   **                             #\n");
	printf("      #                              **     *                            #\n");
	printf("      #                            ** *     *  *                         #\n");
	printf("      #                        ****   ***  * ****                        #\n");
	printf("      #                        *   ******   ** *                         #\n");
	printf("      #                          ***  *                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      ====================================================================\n");

	milli_delay(DELAY_TIME);
	clear();

	printf("      ====================================================================\n");
	printf("      #                                                                  #\n");
	printf("      #                           *******                                #\n");
	printf("      #                              **                                  #\n");
	printf("      #                          ***  **                                 #\n");
	printf("      #                       ******   **                                #\n");
	printf("      #                     ******      **                               #\n");
	printf("      #                     ****      **  **                             #\n");
	printf("      #                              **    **                            #\n");
	printf("      #                          ******     ****                         #\n");
	printf("      #                        **********  ******                        #\n");
	printf("      #                        **********   ****                         #\n");
	printf("      #                          ******                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      ====================================================================\n");

	milli_delay(DELAY_TIME);
	clear();

	printf("      ====================================================================\n");
	printf("      #                                                                  #\n");
	printf("      #                           *******                                #\n");
	printf("      #                              **                                  #\n");
	printf("      #                          ***  **                                 #\n");
	printf("      #                       ******   **                                #\n");
	printf("      #                     ******      **                               #\n");
	printf("      #                     ****      **  **                             #\n");
	printf("      #                              **    **                            #\n");
	printf("      #                          ******     ****                         #\n");
	printf("      #                        **********  ******                        #\n");
	printf("      #                        **********   ****                         #\n");
	printf("      #                          ******                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #    ########   ##    ##  ########  ########  ########  ##    ##   #\n");
	printf("      #    ##         ##    ##  ##        ##    ##  ##    ##  ##    ##   #\n");
	printf("      #    ##         ########  ########  ########  ########  ########   #\n");
	printf("      #    ##         ##    ##  ##        ##   ##   ##   ##      ##      #\n");
	printf("      #    ########   ##    ##  ########  ##    ##  ##    ##     ##      #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      ====================================================================\n");
}
void Loading()
{
	printf("System is loading...");

	for (int i = 0; i < 25; ++i)
	{
		milli_delay(DELAY_TIME / 5);
		printf(".");
	}
}

/*所有指令 & help窗口*/
void CommandList()
{
	printf("      ====================================================================\n");
	printf("      #         *******                         Welcome to               #\n");
	printf("      #            **                            cherryOS                #\n");
	printf("      #        ***  **                                                   #\n");
	printf("      #     ******   **                       [COMMAND LIST]             #\n");
	printf("      #   ******      **            $ help --- show the command list     #\n");
	printf("      #   ****      **  **          $ clear --- clear the cmd            #\n");
	printf("      #            **    **         $ man [command]                      #\n");
	printf("      #        ******     ****          --- know more about the command  #\n");
	printf("      #      **********  ******     $ game [-option]                     #\n");
	printf("      #      **********   ****          --- play the built-in game       #\n");
	printf("      #        ******               $ math [-option] [expression]        #\n");
	printf("      #                                 --- calculate the value          #\n");
	printf("      #                             $ cal [-option]                      #\n");
	printf("      #                                 --- date information             #\n");
	printf("      #                             $ process --- process manager        #\n");
	printf("      #                             $ file --- file manager              #\n");
	printf("      #                                                                  #\n");
	printf("      #               Powered by doubleZ, budi, flyingfish               #\n");
	printf("      #                       ALL RIGHT REVERSED                         #\n");
	printf("      ====================================================================\n");

	printf("\n\n");
}

/*没找到该指令窗口*/
void NotFound()
{
	printf("      ====================================================================\n");
	printf("      #                                                                  #\n");
	printf("      #         *******                                                  #\n");
	printf("      #            **                                                    #\n");
	printf("      #        ***  **                                                   #\n");
	printf("      #     ******   **                                                  #\n");
	printf("      #   ******      **                                                 #\n");
	printf("      #   ****      **  **                      Sorry                    #\n");
	printf("      #            **    **            Your command is not found         #\n");
	printf("      #        ******     ****                                           #\n");
	printf("      #      **********  ******                                          #\n");
	printf("      #      **********   ****                                           #\n");
	printf("      #        ******                                                    #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      #                 Input [help] for more information.               #\n");
	printf("      #                                                                  #\n");
	printf("      #                                                                  #\n");
	printf("      ====================================================================\n");
	printf("\n\n");
}

void manMain(char *option)
{
	if (!strcmp(option, "NULL"))
	{
		printf("Sorry, you should add an option.\n");
	}
	else if (!strcmp(option, "math"))
	{
		clear();
		printf("      ====================================================================\n");
		printf("      #         *******                         Welcome to               #\n");
		printf("      #            **                            cherryOS                #\n");
		printf("      #        ***  **                                                   #\n");
		printf("      #     ******   **                     <COMMAND --- math>           #\n");
		printf("      #   ******      **          Calculate the value of the expression  #\n");
		printf("      #   ****      **  **        and you can do more by adding options. #\n");
		printf("      #            **    **         The operator we support: [+-*/()]    #\n");
		printf("      #        ******     ****                                           #\n");
		printf("      #      **********  ******   [EXAMPLE]                              #\n");
		printf("      #      **********   ****       math -beauty 1 +  2   -3*4          #\n");
		printf("      #        ******                math -rev 3 * ( 7 - 4 ) + 8 / 4     #\n");
		printf("      #                              math 1+2-(3*4)/5                    #\n");
		printf("      #         [OPTION LIST]                                            #\n");
		printf("      #             -beauty [exp] -> beautify the expression             #\n");
		printf("      #             -rev [exp] -> output the reverse polish notation     #\n");
		printf("      #             no option -> just calculate the value                #\n");
		printf("      #                                                                  #\n");
		printf("      #               Powered by doubleZ, budi, flyingfish               #\n");
		printf("      #                       ALL RIGHT REVERSED                         #\n");
		printf("      ====================================================================\n");
	}
	else if (!strcmp(option, "help"))
	{
		clear();
		printf("      ====================================================================\n");
		printf("      #         *******                         Welcome to               #\n");
		printf("      #            **                            cherryOS                #\n");
		printf("      #        ***  **                                                   #\n");
		printf("      #     ******   **                                                  #\n");
		printf("      #   ******      **                                                 #\n");
		printf("      #   ****      **  **                 <COMMAND --- help>            #\n");
		printf("      #            **    **                                              #\n");
		printf("      #        ******     ****         Show the command list again.      #\n");
		printf("      #      **********  ******      And you can input these commands    #\n");
		printf("      #      **********   ****         to interact with the cherryOS.    #\n");
		printf("      #        ******                                                    #\n");
		printf("      #                                                                  #\n");
		printf("      #                                                                  #\n");
		printf("      #                                                                  #\n");
		printf("      #                                                                  #\n");
		printf("      #                                                                  #\n");
		printf("      #                                                                  #\n");
		printf("      #               Powered by doubleZ, budi, flyingfish               #\n");
		printf("      #                       ALL RIGHT REVERSED                         #\n");
		printf("      ====================================================================\n");
	}
	else if (!strcmp(option, "game"))
	{
		clear();
		printf("      ====================================================================\n");
		printf("      #         *******                         Welcome to               #\n");
		printf("      #            **                            cherryOS                #\n");
		printf("      #        ***  **                                                   #\n");
		printf("      #     ******   **                      <COMMAND --- game>          #\n");
		printf("      #   ******      **              You can play the build-in game     #\n");
		printf("      #   ****      **  **                  by adding [-option].         #\n");
		printf("      #            **    **                                              #\n");
		printf("      #        ******     ****    [OPTION LIST]                          #\n");
		printf("      #      **********  ******      -2048 -> for 2048 game              #\n");
		printf("      #      **********   ****       -chess -> for Five Chess game       #\n");
		printf("      #        ******                -mine -> for Mine Sweeping game     #\n");
		printf("      #                              -tictactoe -> for TicTacToe game    #\n");
		printf("      #                              -pushbox -> for PushBox game        #\n");
		printf("      #                                                                  #\n");
		printf("      #                           [EXAMPLE]                              #\n");
		printf("      #                              game -chess -> play Five Chess game #\n");
		printf("      #                                                                  #\n");
		printf("      #               Powered by doubleZ, budi, flyingfish               #\n");
		printf("      #                       ALL RIGHT REVERSED                         #\n");
		printf("      ====================================================================\n");
	}
	else if (!strcmp(option, "cal"))
	{
		clear();
		printf("      ====================================================================\n");
		printf("      #         *******                         Welcome to               #\n");
		printf("      #            **                            cherryOS                #\n");
		printf("      #        ***  **                                                   #\n");
		printf("      #     ******   **                      <COMMAND --- cal>           #\n");
		printf("      #   ******      **                You can search in a calendar     #\n");
		printf("      #   ****      **  **              with adding different option.    #\n");
		printf("      #            **    **                                              #\n");
		printf("      #        ******     ****    [OPTION LIST]                          #\n");
		printf("      #      **********  ******     -month [Y/M] -> search for a month   #\n");
		printf("      #      **********   ****      -week [Y/M/D] -> what day is it      #\n");
		printf("      #        ******               -date [Y/M/D] -> location of a date  #\n");
		printf("      #                                                                  #\n");
		printf("      #                           [EXAMPLE]                              #\n");
		printf("      #                                  cal -month 2019/12              #\n");
		printf("      #                                  cal -week 2017/11/11            #\n");
		printf("      #                                  cal -date 2019/8/31             #\n");
		printf("      #                                                                  #\n");
		printf("      #               Powered by doubleZ, budi, flyingfish               #\n");
		printf("      #                       ALL RIGHT REVERSED                         #\n");
		printf("      ====================================================================\n");
	}
	else if (!strcmp(option, "clear"))
	{
		clear();
		printf("      ====================================================================\n");
		printf("      #         *******                         Welcome to               #\n");
		printf("      #            **                            cherryOS                #\n");
		printf("      #        ***  **                                                   #\n");
		printf("      #     ******   **                                                  #\n");
		printf("      #   ******      **                                                 #\n");
		printf("      #   ****      **  **                 <COMMAND --- clear>           #\n");
		printf("      #            **    **                                              #\n");
		printf("      #        ******     ****        Clear the whole command station.   #\n");
		printf("      #      **********  ******          You can enjoy yourself in       #\n");
		printf("      #      **********   ****         interacting with the cherryOS.    #\n");
		printf("      #        ******                                                    #\n");
		printf("      #                                                                  #\n");
		printf("      #                                                                  #\n");
		printf("      #                                                                  #\n");
		printf("      #                                                                  #\n");
		printf("      #                                                                  #\n");
		printf("      #                                                                  #\n");
		printf("      #               Powered by doubleZ, budi, flyingfish               #\n");
		printf("      #                       ALL RIGHT REVERSED                         #\n");
		printf("      ====================================================================\n");
	}
	else if (!strcmp(option, "man"))
	{
		clear();
		printf("      ====================================================================\n");
		printf("      #         *******                         Welcome to               #\n");
		printf("      #            **                            cherryOS                #\n");
		printf("      #        ***  **                                                   #\n");
		printf("      #     ******   **                                                  #\n");
		printf("      #   ******      **                                                 #\n");
		printf("      #   ****      **  **                  <COMMAND --- man>            #\n");
		printf("      #            **    **                                              #\n");
		printf("      #        ******     ****            Know you want to know.         #\n");
		printf("      #      **********  ******          Just input man [command]        #\n");
		printf("      #      **********   ****         man will tell you everything.     #\n");
		printf("      #        ******                Find a man when you have trouble.   #\n");
		printf("      #                                                                  #\n");
		printf("      #                                                                  #\n");
		printf("      #                                                                  #\n");
		printf("      #                                                                  #\n");
		printf("      #                                                                  #\n");
		printf("      #                                                                  #\n");
		printf("      #               Powered by doubleZ, budi, flyingfish               #\n");
		printf("      #                       ALL RIGHT REVERSED                         #\n");
		printf("      ====================================================================\n");
	}
	else if (!strcmp(option, "process"))
	{
		clear();
		printf("      ====================================================================\n");
		printf("      #         *******                         Welcome to               #\n");
		printf("      #            **                            cherryOS                #\n");
		printf("      #        ***  **                                                   #\n");
		printf("      #     ******   **                                                  #\n");
		printf("      #   ******      **                   <COMMAND --- process>         #\n");
		printf("      #   ****      **  **              You can do process management    #\n");
		printf("      #            **    **            at our Process Management System. #\n");
		printf("      #        ******     ****                                           #\n");
		printf("      #      **********  ******   [COMMAND LIST]                         #\n");
		printf("      #      **********   ****      ps --- show all process              #\n");
		printf("      #        ******               kill [id] --- kill a process         #\n");
		printf("      #                             restart [id] --- restart a process   #\n");
		printf("      #                             quit ---                             #\n");
		printf("      #                                quit process management system    #\n");
		printf("      #                             help ---                             #\n");
		printf("      #                                show command list of this system  #\n");
		printf("      #                                                                  #\n");
		printf("      #               Powered by doubleZ, budi, flyingfish               #\n");
		printf("      #                       ALL RIGHT REVERSED                         #\n");
		printf("      ====================================================================\n");
	}
	else if (!strcmp(option, "file"))
	{
		clear();
		printf("      ====================================================================\n");
		printf("      #         *******                         Welcome to               #\n");
		printf("      #            **                            cherryOS                #\n");
		printf("      #        ***  **                                                   #\n");
		printf("      #     ******   **                                                  #\n");
		printf("      #   ******      **                 <COMMAND --- file>              #\n");
		printf("      #   ****      **  **             You can do file management        #\n");
		printf("      #            **    **          at our File Management System.      #\n");
		printf("      #        ******     ****       You can also use [Ctrl+F2] to       #\n");
		printf("      #      **********  ******   enter into our File Management System. #\n");
		printf("      #      **********   ****                                           #\n");
		printf("      #        ******             [COMMAND LIST]                         #\n");
		printf("      #                           touch [filename]  |   mkdir [dirname]  #\n");
		printf("      #                                  ls         |       help         #\n");
		printf("      #                             cd [dirname]    |       cd ..        #\n");
		printf("      #                             rm [filename]   |  rm -r [dirname]   #\n");
		printf("      #                                 clear       |       quit         #\n");
		printf("      #                                                                  #\n");
		printf("      #               Powered by doubleZ, budi, flyingfish               #\n");
		printf("      #                       ALL RIGHT REVERSED                         #\n");
		printf("      ====================================================================\n");
	}
	else
	{
		printf("Sorry, there no such option for man.\n");
		printf("You can input [help] to know more.\n");
	}

	printf("\n");
}



void gameMain(char *option, int fd_stdin, int fd_stdout)
{
	if (!strcmp(option, "NULL"))
	{
		printf("Sorry, you should add an option.\n");
	}
	else if (!strcmp(option, "-2048"))
	{
		Run2048(fd_stdin, fd_stdout);
	}
	else if (!strcmp(option, "-chess"))
	{
		fiveChess(fd_stdin);
	}
	else if (!strcmp(option, "-tictactoe"))
	{
		TicTacToe(fd_stdin, fd_stdout);
	}
	else if (!strcmp(option, "-pushbox"))
	{
		Runpushbox(fd_stdin, fd_stdout);
	}
	else if (!strcmp(option, "-mine"))
	{
		runMine(fd_stdin, fd_stdout);
	}
	else
	{
		printf("Sorry, there no such option for game.\n");
		printf("You can input [man game] to know more.\n");
	}

	printf("\n");
}
