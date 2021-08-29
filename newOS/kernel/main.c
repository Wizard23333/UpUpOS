
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
	int year = 2021, month = 9, day = 1;//日历需要的全局变量
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
				calendar(rdbuf + 4, &year, &month, &day);
			}
			else
			{
				char *str = "NULL";
				calendar(str, &year, &month, &day);
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
		else if (!strcmp(rdbuf, "systemguide"))
		{
			clear();
			runSystemGuide(fd_stdin, fd_stdout);
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
							   TestB-E
 *======================================================================*/
void TestB()
{
	spin("TestB");
}

void TestC()
{
	spin("TestC");
}
void TestD()
{
	spin("TestD");
}void TestE()
{
	spin("TestE");
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

	// gradualBoot();
}

void emptyWindow()
{
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");

	milli_delay(DELAY_TIME);
	clear();
}
void gradualStart()
{

	for (int i = 0; i < 2; i++)
	{

		printf("||||||||||                                                            ||||||||||");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");

		milli_delay(DELAY_TIME);
		clear();

		printf("||||||||||                                                            ||||||||||");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                  ####                                          ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		printf("                                                                                ");
		milli_delay(DELAY_TIME);
		clear();
	}

	printf("||||||||||                                                            ||||||||||");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                  ####                                          ");
	printf("                                      #####                                     ");
	printf("                                          ##                                    ");
	printf("                                           ##                                   ");
	printf("                                          ##                                    ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");

	milli_delay(DELAY_TIME);
	clear();

	printf("||||||||||                                                            ||||||||||");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                  ####                                          ");
	printf("                                      #####                                     ");
	printf("                                          ##                                    ");
	printf("                                           ##                                   ");
	printf("                                          ##                                    ");
	printf("                                        #                                       ");
	printf("                                          #                                     ");
	printf("                                        ##                                      ");
	printf("                                        #                                       ");
	printf("                                       #                                        ");
	printf("                                     #                                          ");
	printf("                                   ##                                           ");
	printf("                                                                                ");
	printf("                                                                                ");

	milli_delay(DELAY_TIME);
	clear();

	printf("||||||||||                                                            ||||||||||");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                  ####                                          ");
	printf("                                      #####                                     ");
	printf("                                          ##                                    ");
	printf("                                           ##                                   ");
	printf("                                          ##                                    ");
	printf("                                  #######                                       ");
	printf("                                          #                                     ");
	printf("                                        ##                                      ");
	printf("                                 ########                                       ");
	printf("                                       #                                        ");
	printf("                                ######                                          ");
	printf("                                    ##                                          ");
	printf("                                                                                ");
	printf("                                                                                ");

	milli_delay(DELAY_TIME);
	clear();

	printf("||||||||||                                                            ||||||||||");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                  ####                                          ");
	printf("                                      #####                                     ");
	printf("                                          ##                                    ");
	printf("                     ####                  ##                                   ");
	printf("                     ##   ##              ##                                    ");
	printf("                     ###     ############                                       ");
	printf("                     ####                 #                                     ");
	printf("                     ##   ###           ##                                      ");
	printf("                               ##########                                       ");
	printf("                          ###          #                                        ");
	printf("                              ########                                          ");
	printf("                                    ##                                          ");
	printf("                                                                                ");
	printf("                                                                                ");

	milli_delay(DELAY_TIME);
	clear();

	printf("||||||||||                                                            ||||||||||");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                       ###                                                      ");
	printf("                      ##          ####                                          ");
	printf("                     ##               #####                                     ");
	printf("                     ###                  ##                                    ");
	printf("                     ####                  ##                                   ");
	printf("                     ##   ##              ##                                    ");
	printf("                     ###     ############                                       ");
	printf("                     ####                 #                                     ");
	printf("                     ##   ###           ##                                      ");
	printf("                     ####      ##########                                       ");
	printf("                      ##  ###          #                                        ");
	printf("                      ###     ########                                          ");
	printf("                         ##         ##                                          ");
	printf("                                                                                ");
	printf("                                                                                ");

	milli_delay(DELAY_TIME);
	clear();

	printf("||||||||||                                                            ||||||||||");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                      ##                                        ");
	printf("                                     ###                                        ");
	printf("                          #############                                         ");
	printf("                       ###                                                      ");
	printf("                      ##          ####                                          ");
	printf("                     ##               #####                                     ");
	printf("                     ###                  ##                                    ");
	printf("                     ####                  ##                                   ");
	printf("                     ##   ##              ##                                    ");
	printf("                     ###     ############                                       ");
	printf("                     ####                 #                                     ");
	printf("                     ##   ###           ##                                      ");
	printf("                     ####      ##########                                       ");
	printf("                      ##  ###          #                                        ");
	printf("                      ###     ########                                          ");
	printf("                         ####       ##                                          ");
	printf("                             ###########                                        ");
	printf("                                                                                ");

	milli_delay(DELAY_TIME);
	clear();

	printf("||||||||||                                    ##                      ||||||||||");
	printf("                                           ###                                  ");
	printf("                                         ###                                    ");
	printf("                                       ###                                      ");
	printf("                                      ###                                       ");
	printf("                                      ###                                       ");
	printf("                                      ##                                        ");
	printf("                                     ###                                        ");
	printf("                          #############                                         ");
	printf("                       ###                                                      ");
	printf("                      ##          ####                                          ");
	printf("                     ##               #####                                     ");
	printf("                     ###                  ##                                    ");
	printf("                     ####                  ##                                   ");
	printf("                     ##   ##              ##                                    ");
	printf("                     ###     ############                                       ");
	printf("                     ####                 #                                     ");
	printf("                     ##   ###           ##                                      ");
	printf("                     ####      ##########       #                               ");
	printf("                      ##  ###          #       ##                               ");
	printf("                      ###     ########        ###                               ");
	printf("                         ####       ##     ##                                   ");
	printf("                             #############                                      ");
	printf("                                                                                ");

	milli_delay(DELAY_TIME);
	clear();

	printf("||||||||||                                    #########               ||||||||||");
	printf("                                           ###        ###                       ");
	printf("                                         ###            ###                     ");
	printf("                                       ###              ###                     ");
	printf("                                      ###            ####                       ");
	printf("                                      ###                                       ");
	printf("                                      ##                                        ");
	printf("                                     ###                                        ");
	printf("                          #############                                         ");
	printf("                       ###                                                      ");
	printf("                      ##          ####                                          ");
	printf("                     ##               #####                                     ");
	printf("                     ###                  ##                                    ");
	printf("                     ####                  ##                                   ");
	printf("                     ##   ##              ##                                    ");
	printf("                     ###     ############                                       ");
	printf("                     ####                 #                                     ");
	printf("                     ##   ###           ##                                      ");
	printf("                     ####      ##########       #                               ");
	printf("                      ##  ###          #       ##      ##                       ");
	printf("                      ###     ########        ###     ###                       ");
	printf("                         ####       ##     ##  #####                            ");
	printf("                             #############                                      ");
	printf("                                                                                ");

	milli_delay(DELAY_TIME);
	clear();

	printf("||||||||||                                    #########               ||||||||||");
	printf("                                           ###        ###                       ");
	printf("                                         ###            ###                     ");
	printf("                                       ###              ###                     ");
	printf("                                      ###            ####                       ");
	printf("                                      ###            ###                        ");
	printf("                                      ##           ###                          ");
	printf("                                     ###          ##                            ");
	printf("                          #############                                         ");
	printf("                       ###                                                      ");
	printf("                      ##          ####                                          ");
	printf("                     ##               #####                                     ");
	printf("                     ###                  ##                                    ");
	printf("                     ####                  ##                                   ");
	printf("                     ##   ##              ##            ###                     ");
	printf("                     ###     ############               ##                      ");
	printf("                     ####                 #              ###                    ");
	printf("                     ##   ###           ##               ##                     ");
	printf("                     ####      ##########       #       ##                      ");
	printf("                      ##  ###          #       ##      ##                       ");
	printf("                      ###     ########        ###     ###                       ");
	printf("                         ####       ##     ##  #####                            ");
	printf("                             #############                                      ");
	printf("                                                                                ");

	milli_delay(DELAY_TIME);
	clear();

	printf("||||||||||                                    #########               ||||||||||");
	printf("                                           ###        ###                       ");
	printf("                                         ###            ###                     ");
	printf("                                       ###              ###                     ");
	printf("                                      ###            ####                       ");
	printf("                                      ###            ###                        ");
	printf("                                      ##           ###                          ");
	printf("                                     ###          ##                            ");
	printf("                          #############          ###                            ");
	printf("                       ###                       ###                            ");
	printf("                      ##          ####           ###                            ");
	printf("                     ##               #####       ######                        ");
	printf("                     ###                  ##       ##  ###                      ");
	printf("                     ####                  ##      #    ###                     ");
	printf("                     ##   ##              ##            ###                     ");
	printf("                     ###     ############               ##                      ");
	printf("                     ####                 #              ###                    ");
	printf("                     ##   ###           ##               ##                     ");
	printf("                     ####      ##########       #       ##                      ");
	printf("                      ##  ###          #       ##      ##                       ");
	printf("                      ###     ########        ###     ###                       ");
	printf("                         ####       ##     ##  #####                            ");
	printf("                             #############                                      ");
	printf("                                                                                ");

	milli_delay(DELAY_TIME);
	clear();
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
	printf("      #                                         Welcome to               #\n");
	printf("      #  ***     ***                             UpUPOS                  #\n");
	printf("      #  ***     ***                                                     #\n");
	printf("      #  ***     ***  *********               [COMMAND LIST]             #\n");
	printf("      #  ***     ***  **       **   $ help --- show the command list     #\n");
	printf("      #   *********   **       **   $ clear --- clear the cmd            #\n");
	printf("      #               **       **   $ man [command]                      #\n");
	printf("      #               **********        --- know more about the command  #\n");
	printf("      #               **            $ game [-option]                     #\n");
	printf("      #  ***     ***  **                --- play the built-in game       #\n");
	printf("      #  ***     ***                $ math [-option] [expression]        #\n");
	printf("      #  ***     ***                    --- calculate the value          #\n");
	printf("      #  ***     *** *********      $ cal [-option]                      #\n");
	printf("      #   *********  **       **        --- date information             #\n");
	printf("      #              **       **    $ process --- process manager        #\n");
	printf("      #              **       **    $ file --- file manager              #\n");
	printf("      #              **********                                          #\n");
	printf("      #              **                                                  #\n");
	printf("      #              **                                                  #\n");
	printf("      ====================================================================\n");

	printf("\n\n");
}

/*没找到该指令窗口*/
void NotFound()
{
	printf("      ====================================================================\n");
	printf("      #                                                                  #\n");
	printf("      #  ***     ***                                                     #\n");
	printf("      #  ***     ***                                                     #\n");
	printf("      #  ***     ***  *********                                          #\n");
	printf("      #  ***     ***  **       **                                        #\n");
	printf("      #   *********   **       **                                        #\n");
	printf("      #               **       **               Sorry                    #\n");
	printf("      #               **********       Your command is not found         #\n");
	printf("      #               **                                                 #\n");
	printf("      #  ***     ***  **                                                 #\n");
	printf("      #  ***     ***                                                     #\n");
	printf("      #  ***     ***                                                     #\n");
	printf("      #  ***     *** *********                                           #\n");
	printf("      #   *********  **       **                                         #\n");
	printf("      #              **       **                                         #\n");
	printf("      #              **       **                                         #\n");
	printf("      #              **********      Input [help] for more information.  #\n");
	printf("      #              **                                                  #\n");
	printf("      #              **                                                  #\n");
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
		printf("      #                                         Welcome to               #\n");
		printf("      #  ***     ***                              UpUpOS                 #\n");
		printf("      #  ***     ***                                                     #\n");
		printf("      #  ***     ***  *********             <COMMAND --- math>           #\n");
		printf("      #  ***     ***  **       **  Calculate the value of the expression #\n");
		printf("      #   *********   **       **     The operator we support: [+-*/()]  #\n");
		printf("      #               **       **                                        #\n");
		printf("      #               **********   [OPTION LIST]                         #\n");
		printf("      #               **           math expression-> calculate the value #\n");
		printf("      #  ***     ***  **                                                 #\n");
		printf("      #  ***     ***                                                     #\n");
		printf("      #  ***     ***                                                     #\n");
		printf("      #  ***     *** *********      [EXAMPLE]                            #\n");
		printf("      #   *********  **       **     math 1+2-(3*4)/5                    #\n");
		printf("      #              **       **                                         #\n");
		printf("      #              **       **                                         #\n");
		printf("      #              **********                                          #\n");
		printf("      #              **                                                  #\n");
		printf("      #              **                                                  #\n");
		printf("      ====================================================================\n");
	
	}
	else if (!strcmp(option, "help"))
	{
		clear();
		printf("      ====================================================================\n");
		printf("      #                                         Welcome to               #\n");
		printf("      #  ***     ***                              UpUpOS                 #\n");
		printf("      #  ***     ***                                                     #\n");
		printf("      #  ***     ***  *********             <COMMAND --- help>           #\n");
		printf("      #  ***     ***  **       **        Show the command list .         #\n");
		printf("      #   *********   **       **      You can input these commands to   #\n");
		printf("      #               **       **         interact with the  UpUpOS.      #\n");
		printf("      #               **********                                         #\n");
		printf("      #               **                                                 #\n");
		printf("      #  ***     ***  **                                                 #\n");
		printf("      #  ***     ***                                                     #\n");
		printf("      #  ***     ***                                                     #\n");
		printf("      #  ***     *** *********                                           #\n");
		printf("      #   *********  **       **                                         #\n");
		printf("      #              **       **                                         #\n");
		printf("      #              **       **                                         #\n");
		printf("      #              **********                                          #\n");
		printf("      #              **                                                  #\n");
		printf("      #              **                                                  #\n");
		printf("      ====================================================================\n");
	
	}
	else if (!strcmp(option, "game"))
	{
		clear();
		printf("      ====================================================================\n");
		printf("      #                                         Welcome to               #\n");
		printf("      #  ***     ***                              UpUpOS                 #\n");
		printf("      #  ***     ***                                                     #\n");
		printf("      #  ***     ***  *********             <COMMAND --- game>           #\n");
		printf("      #  ***     ***  **       **           Show the game list .         #\n");
		printf("      #   *********   **       **      You can input these games to      #\n");
		printf("      #               **       **                 play.                  #\n");
		printf("      #               **********                                         #\n");
		printf("      #               **               [OPTION LIST]                     #\n");
		printf("      #  ***     ***  **               -chess -> for Five Chess game     #\n");
		printf("      #  ***     ***                   -mine -> for Mine Sweeping game   #\n");
		printf("      #  ***     ***                   -colorball -> for colorball game  #\n");
		printf("      #  ***     *** *********                                           #\n");
		printf("      #   *********  **       **                                         #\n");
		printf("      #              **       **                                         #\n");
		printf("      #              **       **                                         #\n");
		printf("      #              **********      [EXAMPLE]                           #\n");
		printf("      #              **              game -chess -> play Five Chess game #\n");
		printf("      #              **                                                  #\n");
		printf("      ====================================================================\n");
	
	}
	else if (!strcmp(option, "cal"))
	{
		clear();
		printf("      ====================================================================\n");
		printf("      #                                         Welcome to               #\n");
		printf("      #  ***     ***                              UpUpOS                 #\n");
		printf("      #  ***     ***                                                     #\n");
		printf("      #  ***     ***  *********              <COMMAND --- cal>           #\n");
		printf("      #  ***     ***  **       **    You can find the date  instructions #\n");
		printf("      #   *********   **       **      through different instructions    #\n");
		printf("      #               **       **                                        #\n");
		printf("      #               **********   [OPTION LIST]                         #\n");
		printf("      #               **            -y [Y] -> Jump to the input year     #\n");
		printf("      #  ***     ***  **            -m [Y/M] -> Jump to the input month  #\n");
		printf("      #  ***     ***                -d [Y/M/D] -> Jump to the input day  #\n");
		printf("      #  ***     ***                              and show the detailed  #\n");
		printf("      #  ***     *** *********                   information of the day  #\n");
		printf("      #   *********  **       **    -pre -> Jump to the last month       #\n");
		printf("      #              **       **    -next -> Jump to the next month      #\n");
		printf("      #              **       **    -pre-y -> Jump to the last year      #\n");
		printf("      #              **********     -next-y -> Jump to the next year     #\n");
		printf("      #              **           [EXAMPLE]                              #\n");
		printf("      #              **             cal -d 2021/1/1                      #\n");
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
	else if (!strcmp(option, "-colorball"))
	{
		colorBall(fd_stdin);
	}
	else if (!strcmp(option, "-draughts"))
	{
		runDraghts(fd_stdin, fd_stdout);
	}
	else
	{
		printf("Sorry, there no such option for game.\n");
		printf("You can input [man game] to know more.\n");
	}

	printf("\n");
}
