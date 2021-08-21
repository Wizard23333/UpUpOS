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
 *                                2048
 *****************************************************************************/

#define KEY_CODE_UP    0x41
#define KEY_CODE_DOWN  0x42
#define KEY_CODE_LEFT  0x44
#define KEY_CODE_RIGHT 0x43
#define KEY_CODE_QUIT  0x71

struct termios old_config; /* linux下终端属性配置备份 */


static char config_path[4096] = { 0 }; /* 配置文件路径 */

static void init_game();    /* 初始化游戏 */
static void loop_game(int fd_stdin);    /* 游戏循环 */
static void reset_game();   /* 重置游戏 */
static void release_game(int signal); /* 释放游戏 */

static char* read_keyboard(int fd_stdin);

static void move_left();  /* 左移 */
static void move_right(); /* 右移 */
static void move_up();    /* 上移 */
static void move_down();  /* 下移 */

static void add_rand_num();    /* 生成随机数，本程序中仅生成2或4，概率之比设为9:1 */
static void check_game_over(); /* 检测是否输掉游戏，设定游戏结束标志 */
static int get_null_count();   /* 获取游戏面板上空位置数量 */
static void clear_screen();    /* 清屏 */
static void refresh_show();    /* 刷新界面显示 */

static int board[4][4];     /* 游戏数字面板，抽象为二维数组 */
static int score;           /* 游戏得分 */
static int best;            /* 游戏最高分 */
static int if_need_add_num; /* 是否需要生成随机数标志，1表示需要，0表示不需要 */
static int if_game_over;    /* 是否游戏结束标志，1表示游戏结束，0表示正常 */
static int if_prepare_exit; /* 是否准备退出游戏，1表示是，0表示否 */

/* main函数 函数定义 */
void Run2048(fd_stdin, fd_stdout)
{
	clear();
	init_game();
	loop_game(fd_stdin);
	release_game(0);
}

/* 开始游戏 函数定义 */
void loop_game(int fd_stdin) {
	while (1) {
		/* 接收标准输入流字符命令 */
		char rdbuf[128];
		int r = 0;
		r = read(fd_stdin, rdbuf, 70);
		if (r > 1)
		{
			refresh_show();
			continue;
		}
		rdbuf[r] = 0;
		char cmd = rdbuf[0];
		/* 判断是否准备退出游戏 */
		if (if_prepare_exit) {
			if (cmd == 'y' || cmd == 'Y') {
				/* 退出游戏，清屏后退出 */
				clear_screen();
				return;
			}
			else if (cmd == 'n' || cmd == 'N') {
				/* 取消退出 */
				if_prepare_exit = 0;
				refresh_show();
				continue;
			}
			else {
				continue;
			}
		}

		/* 判断是否已经输掉游戏 */
		if (if_game_over) {
			if (cmd == 'y' || cmd == 'Y') {
				/* 重玩游戏 */
				reset_game();
				continue;
			}
			else if (cmd == 'n' || cmd == 'N') {
				/* 退出游戏，清屏后退出  */
				clear();
				return;
			}
			else {
				continue;
			}
		}

		if_need_add_num = 0; /* 先设定不默认需要生成随机数，需要时再设定为1 */
		/* 命令解析，上下左右箭头代表上下左右命令，q代表退出 */
		switch (cmd) {
		case 'a':
			move_left();
			break;
		case 's':
			move_down();
			break;
		case 'w':
			move_up();
			break;
		case 'd':
			move_right();
			break;
		case 'q':
			if_prepare_exit = 1;
			break;
		default:
			refresh_show();
			continue;
		}

		/* 默认为需要生成随机数时也同时需要刷新显示，反之亦然 */
		if (if_need_add_num) {
			add_rand_num();
			refresh_show();
		}
		else if (if_prepare_exit) {
			refresh_show();
		}
	}
}

