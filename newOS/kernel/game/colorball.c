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

#define DELAY_TIME 3000 //延迟时间
#define NULL ((void *)0)
int seed[162] = { 66,4,76,46,46,19,20,72,37,21,40,66,28,56,73,32,22,42,58,79,13,69,47,27,36,70,67,62,45,30,51,48,80,63,14,12,25,10,57,73,53,78,28,8,59,29,27,58,61,34,54,81,15,71,7,3,81,74,17,56,65,14,64,4,41,6,72,23,74,43,60,2,16,64,70,53,38,79,24,43,57,60,49,11,75,5,20,71,68,45,24,48,15,54,33,10,40,63,31,59,42,78,62,23,77,22,13,19,49,65,21,31,69,6,75,11,29,1,47,50,2,36,39,35,3,44,8,26,38,39,55,51,35,30,18,1,52,32,77,44,18,7,34,12,61,9,80,67,9,55,76,41,26,50,52,25,68,37,5,17,16,33 };
int loop = 0;
struct zhu //棋子结构体，包括颜色信息和坐标信息
{
	int colour; //0代表空，1 - 7代表各种颜色
	int x; //横坐标
	int y; //纵坐标
};
#define max 81//表示顺序表申请的空间大小
int enQueue(struct zhu *a, int front, int rear, struct zhu data) {
	//添加判断语句，如果rear超过max，则直接将其从a[0]重新开始存储，如果rear+1和front重合，则表示数组已满
	if ((rear + 1) % max == front) {
		printf("full");
		return rear;
	}
	a[rear % max] = data;
	rear++;
	return rear;
}

int empty(struct zhu* a, int front, int rear) {
	
	return front == rear % max;
}




struct zhu map[9][9]; //游戏棋盘
struct zhu* pp[81]; //指向棋盘空的位置的指针组
int score; //分数
int mark[9][9]; //记录寻找路径时该位置是否走过
int number; //记录棋盘上空的个数

int MAP(); //初始化函数
int randomin(int n); //随机产生n个珠子
int judge(int x, int y, int ifauto); //以一个棋子为中心判断是否组成五个及以上的棋子，ifauto用于区分是随机生成还是玩家移动
void zhudelete(struct zhu *temp);//删除棋子
void move(int inx, int iny, int tox, int toy); //移动棋子，参数依次是初始位置，目标位置
void coutmap(); //输出棋盘
void coutscore(); //输出分数
int readnumber(); //外部读取nember的数值
int findpath(int inx, int iny, int tox, int toy); //判断路径是否可行
int iffine(int inx, int iny, int tox, int toy); //判断输入是否合法


int MAP() //初始化函数
{
	score = 0;
	number = 0;
	memset(map, 0, sizeof(map));
	memset(pp, 0, sizeof(pp));
	memset(mark, 0, sizeof(mark)); //全部重置为0代表未走过
	for (int i1 = 0; i1 <= 8; i1++) //初始化指针、zhu类的颜色坐标值、棋盘
	{
		for (int i2 = 0; i2 <= 8; i2++)
		{
			map[i1][i2].x = i1, map[i1][i2].y = i2; //初始化坐标信息
			map[i1][i2].colour = 0; //初始棋盘全部设定为空
			pp[number] = &map[i1][i2], number++; //pp指针数组依次指向空的棋子，nember记录个数
		}
	}
	return 0;
}

int randomin(int n) //随机产生n个珠子
{
	if (number < 3)n = number; //当空余数量小于3时只填满棋盘
	for (int i = 0; i <= n - 1; i++)
	{
		int temp = seed[(i+loop)%162] % number;
		pp[temp]->colour = seed[(i + loop) % 162] % 7 + 1;
		loop = loop + 1;
		if (loop >= 162)loop = 0;
		if (number >= 3)judge(pp[temp]->x, pp[temp]->y, 1); //判断生成的棋子是否组成5子，true代表是系统生成棋子不用于计分
		number--; //空的数量减少
		pp[temp] = pp[number]; //将最后一位指向空的指针覆盖给当前指针，相当于将此地址从指针数组中删除
	}
	if (n < 3)return 0;
	else return 1; //用于判断游戏是否结束
}

