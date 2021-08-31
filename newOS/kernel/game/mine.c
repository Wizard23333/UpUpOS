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
							mine
 *======================================================================*/

#define N 15
#define MAX_OF_MINE 40
#define MAX 9999
#define _CRT_SECURE_NO_WARNINGS
struct ROOM
{
	int flag;  //flag为1表示标记为雷  0表示没有被标记
	int num;   //0-8表示周围的雷数 9表示此地有雷
	int found; //0表示未点击区域 1表示点击过的区域
};

struct ROOM room[15][15];
int max = 21;
void room_init() //初始化ROOM结构体
{
	int i = 0, j = 0;
	for (i = 0; i < 15; i++)
	{
		for (j = 0; j < 15; j++)
		{
			room[i][j].flag = 0;
			room[i][j].num = 0;
			room[i][j].found = 0;
		}
	}
}
void print_table() //打印扫雷当前状态
{

	printf("           Mine Clearance Game       \n");
	printf("You can quit anytime by entering [q]\n");
	printf("    1 2 3 4 5 6 7 8 9 101112131415\n");
	printf("  |-------------------------------|\n");
	for (int i = 0; i < N; ++i)
	{
		printf("%2d|", i + 1);
		for (int j = 0; j < N; ++j)
		{
			if (room[i][j].found == 0 && room[i][j].flag == 0)
			{
				printf("-%c", '?');
			}
			else
			{
				if (room[i][j].found == 0 && room[i][j].flag == 1)
				{
					printf("-%c", '*');
				}
				else
				{
					if (room[i][j].num == 0)
					{
						printf("-%c", ' ');
					}
					else
					{
						printf("-%d", room[i][j].num);
					}
				}
			}
		}
		printf("-|\n");
	}
	printf("  |-------------------------------|\n");
}
void room_update_num() //根据雷的位置更新room中的num属性
{
	int i, j;
	for (i = 0; i < 15; i++)
	{
		for (j = 0; j < 15; j++)
		{
			if (room[i][j].num == 9)
			{
				continue;
			}

			if (i - 1 >= 0 && i - 1 <= 14 && j - 1 >= 0 && j - 1 <= 14 && room[i - 1][j - 1].num == 9)
			{
				room[i][j].num++;
			}
			if (i - 1 >= 0 && i - 1 <= 14 && j >= 0 && j <= 14 && room[i - 1][j].num == 9)
			{
				room[i][j].num++;
			}
			if (i - 1 >= 0 && i - 1 <= 14 && j + 1 >= 0 && j + 1 <= 14 && room[i - 1][j + 1].num == 9)
			{
				room[i][j].num++;
			}
			if (i >= 0 && i <= 14 && j - 1 >= 0 && j - 1 <= 14 && room[i][j - 1].num == 9)
			{
				room[i][j].num++;
			}
			if (i >= 0 && i <= 14 && j + 1 >= 0 && j + 1 <= 14 && room[i][j + 1].num == 9)
			{
				room[i][j].num++;
			}
			if (i + 1 >= 0 && i + 1 <= 14 && j - 1 >= 0 && j - 1 <= 14 && room[i + 1][j - 1].num == 9)
			{
				room[i][j].num++;
			}
			if (i + 1 >= 0 && i + 1 <= 14 && j >= 0 && j <= 14 && room[i + 1][j].num == 9)
			{
				room[i][j].num++;
			}
			if (i + 1 >= 0 && i + 1 <= 14 && j + 1 >= 0 && j + 1 <= 14 && room[i + 1][j + 1].num == 9)
			{
				room[i][j].num++;
			}
		}
	}
}
void find_zero(int x, int y) //递归函数 查找x y周围相邻的所有num为0的地区
{
	if (room[x][y].found == 1)
	{
		return;
	}
	room[x][y].found = 1;
	if (x - 1 <= 14 && x - 1 >= 0 && y <= 14 && y >= 0 && room[x - 1][y].num == 0)
	{
		find_zero(x - 1, y);
	}
	if (x - 1 <= 14 && x - 1 >= 0 && y <= 14 && y >= 0 && room[x - 1][y].num != 9)
	{
		room[x - 1][y].found = 1;
	}

	if (x + 1 <= 14 && x + 1 >= 0 && y <= 14 && y >= 0 && room[x + 1][y].num == 0)
	{
		find_zero(x + 1, y);
	}
	if (x + 1 <= 14 && x + 1 >= 0 && y <= 14 && y >= 0 && room[x + 1][y].num != 9)
	{
		room[x + 1][y].found = 1;
	}

	if (x <= 14 && x >= 0 && y - 1 <= 14 && y - 1 >= 0 && room[x][y - 1].num == 0)
	{
		find_zero(x, y - 1);
	}
	if (x <= 14 && x >= 0 && y - 1 <= 14 && y - 1 >= 0 && room[x][y - 1].num != 9)
	{
		room[x][y - 1].found = 1;
	}

	if (x <= 14 && x >= 0 && y + 1 <= 14 && y + 1 >= 0 && room[x][y + 1].num == 0)
	{
		find_zero(x, y + 1);
	}
	if (x <= 14 && x >= 0 && y + 1 <= 14 && y + 1 >= 0 && room[x][y + 1].num != 9)
	{
		room[x][y + 1].found = 1;
	}
}
int input(fd_stdin) //处理输入和输入之后的操作 返回0表示退出游戏 返回1表示未退出
{
	int x, y, z;
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

		printf("input y:");
		r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;
		if (strcmp(rdbuf, "q") == 0 || strcmp(rdbuf, "Q") == 0)
		{
			return 0;
		}
		atoi(rdbuf, &y);

		printf("input z:(1 means choose, 2 means sign)");
		r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;
		if (strcmp(rdbuf, "q") == 0 || strcmp(rdbuf, "Q") == 0)
		{
			return 0;
		}
		atoi(rdbuf, &z);

		if (x < 1 || x > 15 || y < 1 || y > 15 || z < 1 || z > 2)
		{
			printf("Wrong input, please re-enter：\n");
			continue;
		}
		else
		{
			break;
		}
	}
	if (z == 2)
	{
		room[x - 1][y - 1].flag = 1;
	}
	if (z == 1)
	{

		if (room[x - 1][y - 1].num == 0)
		{
			find_zero(x - 1, y - 1);
		}
		else
		{
			room[x - 1][y - 1].found = 1;
		}
	}
	return 1;
}

