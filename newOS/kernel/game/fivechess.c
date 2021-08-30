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

/*****************************************************************************
 *                               five chess
 *****************************************************************************/

#define true 1
#define false 0
int N = 15;					 //15*15的棋盘
const char ChessBoard = '+'; //棋盘标志
const char flag1 = 'o';		 //玩家1标志
const char flag2 = 'x';		 //玩家2或电脑标志
int me = true;				 //true黑子， false， 白子
int over = false;
char box[15][15] = {}; //全部落子点

int init()
{
	me = true; //true黑子， false， 白子
	over = false;

	for (int i = 0; i < 15; i++)
	{
		for (int j = 0; j < 15; j++)
		{
			box[i][j] = ChessBoard;
		}
	}
}
int mode[][2] = {
	{1, 0}, //水平方向
	{0, 1}, //垂直方向
	{1, 1}, //右下 左上
	{1, -1} //右上 左下
};

int check(int x, int y, char player, int mode[2])
{
	// console.log(x,y,color,maparr[x][y]);
	int count = 0;
	for (int i = 1; i < 5; i++)
	{

		if (box[x + i * mode[0]][y + i * mode[1]] == player)
		{
			count++;
		}
		else
		{
			break;
		}
	}
	for (int i = 1; i < 5; i++)
	{
		if (box[x - i * mode[0]][y - i * mode[1]] == player)
		{
			count++;
		}
		else
		{
			break;
		}
	}
	return count;
}
void PrintChessBoard()
{ //打印棋盘
	clear();
	printf("           Five Chess Game       \n");
	printf("You can quit anytime by entering [q]\n");
	printf("    1 2 3 4 5 6 7 8 9 101112131415\n");
	printf("  |-------------------------------|\n");
	for (int i = 0; i < N; ++i)
	{
		printf("%2d|", i + 1);
		for (int j = 0; j < N; ++j)
			printf("-%c", box[j][i]);
		printf("-|\n");
	}
	printf("  |-------------------------------|\n");
}

int JudgeValue(int x, int y)
{	//判断坐标的合法性
	//1.在棋盘上
	if (x >= 0 && x < N && y >= 0 && y < N)
	{
		//2.所在位置为空（没有棋子）
		if (box[x][y] == ChessBoard)
		{
			return 1; //合法
		}
	}
	return 0; //非法
}

void user(int x, int y)
{
	if (over)
		return;
	if (!me)
		return;
	int temp = 0;
	if (box[x][y] == ChessBoard)
	{ //判断没有落子

		box[x][y] = flag1;
		//PrintChessBoard();
		for (int l = 0; l < 4; l++)
		{
			if (temp < check(x, y, flag1, mode[l]))
				temp = check(x, y, flag1, mode[l]);

			if (temp >= 4)
			{
				printf("player win\n");

				over = true;
				return;
			}
		}
		if (!over)
		{
			me = !me;
			return;
		}
	}
}

//计算机
int computerAI()
{
	int myScore[15][15];	   //我方分数
	int computerScore[15][15]; //计算机分数
	int max = 0;			   //最大分数
	int u = 0, v = 0;		   //最大分数点
	int temp = 0;
	for (int i = 0; i < 15; i++)
	{

		for (int j = 0; j < 15; j++)
		{
			myScore[i][j] = 0;
			computerScore[i][j] = 0;
		}
	}

	for (int i = 0; i < 15; i++)
	{
		for (int j = 0; j < 15; j++)
		{
			if (box[i][j] == ChessBoard)
			{ //每个空闲点上进行计算分数
				temp = 0;
				for (int l = 0; l < 4; l++)
					if (temp < check(i, j, flag1, mode[l]))
						temp = check(i, j, flag1, mode[l]);
				if (temp == 1)
				{
					myScore[i][j] += 200;
				}
				else if (temp == 2)
				{
					myScore[i][j] += 400;
				}
				else if (temp == 3)
				{
					myScore[i][j] += 2000;
				}
				else if (temp == 4)
				{
					myScore[i][j] += 10000;
				}
				temp = 0;
				for (int l = 0; l < 4; l++)
					if (temp < check(i, j, flag2, mode[l]))
						temp = check(i, j, flag2, mode[l]);
				if (temp == 1)
				{
					computerScore[i][j] += 220;
				}
				else if (temp == 2)
				{
					computerScore[i][j] += 420;
				}
				else if (temp == 3)
				{
					computerScore[i][j] += 2100;
				}
				else if (temp == 4)
				{
					computerScore[i][j] += 20000;
				}
			}

			//得出最大分数的点，并赋给u,v
			if (myScore[i][j] > max)
			{
				max = myScore[i][j];
				u = i;
				v = j;
			}
			else if (myScore[i][j] == max)
			{
				if (computerScore[i][j] > computerScore[u][v])
				{
					u = i;
					v = j;
				}
			}

			if (computerScore[i][j] > max)
			{
				max = computerScore[i][j];
				u = i;
				v = j;
			}
			else if (computerScore[i][j] == max)
			{
				if (myScore[i][j] > myScore[u][v])
				{
					u = i;
					v = j;
				}
			}

		} //所有空闲点上进行计算分数
	}
	box[u][v] = flag2;

	PrintChessBoard();

	for (int l = 0; l < 4; l++)
	{
		int temp1 = 0;
		if (temp1 < check(u, v, flag2, mode[l]))
			temp1 = check(u, v, flag2, mode[l]);

		if (temp1 >= 4)
		{
			printf("computer win\n");

			over = true;
			return 0;
		}
	}

	if (!over)
	{
		me = !me;
		return v;
	}
}

int fiveChess(fd_stdin)
{
	int x, y;
	init();
	int v;
	//PrintChessBoard();
	while (1)
	{
		init();
		while (!over)
		{

			PrintChessBoard();
			while (1)
			{

				char rdbuf[128];
				int r = 0;

				printf("input x:");
				r = read(fd_stdin, rdbuf, 70);
				rdbuf[r] = 0;
				if (strcmp(rdbuf, "q") == 0 || strcmp(rdbuf, "Q") == 0)
				{
					return 0;
				}
				atoi(rdbuf, &x);
				if (r > 2)
				{
					x = -1;
				}

				printf("input y:");
				r = read(fd_stdin, rdbuf, 70);
				rdbuf[r] = 0;
				if (strcmp(rdbuf, "q") == 0 || strcmp(rdbuf, "Q") == 0)
				{
					return 0;
				}
				atoi(rdbuf, &y);
				if (JudgeValue(x - 1, y - 1) == 1)
				{ //判断坐标是否合法
					break;
				}
				printf("Wrong input, please re-enter：\n");
			}
			user(x - 1, y - 1);
			v = computerAI();
		}
		printf("input any(except q/Q) again:");
		char rdbuf[128];
		int r = 0;
		r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;
		if (strcmp(rdbuf, "q") == 0 || strcmp(rdbuf, "Q") == 0)
		{
			return 0;
		}
	}
}