int judge(int x, int y, int ifauto) //判断五子,调用删除函数删除棋子，记录得分
{
	int tempx = x, tempy = y;
	int colour = map[x][y].colour; //临时保存当前位置坐标和颜色
	int up = 0, down = 0, left = 0, right = 0, lup = 0, rup = 0, ldown = 0, rdown = 0; //分别代表各个方向上连续同向的棋子个数
	while (y - 1 >= 0 && map[tempx][y - 1].colour == colour) //向上判断
	{
		up++;
		y--;
	}
	y = tempy; //坐标回归原位用于其他方向判断
	while (y + 1 <= 8 && map[tempx][y + 1].colour == colour) //向下判断
	{
		down++;
		y++;
	}
	y = tempy;
	while (x - 1 >= 0 && map[x - 1][tempy].colour == colour) //向左判断
	{
		left++;
		x--;
	}
	x = tempx;
	while (x + 1 <= 8 && map[x + 1][tempy].colour == colour) //向右判断
	{
		right++;
		x++;
	}
	x = tempx;
	while (x + 1 <= 8 && y + 1 <= 8 && map[x + 1][y + 1].colour == colour) //向右下判断
	{
		rdown++;
		x++;
		y++;
	}
	x = tempx;
	y = tempy;
	while (x + 1 <= 8 && y - 1 >= 0 && map[x + 1][y - 1].colour == colour) //向右上判断
	{
		rup++;
		x++;
		y--;
	}
	x = tempx;
	y = tempy;
	while (x - 1 >= 0 && y - 1 >= 0 && map[x - 1][y - 1].colour == colour) //向左上判断
	{
		lup++;
		x--;
		y--;
	}
	x = tempx;
	y = tempy;
	while (x - 1 >= 0 && y + 1 <= 8 && map[x - 1][y + 1].colour == colour) //向左下判断
	{
		ldown++;
		x--;
		y++;
	}
	x = tempx;
	y = tempy;
	int i = 0;
	for (int n = up + down; n >= 4 && i <= up;) //满足上下5子时向上删除
	{
		zhudelete(&map[x][y]); //调用删除函数
		y--;
		i++;
	}
	x = tempx;
	y = tempy;
	i = 0;
	for (int n = up + down; n >= 4 && i <= down;) //满足上下5子时向下删除
	{
		zhudelete(&map[x][y]);
		y++;
		i++;
	}
	x = tempx;
	y = tempy;
	i = 0;
	for (int n = right + left; n >= 4 && i <= left;) //向左删除
	{
		zhudelete(&map[x][y]);
		x--;
		i++;
	}
	x = tempx;
	y = tempy;
	i = 0;
	for (int n = right + left; n >= 4 && i <= right;) //向右删除
	{
		zhudelete(&map[x][y]);
		x++;
		i++;
	}
	x = tempx;
	y = tempy;
	i = 0;
	for (int n = rdown + lup; n >= 4 && i <= rdown;) //向右下删除
	{
		zhudelete(&map[x][y]);
		x++;
		y++;
		i++;
	}
	x = tempx;
	y = tempy;
	i = 0;
	for (int n = rdown + lup; n >= 4 && i <= lup;) //向左上删除
	{
		zhudelete(&map[x][y]);
		x--;
		y--;
		i++;
	}
	x = tempx;
	y = tempy;
	i = 0;
	for (int n = rup + ldown; n >= 4 && i <= rup;) //向右上删除
	{
		zhudelete(&map[x][y]);
		x++;
		y--;
		i++;
	}
	x = tempx;
	y = tempy;
	i = 0;
	for (int n = rup + ldown; n >= 4 && i <= ldown;) //向左下删除
	{
		zhudelete(&map[x][y]);
		x--;
		y++;
		i++;
	}
	x = tempx;
	y = tempy;
	int dnember = 0; //删除的棋子总数
	if (up + down >= 4)dnember = dnember + up + down;
	if (left + right >= 4)dnember = dnember + left + right;
	if (rup + ldown >= 4)dnember = dnember + rup + ldown;
	if (lup + rdown >= 4)dnember = dnember + lup + rdown; //各个方向删除的棋子个数
	if (dnember > 0)dnember++; //算上自身
	if (ifauto == 0 && dnember > 0)score = score + (dnember - 5) * 2 + 10; //当ifauto为假即是玩家移动时计算得分
	if (dnember > 0)return 1; //形成五子返回true
	else return 0;
}