/* 重置游戏 函数定义 */
void reset_game() {
	score = 0;
	if_need_add_num = 1;
	if_game_over = 0;
	if_prepare_exit = 0;

	/* 了解到游戏初始化时出现的两个数一定会有个2，所以先随机生成一个2，其他均为0 */
	int n = get_ticks() % 16;
	int i;
	for (i = 0; i < 4; ++i) {
		int j;
		for (j = 0; j < 4; ++j) {
			board[i][j] = (n-- == 0 ? 2 : 0);
		}
	}

	/* 前面已经生成了一个2，这里再生成一个随机的2或者4，概率之比9:1 */
	add_rand_num();

	/* 在这里刷新界面并显示的时候，界面上已经默认出现了两个数字，其他的都为空（值为0） */
	refresh_show();
}

/* 生成随机数 函数定义 */
void add_rand_num() {
	int n = get_ticks() % get_null_count(); /* 确定在何处空位置生成随机数 */
	int i;
	for (i = 0; i < 4; ++i) {
		int j;
		for (j = 0; j < 4; ++j) {
			/* 定位待生成的位置 */
			if (board[i][j] == 0 && n-- == 0) {
				board[i][j] = (get_ticks() % 10 ? 2 : 4); /* 生成数字2或4，生成概率为9:1 */
				return;
			}
		}
	}
}

/* 获取空位置数量 函数定义 */
int get_null_count() {
	int n = 0;
	int i;
	for (i = 0; i < 4; ++i) {
		int j;
		for (j = 0; j < 4; ++j) {
			board[i][j] == 0 ? ++n : 1;
		}
	}
	return n;
}

/* 检查游戏是否结束 函数定义 */
void check_game_over() {
	int i;
	for (i = 0; i < 4; ++i) {
		int j;
		for (j = 0; j < 3; ++j) {
			/* 横向和纵向比较挨着的两个元素是否相等，若有相等则游戏不结束 */
			if (board[i][j] == board[i][j + 1] || board[j][i] == board[j + 1][i]) {
				if_game_over = 0;
				return;
			}
		}
	}
	if_game_over = 1;
}

/*
 * 如下四个函数，实现上下左右移动时数字面板的变化算法
 * 左和右移动的本质一样，区别仅仅是列项的遍历方向相反
 * 上和下移动的本质一样，区别仅仅是行项的遍历方向相反
 * 左和上移动的本质也一样，区别仅仅是遍历时行和列互换
*/

/*  左移 函数定义 */
void move_left() {
	/* 变量i用来遍历行项的下标，并且在移动时所有行相互独立，互不影响 */
	int i;
	for (i = 0; i < 4; ++i) {
		/* 变量j为列下标，变量k为待比较（合并）项的下标，循环进入时k<j */
		int j, k;
		for (j = 1, k = 0; j < 4; ++j) {
			if (board[i][j] > 0) /* 找出k后面第一个不为空的项，下标为j，之后分三种情况 */
			{
				if (board[i][k] == board[i][j]) {
					/* 情况1：k项和j项相等，此时合并方块并计分 */
					score += board[i][k++] *= 2;
					board[i][j] = 0;
					if_need_add_num = 1; /* 需要生成随机数和刷新界面 */
				}
				else if (board[i][k] == 0) {
					/* 情况2：k项为空，则把j项赋值给k项，相当于j方块移动到k方块 */
					board[i][k] = board[i][j];
					board[i][j] = 0;
					if_need_add_num = 1;
				}
				else {
					/* 情况3：k项不为空，且和j项不相等，此时把j项赋值给k+1项，相当于移动到k+1的位置 */
					board[i][++k] = board[i][j];
					if (j != k) {
						/* 判断j项和k项是否原先就挨在一起，若不是则把j项赋值为空（值为0） */
						board[i][j] = 0;
						if_need_add_num = 1;
					}
				}
			}
		}
	}
}

/* 右移 函数定义 */
void move_right() {
	/* 仿照左移操作，区别仅仅是j和k都反向遍历 */
	int i;
	for (i = 0; i < 4; ++i) {
		int j, k;
		for (j = 2, k = 3; j >= 0; --j) {
			if (board[i][j] > 0) {
				if (board[i][k] == board[i][j]) {
					score += board[i][k--] *= 2;
					board[i][j] = 0;
					if_need_add_num = 1;
				}
				else if (board[i][k] == 0) {
					board[i][k] = board[i][j];
					board[i][j] = 0;
					if_need_add_num = 1;
				}
				else {
					board[i][--k] = board[i][j];
					if (j != k) {
						board[i][j] = 0;
						if_need_add_num = 1;
					}
				}
			}
		}
	}
}

