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

/*======================================================================*
							TicTacToe
 *======================================================================*/
int tmpQP[3][3]; //定义棋盘，0为空，1为敌方棋子，-1为我方棋子
#define MAX_NUM 1000
const int NO_BLANK = -1001;
const int TREE_DEPTH = 3; //搜索树的最大深度
const int NIL = 1001;    //空
static int s_count;


struct State//棋盘状态
{
	int QP[3][3];
	int e_fun; //评估函数值
	int child[9];
	int parent;
	int bestChild;
}States[MAX_NUM];


void chessMenu()
{
	printf("      ====================================================================\n");
	printf("      #         *******                         Welcome to               #\n");
	printf("      #            **                            cherryOS                #\n");
	printf("      #        ***  **                                                   #\n");
	printf("      #     ******   **                                                  #\n");
	printf("      #   ******      **                         Instruction             #\n");
	printf("      #   ****      **  **               set: O--uesr X--computer        #\n");
	printf("      #            **    **            You can choose to play first or   #\n");
	printf("      #        ******     ****        let the computer first. Enter 'x'  #\n");
	printf("      #      **********  ******        from1,2,3,and then 'y' the same.  #\n");
	printf("      #      **********   ****         Can you defeat the computer?      #\n");
	printf("      #        ******                                                    #\n");
	printf("      #                                 PS:you can enter'q' to quit      #\n");
	printf("      #                                    before the game start.        #\n");
	printf("      #                                                                  #\n");
	printf("      #               Powered by doubleZ, budi, flyingfish               #\n");
	printf("      #                       ALL RIGHT REVERSED                         #\n");
	printf("      ====================================================================\n");
}

void Init()//初始化
{
	int i, j;
	s_count = 0;
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			States[0].QP[i][j] = 0; //清空棋盘
		}
	}
	States[0].parent = NIL;
}

void Printchessboard() //打印棋盘
{
	clear();
	chessMenu();
	int i, j;
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			if (States[0].QP[i][j] == -1)
			{
				printf("O     ");
			}
			else if (States[0].QP[i][j] == 1)
			{
				printf("X     ");
			}
			else
			{
				printf("#     ");
			}
		}
		printf("\n");
	}
}

int IsWin(struct State s) //判断输赢
{
	int i, j;
	for (i = 0; i < 3; i++)
	{
		if (s.QP[i][0] == 1 && s.QP[i][1] == 1 && s.QP[i][2] == 1)
			return 1;
		if (s.QP[i][0] == -1 && s.QP[i][1] == -1 && s.QP[i][2] == -1)
			return -1;
	}
	for (i = 0; i < 3; i++)
	{
		if (s.QP[0][i] == 1 && s.QP[1][i] == 1 && s.QP[2][i] == 1)return 1;
		if (s.QP[0][i] == -1 && s.QP[1][i] == -1 && s.QP[2][i] == -1)return -1;
	}
	if ((s.QP[0][0] == 1 && s.QP[1][1] == 1 && s.QP[2][2] == 1) || (s.QP[2][0] == 1 && s.QP[1][1] == 1 && s.QP[0][2] == 1))
		return 1;//计算机赢
	if ((s.QP[0][0] == -1 && s.QP[1][1] == -1 && s.QP[2][2] == -1) || (s.QP[2][0] == -1 && s.QP[1][1] == -1 && s.QP[0][2] == -1))
		return -1;//人赢
	return 0;//平局
}