void zhudelete(struct zhu *temp) //删除棋子
{
	pp[number] = temp; //在末尾添加指向此棋子的地址
	temp->colour = 0;
	number++; //空个数增加
}

void move(int inx, int iny, int tox, int toy) //移动棋子
{
	map[tox - 1][toy - 1].colour = map[inx - 1][iny - 1].colour; //目的棋子颜色复制
	map[inx - 1][iny - 1].colour = 0; //原棋子
	for (int i = 0; i <= number - 1; i++) //指向空棋子的指针转移
	{
		if (pp[i]->x == tox - 1 && pp[i]->y == toy - 1)pp[i] = &map[inx - 1][iny - 1];
	}
}

void coutmap() //输出棋盘
{
	printf("    1   2   3   4   5   6   7   8   9   X\n");
	for (int i2 = 0; i2 <= 8; i2++)
	{
		printf("  -------------------------------------\n");
		printf("%d ", i2 + 1);
		for (int i1 = 0; i1 <= 8; i1++)
		{
			printf("|");
			switch (map[i1][i2].colour)
			{
			case 0: printf("   "); break;
			case 1: printf(" A "); break;
			case 2: printf(" B "); break;
			case 3: printf(" C "); break;
			case 4: printf(" D "); break;
			case 5: printf(" E "); break;
			case 6: printf(" F "); break;
			case 7: printf(" G "); break;
			}
		}
		printf("|\n");
	}
	printf("  -------------------------------------\n");
	printf("Y         ");
	
}

void coutscore() //输出得分
{
	printf("score:%d\n", score);
}

int readnumber() //读取空的个数用于判断是否结束游戏
{
	return number;
}

int findpath(int inx, int iny, int tox, int toy) //使用bfs算法判断路径，inx,iny为原位置,tox,toy为目的位置
{
	struct zhu a[max];
    int front,rear; 
	front = rear = 0;//创建一个棋子类的队列，用于保存搜索到的位置
	inx--, iny--, tox--, toy--;
	memset(mark, 0, sizeof(mark)); //全部重置为0代表未走过
	int success = 0; //用于记录是否寻找到目的地
	rear = enQueue(a, front, rear, map[inx][iny]);
	while (!empty(a, front, rear)) //当队列为空时结束搜索
	{
		struct zhu temp = a[front]; //读取队列第一个棋子
		front = (front + 1) % max;
		if (temp.x == tox && temp.y == toy) { success = 1; break; } //找到目标地点时停止
		mark[temp.x][temp.y] = 1; //标记读取位置已经走过防止走回头路
		for (int tempx = -1; tempx <= 1; tempx++)
		{
			for (int tempy = -1; tempy <= 1; tempy++)
			{
				if (tempy != tempx && tempx != -tempy) //分别选择上下左右四个方向寻路
				{
					int x, y;
					x = temp.x + tempx, y = temp.y + tempy; //分别选择上下左右四个方向寻路
					if (x >= 0 && x <= 8 && y >= 0 && y <= 8 && mark[x][y] == 0 && map[x][y].colour == 0) //寻找不超出范围且为空且没有走过的路
					{
						mark[x][y] = 1; //标记读取位置已经走过防止走回头路
						rear = enQueue(a, front, rear, map[x][y]);
					}
				}
			}
		}
	}
	if (success)return 1; //找到目的坐标返回true
	else return 0;
}

int iffine(int inx, int iny, int tox, int toy) //判断输入坐标是否合法，包括是否超出范围，原位置是否有棋子，目的位置是否为空
{
	if (map[inx - 1][iny - 1].colour == 0 || map[tox - 1][toy - 1].colour != 0 || inx < 1 || inx>9 || iny < 1 || iny>9 || tox < 1 || tox>9 || toy < 1 || toy>9)
		return 0;
	else return 1;
}