/* 上移 函数定义 */
void move_up() {
	/* 仿照左移操作，区别仅仅是行列互换后遍历 */
	int i;
	for (i = 0; i < 4; ++i) {
		int j, k;
		for (j = 1, k = 0; j < 4; ++j) {
			if (board[j][i] > 0) {
				if (board[k][i] == board[j][i]) {
					score += board[k++][i] *= 2;
					board[j][i] = 0;
					if_need_add_num = 1;
				}
				else if (board[k][i] == 0) {
					board[k][i] = board[j][i];
					board[j][i] = 0;
					if_need_add_num = 1;
				}
				else {
					board[++k][i] = board[j][i];
					if (j != k) {
						board[j][i] = 0;
						if_need_add_num = 1;
					}
				}
			}
		}
	}
}

/* 下移 函数定义 */
void move_down() {
	/* 仿照左移操作，区别仅仅是行列互换后遍历，且j和k都反向遍历 */
	int i;
	for (i = 0; i < 4; ++i) {
		int j, k;
		for (j = 2, k = 3; j >= 0; --j) {
			if (board[j][i] > 0) {
				if (board[k][i] == board[j][i]) {
					score += board[k--][i] *= 2;
					board[j][i] = 0;
					if_need_add_num = 1;
				}
				else if (board[k][i] == 0) {
					board[k][i] = board[j][i];
					board[j][i] = 0;
					if_need_add_num = 1;
				}
				else {
					board[--k][i] = board[j][i];
					if (j != k) {
						board[j][i] = 0;
						if_need_add_num = 1;
					}
				}
			}
		}
	}
}

/* 刷新界面 函数定义 */
void refresh_show() {
	clear();

	printf("\n\n\n\n");
	printf("                  GAME: 2048     SCORE: %05d    \n", score);
	printf("               --------------------------------------------------");

	/* 绘制方格和数字 */
	printf("\n\n                             |----|----|----|----|\n");
	int i;
	for (i = 0; i < 4; ++i) {
		printf("                             |");
		int j;
		for (j = 0; j < 4; ++j) {
			if (board[i][j] != 0) {
				if (board[i][j] < 10) {
					printf("  %d |", board[i][j]);
				}
				else if (board[i][j] < 100) {
					printf(" %d |", board[i][j]);
				}
				else if (board[i][j] < 1000) {
					printf(" %d|", board[i][j]);
				}
				else if (board[i][j] < 10000) {
					printf("%4d|", board[i][j]);
				}
				else {
					int n = board[i][j];
					int k;
					for (k = 1; k < 20; ++k) {
						n = n >> 1;
						if (n == 1) {
							printf("2^%02d|", k); /* 超过四位的数字用2的幂形式表示，如2^13形式 */
							break;
						}
					}
				}
			}
			else printf("    |");
		}

		if (i < 3) {
			printf("\n                             |----|----|----|----|\n");
		}
		else {
			printf("\n                             |----|----|----|----|\n");
		}
	}
	printf("\n");
	printf("               --------------------------------------------------\n");
	printf("                  [W]:UP [S]:DOWN [A]:LEFT [D]:RIGHT [Q]:EXIT\n");
	printf("                  Enter your commond here:");

	if (get_null_count() == 0) {
		check_game_over();

		/* 判断是否输掉游戏 */
		if (if_game_over) {
			printf("\r                      \nGAME OVER! TRY THE GAME AGAIN? [Y/N]:     \b\b\b\b");
		}
	}

	/* 判断是否准备退出游戏 */
	if (if_prepare_exit) {
		printf("\r                   \nDO YOU REALLY WANT TO QUIT THE GAME? [Y/N]:   \b\b");

	}
}

/* 初始化游戏 */
void init_game() {
	reset_game();
}

/* 释放游戏 */
void release_game(int signal) {
	clear();

	if (signal == SIGINT) {
		printf("\n");
	}

}