int judge111() // 遍历整个表判断游戏状态，返回1代表继续游戏 返回2代表游戏失败 返回3代表游戏成功
{
	int i, j;
	int z = 0;
	for (i = 0; i < 15; i++)
	{
		for (j = 0; j < 15; j++)
		{
			if (room[i][j].num == 9 && room[i][j].found == 1)
			{
				z = 2;
			}
		}
	}
	if (z == 2)
	{
		return z;
	}
	z = 3;
	for (i = 0; i < 15; i++)
	{
		for (j = 0; j < 15; j++)
		{
			if (room[i][j].num == 9 && room[i][j].flag == 0)
			{
				z = 0;
			}
		}
	}
	if (z == 3)
	{
		return z;
	}
	return 1;
}
void mine_maker() //随机产生地雷 存储在结构体中
{
	int i, j;
	for (i = 0; i < MAX_OF_MINE; i++)
	{
		j = (max * max) % 225;
		room[j / 15][j - j / 15].num = 9;
		max++;
	}
	if (max > MAX)
	{
		max = 21;
	}
}

int runMine(fd_stdin, fd_stdout)
{
	while (1)
	{

		room_init();
		print_table();
		mine_maker();
		room_update_num();
		while (1)
		{
			/*if (input() == 0) {
				break;
			}*/
			int x, y, z;
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

				printf("input y:");
				r = read(fd_stdin, rdbuf, 70);
				rdbuf[r] = 0;
				if (strcmp(rdbuf, "q") == 0 || strcmp(rdbuf, "Q") == 0)
				{
					return 0;
				}
				atoi(rdbuf, &y);

				printf("input z:(1 means choose, 2 means sign)");
				r = read(fd_stdin, rdbuf, 70);
				rdbuf[r] = 0;
				if (strcmp(rdbuf, "q") == 0 || strcmp(rdbuf, "Q") == 0)
				{
					return 0;
				}
				atoi(rdbuf, &z);

				if (x < 1 || x > 15 || y < 1 || y > 15 || z < 1 || z > 2)
				{
					printf("Wrong input, please re-enter：\n");
					continue;
				}
				else
				{
					break;
				}
			}
			if (z == 2)
			{
				room[x - 1][y - 1].flag = 1;
			}
			if (z == 1)
			{

				if (room[x - 1][y - 1].num == 0)
				{
					find_zero(x - 1, y - 1);
				}
				else
				{
					room[x - 1][y - 1].found = 1;
				}
			}

			if (judge111() == 2 || judge111() == 3)
			{
				break;
			}
			clear();
			print_table();
		}
		if (judge111() == 2)
		{
			printf("game fail\n");
		}
		if (judge111() == 3)
		{
			printf("game success\n");
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
		clear();
	}
	return 0;
}
