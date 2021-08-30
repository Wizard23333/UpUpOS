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
int answer[9][9] = {0};
int now[9][9] = { 0 };
int x = 4;
struct room {
	int figure;
	struct room* front;
	struct room* next; 
	int x;
	int y;
};
void print_table111() //打印now当前状态
{

	printf("         Sudoku  Game        \n");
	printf("You can quit anytime by entering [q]\n");
	printf("    1 2 3   4 5 6   7 8 9 \n");
	printf("  |-------|-------|-------|\n");
	for (int i = 0; i < 9; i++)
	{
		if (i%3==0&&i!=0) {
			printf("  |-------|-------|-------|\n");
			
		}
		printf("%2d|", i + 1);
		for (int j = 0; j < 9; ++j) {
			if (j%3==0&&j!=0) {
				printf("-|");
			}
		     printf("-%d", now[i][j]);
		}
		printf("-|\n");
	}
	printf("  |-------|-------|-------|\n");

}

void answer_maker()   //产生答案
{
	int i, j ;
	              //首先随机产生1的所有位置
	for (i=0; i < 9; i++) {
			if (judge222(1,i,x%9)==1) {
				answer[i][x % 9] = 1;
			}
			else {
				i--;
			}
			x++;
	}
	struct room first[9][9];
	for (i = 0; i < 9;i++) {
		for (j = 0; j < 9; j++) {
			first[i][j].figure = i+1;
			first[i][j].x = j;
			first[i][j].y = -1;
		}
	}
	i = 1, j = 0;
	first[i][j].figure = 2;
	first[i][j].x = 0;
	first[i][j].y = -1;
	while (1) {
		if (find_next(first[i][j].figure, first[i][j].x, first[i][j].y) != -1) {
			if (first[i][j].y != -1) {
				answer[first[i][j].x][first[i][j].y] = 0;
			}

			first[i][j].y = find_next(first[i][j].figure, first[i][j].x, first[i][j].y);
			answer[first[i][j].x][first[i][j].y] = first[i][j].figure;
			if (i == 8 && j == 8) {
				break;
			}
			if (j == 8) {
				j = 0;
				i++;
			}
			else {
				j++;
			}
		}
		else {
			if (first[i][j].y != -1) {
				answer[first[i][j].x][first[i][j].y] = 0;
			}
			first[i][j].y = -1;
			if (j==0) {
				i--;
				j = 8;
			}
			else {
				j--;
			}
		}
	}
	i = 1;
	j = 0;

	
}
int find_next(int figure, int x, int y) //寻找下一个可选位置 -1表示没有可选位置了 0-8表示可选位置的y值
{
	int i;
	for (i=y+1;i<9;i++) {
		if (judge222(figure,x,i)==1) {
			return i;
		}
	}
	return -1;
}
int judge222(int rand,int x,int y)    // 1正确 0不正确
{
	if (answer[x][y]!=0) {
		return 0;
	}
	/*if (rand == answer[x][0] || rand == answer[x][1] || rand == answer[x][2] || rand == answer[x][3] || rand == answer[x][4] || rand == answer[x][5] || rand == answer[x][6] || rand == answer[x][7] || rand == answer[x][8]) {
		return 0;
	}*/
	if (rand == answer[0][y] || rand == answer[1][y] || rand == answer[2][y] || rand == answer[3][y] || rand == answer[4][y] || rand == answer[5][y] || rand == answer[6][y] || rand == answer[7][y] || rand == answer[8][y]) {
		return 0;
	}
	if (rand==answer[(x / 3) * 3][(y / 3)*3]|| rand == answer[(x / 3) * 3][(y / 3) * 3 +1] || rand == answer[(x / 3) * 3][(y / 3) * 3 +2] || rand == answer[(x / 3) * 3 +1][(y / 3) * 3] || rand == answer[(x / 3) * 3 +1][(y / 3) * 3 +1] || rand == answer[(x / 3) * 3 +1][(y / 3) * 3 +2] || rand == answer[(x / 3) * 3 +2][(y / 3) * 3] || rand == answer[(x / 3) * 3 +2][(y / 3) * 3 +1] || rand == answer[(x / 3)*3+2][(y / 3) * 3 +2]) {
		return 0;
	}
	return 1;
}
void dig()  //随机打洞
{
	int i, j;
	for (i=0;i<9;i++) {
		for (j=0;j<9;j++) {
			if ((x*x+x%2-1)%3==0) {
				now[i][j] = answer[i][j];
			}
			else {
				now[i][j] = 0;
			}
			x++;
		}
	}
}

int judge_end() //遍历整个表判断游戏状态 1代表游戏成功 2代表继续
{
	int i, j;
	int sum=0;
	for (i=0;i<9;i++) {
		for (j=0;j<9;j++) {
			sum = now[i][0] + now[i][1] + now[i][2] + now[i][3] + now[i][4] + now[i][5] + now[i][6] + now[i][7] + now[i][8];
			if (sum!=45) {
				return 2;
			}
			sum = now[0][j] + now[1][j] + now[2][j] + now[3][j] + now[4][j] + now[5][j] + now[6][j] + now[7][j] + now[7][j];
			if (sum != 45) {
				return 2;
			}
			sum = now[(i / 3) * 3][(j / 3) * 3] + now[(i / 3) * 3][(j / 3) * 3+1] + now[(i / 3) * 3][(j / 3) * 3+2] + now[(i / 3) * 3+1][(j / 3) * 3] + now[(i / 3) * 3+1][(j / 3) * 3+1] + now[(i / 3) * 3+1][(j / 3) * 3+2] + now[(i / 3) * 3+2][(j / 3) * 3] + now[(i / 3) * 3+2][(j / 3) * 3+1] + now[(i / 3) * 3+2][(j / 3) * 3+2];
			if (sum != 45) {
				return 2;
			}
		}
	}
	return 1;
}

void runSudoku(fd_stdin, fd_stdout)
{
	while (1) {
		answer_maker();
		dig();
		print_table111();
		while (1) {
			int x, y, z;
	while (1) {
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
		if (r > 2)
		{
			y = -1;
		}
		
		printf("input z:");
		r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;
		if (strcmp(rdbuf, "q") == 0 || strcmp(rdbuf, "Q") == 0)
		{
			return 0;
		}
		atoi(rdbuf, &z);
		if (r > 2)
		{
			z = -1;
		}
		
		

		if (x < 1 || x>9 || y < 1 || y>9 || z < 0 || z>9) {
			printf("Wrong input, please re-enter：\n");
			continue;
		}
		else {
			break;
		}
	}
	
	now[x-1][y-1] = z;
			
			
			
			
			
			
			
			
			

			if (judge_end() == 1 ) {
				break;
			}
			clear();
			print_table();
		}
		
		if (judge_end() == 1) {
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