int colorBall(fd_stdin)
{
	MAP(); //创建棋盘
	char rdbuf[128];
	int inx, iny, tox, toy, n;
	clear();
	printf("Please enter the initial number of pieces in the game(1-81):");
	int r = read(fd_stdin, rdbuf, 70);
	rdbuf[r] = 0;
	if (strcmp(rdbuf, "q") == 0 || strcmp(rdbuf, "Q") == 0)
	{
		return 0;
	}
	atoi(rdbuf, &n);
	while (n <= 0 || n > 81 || r<1) 
	{
		printf("Please input a valid number!Input again\n");
		printf("Please enter the initial number of pieces in the game(1-81):");
		int r = read(fd_stdin, rdbuf, 70);
	        rdbuf[r] = 0;
	        if (strcmp(rdbuf, "q") == 0 || strcmp(rdbuf, "Q") == 0)
	        {
		 	return 0;
	        }
	atoi(rdbuf, &n);
	}
	randomin(n); //随机产生棋子
	while (1)
	{
		clear(); //清屏
		coutmap(); //输出棋盘
		coutscore(); //输出得分
		if (readnumber() == 0)break; //棋盘全满时终止游戏

		printf("input Chess coordinates X:");
		int r1 = read(fd_stdin, rdbuf, 70);
		rdbuf[r1] = 0;
		if (strcmp(rdbuf, "q") == 0 || strcmp(rdbuf, "Q") == 0)
		{
			return 0;
		}
		atoi(rdbuf, &inx);

		printf("input Chess coordinates Y:");
		int r2 = read(fd_stdin, rdbuf, 70);
		rdbuf[r2] = 0;
		if (strcmp(rdbuf, "q") == 0 || strcmp(rdbuf, "Q") == 0)
		{
			return 0;
		}
		atoi(rdbuf, &iny);

		printf("input destination coordinates X:");
		int r3 = read(fd_stdin, rdbuf, 70);
		rdbuf[r3] = 0;
		if (strcmp(rdbuf, "q") == 0 || strcmp(rdbuf, "Q") == 0)
		{
			return 0;
		}
		atoi(rdbuf, &tox);

		printf("input destination coordinates Y:");
		int r4 = read(fd_stdin, rdbuf, 70);
		rdbuf[r4] = 0;
		if (strcmp(rdbuf, "q") == 0 || strcmp(rdbuf, "Q") == 0)
		{
			return 0;
		}
		atoi(rdbuf, &toy);

		while (iffine(inx, iny, tox, toy) == 0 || findpath(inx, iny, tox, toy) == 0 || r1<1 || r2<1 || r3<1 || r4<1)
		{
			if (iffine(inx, iny, tox, toy) == 0)printf("Please input a valid coordinate!Input again\n");
			if (findpath(inx, iny, tox, toy) == 0)printf("No feasible path found!Input again\n");
			printf("input Chess coordinates X:");
			r = read(fd_stdin, rdbuf, 70);
			rdbuf[r] = 0;
			if (strcmp(rdbuf, "q") == 0 || strcmp(rdbuf, "Q") == 0)
			{
				return 0;
			}
			atoi(rdbuf, &inx);

			printf("input Chess coordinates Y:");
			r = read(fd_stdin, rdbuf, 70);
			rdbuf[r] = 0;
			if (strcmp(rdbuf, "q") == 0 || strcmp(rdbuf, "Q") == 0)
			{
				return 0;
			}
			atoi(rdbuf, &iny);

			printf("input destination coordinates X:");
			r = read(fd_stdin, rdbuf, 70);
			rdbuf[r] = 0;
			if (strcmp(rdbuf, "q") == 0 || strcmp(rdbuf, "Q") == 0)
			{
				return 0;
			}
			atoi(rdbuf, &tox);

			printf("input destination coordinates Y:");
			r = read(fd_stdin, rdbuf, 70);
			rdbuf[r] = 0;
			if (strcmp(rdbuf, "q") == 0 || strcmp(rdbuf, "Q") == 0)
			{
				return 0;
			}
			atoi(rdbuf, &toy);
		}
		move(inx, iny, tox, toy); //调用移动棋子函数
		clear();
		coutmap(); //输出移动后的情况
		milli_delay(DELAY_TIME); //暂停一秒
		if (judge(tox - 1, toy - 1, 0) == 0) //判断移动位置是否形成五子
		{
			if (randomin(3) == 0)break; //棋盘空不足三个时填满并结束游戏
		}
		else
		{
			clear();
			coutmap(); //输出清除棋子后的情况
			milli_delay(DELAY_TIME);
			printf("\n");
			printf("Congratulations on getting an extra move!\n");
			milli_delay(DELAY_TIME);
		}
	}
	clear();
	coutmap();
	printf("\n");
	printf("Game over!\n");
	printf("Final ");
	coutscore();
}