int e_fun(struct State s)//评估函数
{
	int flag = 1;
	int i, j;
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			if (s.QP[i][j] == 0)
				flag = FALSE;
		}
	}
	if (flag)
		return NO_BLANK;

	if (IsWin(s) == -1)
		return -MAX_NUM;
	if (IsWin(s) == 1)
		return MAX_NUM;
	int count = 0;

	//将空格填满电脑的棋子
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			if (s.QP[i][j] == 0)
				tmpQP[i][j] = 1;
			else
				tmpQP[i][j] = s.QP[i][j];
		}
	}
	for (i = 0; i < 3; i++)
	{
		count += (tmpQP[i][0] + tmpQP[i][1] + tmpQP[i][2]) / 3;
	}
	for (i = 0; i < 3; i++)
	{
		count += (tmpQP[0][i] + tmpQP[1][i] + tmpQP[2][i]) / 3;
	}
	count += (tmpQP[0][0] + tmpQP[1][1] + tmpQP[2][2]) / 3;
	count += (tmpQP[2][0] + tmpQP[1][1] + tmpQP[0][2]) / 3;

	//将空格填满人的棋子
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			if (s.QP[i][j] == 0)
				tmpQP[i][j] = -1;
			else
				tmpQP[i][j] = s.QP[i][j];
		}
	}

	//行连成3个
	for (i = 0; i < 3; i++)
	{
		count += (tmpQP[i][0] + tmpQP[i][1] + tmpQP[i][2]) / 3;
	}
	//列连成3个
	for (i = 0; i < 3; i++)
	{
		count += (tmpQP[0][i] + tmpQP[1][i] + tmpQP[2][i]) / 3;
	}
	//斜行连成3个
	count += (tmpQP[0][0] + tmpQP[1][1] + tmpQP[2][2]) / 3;
	count += (tmpQP[2][0] + tmpQP[1][1] + tmpQP[0][2]) / 3;

	return count;

}


int AutoDone()//计算机决定落子位置，并判断当前棋局。
{

	int
		MAX_F = NO_BLANK, //最大的评估函数值
		parent = -1,
		count,
		tag;
	int
		max_min = TREE_DEPTH % 2,//max_min=1取下一层评估函数的最大值(电脑下)，max_min=0取最小值（人下）
		IsOK = FALSE;    //有没有找到下一步落子的位置？
	s_count = 0;   //扩展生成的节点数初始值为0

	if (IsWin(States[0]) == -1)//如果人赢了
	{
		printf("Conguatulations! You Win! GAME OVER.\n");
		return TRUE;
	}

	int i, j, t, k, i1, j1;
	for (t = 0; t < TREE_DEPTH; t++)//t等于偶数电脑下棋，t等于奇数人下棋
	{
		count = s_count;
		for (k = parent + 1; k <= count; k++)
		{
			int n_child = 0;
			for (i = 0; i < 3; i++)
			{
				for (j = 0; j < 3; j++)
				{
					if (States[k].QP[i][j] == 0)//如果在位置(i,j)可以放置一个棋子
					{
						s_count++;    //生成一个节点
						for (i1 = 0; i1 < 3; i1++)
						{
							for (j1 = 0; j1 < 3; j1++)
							{
								States[s_count].QP[i1][j1] = States[k].QP[i1][j1];//将当前棋局复制到新节点对应的棋局中
							}
						}
						States[s_count].QP[i][j] = t % 2 == 0 ? 1 : -1;//人下棋置-1，计算机下棋置1
						States[s_count].parent = k;   //其父母节点的下标为k
						States[k].child[n_child++] = s_count; //下标为k的父母节点多了个子女
						if (t == 0 && e_fun(States[s_count]) == MAX_NUM)//如果电脑能赢，则停止扩展
						{
							States[k].e_fun = MAX_NUM;
							States[k].bestChild = s_count;
							goto L2;
						}
					}
				}
			}
		}
		parent = count;   //将双亲节点设置为当前双亲节点的下一层节点
	}

	tag = States[s_count].parent;
	int pos_x, pos_y;//保存计算机落子的位置
	for (i = 0; i <= s_count; i++)
	{
		if (i > tag) //i>tag的为叶节点
		{
			States[i].e_fun = e_fun(States[i]);
		}
		else //否则为非叶节点
		{
			States[i].e_fun = NIL;
		}
	}
	while (!IsOK)//寻找最佳落子
	{
		for (i = s_count; i > tag; i--)
		{
			if (max_min)//取子女节点的最大值
			{
				if (States[States[i].parent].e_fun < States[i].e_fun || States[States[i].parent].e_fun == NIL)
				{
					States[States[i].parent].e_fun = States[i].e_fun;
					States[States[i].parent].bestChild = i;   //设置父母节点的最佳子女的下标
				}
			}
			else//取子女节点的最小值
			{
				if (States[States[i].parent].e_fun > States[i].e_fun || States[States[i].parent].e_fun == NIL)
				{
					States[States[i].parent].e_fun = States[i].e_fun;
					States[States[i].parent].bestChild = i;   //设置父母节点的最佳子女的下标
				}
			}
		}
		s_count = tag; //将遍历的节点上移一层
		max_min = !max_min; //如果该层都是MAX节点，则它的上一层都是MIN节点，反之亦然（电脑和人交替落子）。
		if (States[s_count].parent != NIL)
			tag = States[s_count].parent;
		else
			IsOK = TRUE; //结束搜索
	}
	int x, y;
L2: //取落子的位置，将x,y坐标保存在变量pos_x和pos_y中，并将根（当前）节点中的棋局设为最佳儿子节点的棋局

	for (x = 0; x < 3; x++)
	{
		for (y = 0; y < 3; y++)
		{
			if (States[States[0].bestChild].QP[x][y] != States[0].QP[x][y])
			{
				pos_x = x;
				pos_y = y;
			}
			States[0].QP[x][y] = States[States[0].bestChild].QP[x][y];
		}
	}


	MAX_F = States[0].e_fun;

	printf("The computer put a Chessman at: %d,%d\nchessboard:\n", pos_x + 1, pos_y + 1);
	Printchessboard();
	if (IsWin(States[0]) == 1)//计算机赢
	{
		printf("The computer WIN!GAME OVER.\n");
		return TRUE;
	}
	if (MAX_F == NO_BLANK) //棋盘没空可放了，平局。
	{
		printf("DRAW GAME!\n");
		return TRUE;
	}
	return FALSE;
}


void UserInput(int fd_stdin, int fd_stdout)//用户输入落子位置
{

	int n;
	int pos = -1, x, y;
	char szCmd[80] = { 0 };
	printf("Now put your Chessman:\n ");
L1: printf("Please Input The Line Position (x): "); //x坐标
	n = read(fd_stdin, szCmd, 80);
	szCmd[1] = 0;
	atoi(szCmd, &x);
	printf("Please Input The Column Position (y): "); //y坐标
	n = read(fd_stdin, szCmd, 80);
	szCmd[1] = 0;
	atoi(szCmd, &y);
	if (x > 0 && x < 4 && y>0 && y < 4 && States[0].QP[x - 1][y - 1] == 0)
		States[0].QP[x - 1][y - 1] = -1;
	else
	{
		printf("Input Error!\n");
		goto L1;
	}

}
void TicTacToe(int fd_stdin, int fd_stdout)
{
	char buf[80] = { 0 };
	char IsFirst = 0;
	int IsFinish = FALSE;
	chessMenu();
	while (!IsFinish)
	{

		Init();
		printf("chessboard: \n");
		Printchessboard();
		printf("Do you want to be the first one to play chess?(y/n):");
		read(fd_stdin, buf, 2);
		IsFirst = buf[0];
		do {
			if (IsFirst == 'y')
			{
				UserInput(fd_stdin, fd_stdout);
				IsFinish = AutoDone();
			}
			else if (IsFirst == 'n')
			{
				IsFinish = AutoDone();
				if (!IsFinish)
					UserInput(fd_stdin, fd_stdout);
			}
			else if (IsFirst == 'q')
			{
				IsFinish = TRUE;
			}
		} while (!IsFinish);
		if (IsFinish)
		{
			printf("Do you want to quit?(y/n)");
			char c;
			read(fd_stdin, buf, 2);
			c = buf[0];
			if (c == 'n')
			{
				IsFinish = FALSE;
			}
		}
	}
}