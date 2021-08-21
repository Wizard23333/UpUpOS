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
#define WIDTH 15
#define HIGHT 15
#define WHITE 0
#define BLACK 1
#define WIN 2

char WHITE_FLAG = 'O';//大写o
char BLACK_FLAG = 'X';

char map[WIDTH][HIGHT];
int situationPC[WIDTH][HIGHT] = { 0 };  //situation
int situationPlayer[WIDTH][HIGHT] = { 0 };

int player = BLACK;

void init_map();
void draw_pc_map();
void draw_player_map();
void draw_map_chess();
int horizontal(int row, int col, char whoFlag);
int vertical(int row, int col, char whoFlag);
int leftSlope(int row, int col, char whoFlag);
int rightSlope(int row, int col, char whoFlag);
int result(int left, int right, int count, int k, char num);
void pcLoad();
void benefit();
int playerLoad(fd_stdin);
int win();


int fiveChess(fd_stdin) {
	int result;
	int nums;
	init_map();

	draw_map_chess();

	for (nums = 0; nums < 225; nums++) {
		if (player == BLACK) {
			//玩家下子
			printf("please white(%c) play\n", WHITE_FLAG);
			player = WHITE;
			result = playerLoad(fd_stdin);
			if (!result) break;
		}
		else {
			//电脑下子
			player = BLACK;
			pcLoad();
		}

		draw_map_chess();
		if (win()) {
			if (player == WHITE) {
				printf("WHITE(%c) is winner.\n", WHITE_FLAG);
			}
			else {
				printf("BLACK(%c) is winner.\n", BLACK_FLAG);
			}
			printf("Enter anything to exit...\n");
			char rdbuf[128];
			int r = read(fd_stdin, rdbuf, 70);
			rdbuf[r] = 0;
			while (r < 1)
			{
				r = read(fd_stdin, rdbuf, 70);
				rdbuf[r] = 0;
			}

			break;
		}

	}

	clear();
}

void init_map() {
	int i = 0;
	int j = 0;
	for (i = 0; i < HIGHT; i++) {
		for (j = 0; j < WIDTH; j++) {
			map[i][j] = '+';
		}
	}
}

void draw_pc_map() {
	int i, j, k;

	for (i = 0; i < WIDTH; i++) {
		for (j = 0; j < WIDTH; j++) {
			printf("%d", situationPC[i][j]);
			if (j < (WIDTH - 1)) printf("-");
		}
		printf("\n");
	}
}

void draw_player_map() {
	int i = 0, j = 0;

	for (i = 0; i < WIDTH; i++) {
		for (j = 0; j < WIDTH; j++) {
			printf("%d", situationPlayer[i][j]);
			if (j < (WIDTH - 1)) printf("-");
		}
		printf("\n");
	}
}

void draw_map_chess() {
	int i = 0, j = 0, k = 0;

	clear();

	printf("           Five Chess Game       \n");
	printf("You can quit anytime by entering [q]\n");
	printf("    X--------------------------->\n");
	printf("    ");
	for (k = 0; k < HIGHT; k++) {
		if (k >= 0 && k <= 8) {
			printf("%d ", k + 1);
		}
		else {
			printf("%d", k + 1);
		}
	}
	printf("\n");
	for (i = 0; i < WIDTH; i++) {
		printf("Y");
		if (i >= 0 && i <= 8) {
			printf("0%d ", i + 1);
		}
		else {
			printf("%d ", i + 1);
		}
		for (j = 0; j < WIDTH; j++) {
			printf("%c", map[i][j]);
			if (j < (WIDTH - 1)) printf("-");
		}
		printf("\n");
	}
}

int horizontal(int row, int col, char whoFlag) {
	int spaceNum = 0;//空白数
	int count = 1;//几连，包含当前要下的子
	int leftHad = 0;//左边是否有同子
	int x = row;
	int y = col;
	int liveLeft = 0;
	int liveRight = 0;

	if (map[row][col] != '+') {
		return 0;
	}
	while (y > 0 && (map[x][y - 1] == '+' || map[x][y - 1] == whoFlag)) {
		if (map[x][y - 1] == '+' && spaceNum < 1) {//第一个空白
			if (map[x][y - 2] != whoFlag) {
				liveLeft = 1;
				break;
			}
			spaceNum++;
			y--;
		}
		else if (map[x][y - 1] == whoFlag) {
			leftHad = 1;
			y--;
			count++;
		}
		else {//第2个空白
			liveLeft = 1;
			break;
		}
	}

	//如果左边没有同色子，设置空白数为0
	if (!leftHad) {
		spaceNum = 0;
	}

	y = col;
	while (y < 14 && (map[x][y + 1] == '+' || map[x][y + 1] == whoFlag)) {
		if (map[x][y + 1] == '+' && spaceNum < 1) {//第一个空白
			if (map[x][y + 2] != whoFlag) {
				liveRight = 1;
				break;
			}
			spaceNum++;
			y++;
		}
		else if (map[x][y + 1] == '+' && spaceNum > 0) {//第2个空白
			liveRight = 1;
			break;
		}
		else {
			y++;
			count++;
		}

	}
	return result(liveLeft, liveRight, count, spaceNum, whoFlag);
}

int vertical(int row, int col, char whoFlag) {
	int spaceNum = 0;//空白数
	int count = 1;//几连，包含当前要下的子
	int topHad = 0;//上边是否有同子
	int x = row;
	int y = col;
	int liveLeft = 0;
	int liveRight = 0;

	if (map[row][col] != '+') {
		return 0;
	}
	while (x > 0 && (map[x - 1][y] == '+' || map[x - 1][y] == whoFlag)) {
		if (map[x - 1][y] == '+' && spaceNum < 1) {//第一个空白
			if (map[x - 2][y] != whoFlag) {
				liveLeft = 1;
				break;
			}
			spaceNum++;
			x--;
		}
		else if (map[x - 1][y] == whoFlag) {
			topHad = 1;
			x--;
			count++;
		}
		else {//第2个空白
			liveLeft = 1;
			break;
		}
	}

	//如果左边没有同色子，设置空白数为0
	if (!topHad) {
		spaceNum = 0;
	}
	x = row;
	while (x < 14 && (map[x + 1][y] == '+' || map[x + 1][y] == whoFlag)) {
		if (map[x + 1][y] == '+' && spaceNum < 1) {//第一个空白
			if (map[x + 2][y] != whoFlag) {
				liveRight = 1;
				break;
			}
			spaceNum++;
			x++;
		}
		else if (map[x + 1][y] == '+' && spaceNum > 0) {//第2个空白
			liveRight = 1;
			break;
		}
		else {
			x++;
			count++;
		}

	}
	return result(liveLeft, liveRight, count, spaceNum, whoFlag);
}

// +-+-+-@-+-+
// +-+-@-+-+
// +-@-+-+-+
int leftSlope(int row, int col, char whoFlag) {
	int spaceNum = 0;//空白数
	int count = 1;//几连，包含当前要下的子
	int topHad = 0;//上边是否有同子
	int x = row;
	int y = col;
	int liveLeft = 0;
	int liveRight = 0;

	if (map[row][col] != '+') {
		return 0;
	}
	//向下
	while (x < 14 && y>0 && (map[x + 1][y - 1] == '+' || map[x + 1][y - 1] == whoFlag)) {
		if (map[x + 1][y - 1] == '+' && spaceNum < 1) {//第一个空白
			if (map[x + 2][y - 2] != whoFlag) {
				liveLeft = 1;
				break;
			}
			spaceNum++;
			x++;
			y--;
		}
		else if (map[x + 1][y - 1] == whoFlag) {
			topHad = 1;
			x++;
			y--;
			count++;
		}
		else {//第2个空白
			liveLeft = 1;
			break;
		}
	}

	//如果上边没有同色子，设置空白数为0
	if (!topHad) {
		spaceNum = 0;
	}

	x = row;
	y = col;

	//向上
	while (x > 0 && y < 14 && (map[x - 1][y + 1] == '+' || map[x - 1][y + 1] == whoFlag)) {
		if (map[x - 1][y + 1] == '+' && spaceNum < 1) {//第一个空白
			if (map[x - 2][y + 2] != whoFlag) {
				liveRight = 1;
				break;
			}
			spaceNum++;
			x--;
			y++;
		}
		else if (map[x - 1][y + 1] == '+' && spaceNum > 0) {//第2个空白
			liveRight = 1;
			break;
		}
		else {
			x--;
			y++;
			count++;
		}

	}
	return result(liveLeft, liveRight, count, spaceNum, whoFlag);
}

int rightSlope(int row, int col, char whoFlag) {
	int spaceNum = 0;//空白数
	int count = 1;//几连，包含当前要下的子
	int topHad = 0;//上边是否有同子
	int x = row;
	int y = col;
	int liveLeft = 0;
	int liveRight = 0;

	if (map[row][col] != '+') {
		return 0;
	}
	//向上
	while (x > 0 && y > 0 && (map[x - 1][y - 1] == '+' || map[x - 1][y - 1] == whoFlag)) {
		if (map[x - 1][y - 1] == '+' && spaceNum < 1) {//第一个空白
			if (map[x - 2][y - 2] != whoFlag) {
				liveLeft = 1;
				break;
			}
			spaceNum++;
			x--;
			y--;
		}
		else if (map[x - 1][y - 1] == whoFlag) {
			topHad = 1;
			x--;
			y--;
			count++;
		}
		else {//第2个空白
			liveLeft = 1;
			break;
		}
	}

	//如果上边没有同色子，设置空白数为0
	if (!topHad) {
		spaceNum = 0;
	}

	x = row;
	y = col;

	//向下
	while (x < 14 && y < 14 && (map[x + 1][y + 1] == '+' || map[x + 1][y + 1] == whoFlag)) {
		if (map[x + 1][y + 1] == '+' && spaceNum < 1) {//第一个空白
			if (map[x + 2][y + 2] != whoFlag) {
				liveRight = 1;
				break;
			}
			spaceNum++;
			x++;
			y++;
		}
		else if (map[x + 1][y + 1] == '+' && spaceNum > 0) {//第2个空白
			liveRight = 1;
			break;
		}
		else {
			x++;
			y++;
			count++;
		}

	}
	return result(liveLeft, liveRight, count, spaceNum, whoFlag);
}

int result(int left, int right, int count, int k, char num) {
	if (count == 1) {
		return 1;
	}
	else if (count == 2) {
		if (left && right) {//左右两边都是空的
			if (k == 0) {
				//电脑60
				return num == BLACK_FLAG ? 60 : 50;
			}
			else {
				return num == BLACK_FLAG ? 40 : 35;
			}
		}
		else if (!left && !right) {
			return 1;
		}
		else {
			return 10;
		}
	}
	else if (count == 3) {

		if (left && right) {//左右两边都是空的
			if (k == 0) {
				//电脑950
				return num == BLACK_FLAG ? 950 : 700;
			}
			else {
				return num == BLACK_FLAG ? 900 : 650;
			}
		}
		else if (!left && !right) {
			return 1;
		}
		else {
			return 100;
		}
	}
	else if (count == 4) {
		if (left && right) {//左右两边都是空的
			if (k == 0) {
				return num == BLACK_FLAG ? 6000 : 3500;
			}
			else {
				return num == BLACK_FLAG ? 5000 : 3000;
			}
		}
		else if (!left && !right) {
			return 1;
		}
		else {
			if (k == 0) {
				return num == BLACK_FLAG ? 4000 : 800;
			}
			else {
				return num == BLACK_FLAG ? 3600 : 750;
			}
		}
	}
	else {
		if (k == 0) {
			return num == BLACK_FLAG ? 20000 : 15000;
		}
		else {
			return num == BLACK_FLAG ? 10000 : 3300;
		}
	}

}

void pcLoad() {
	benefit();

	int count = 0;
	int row = 0;
	int col = 0;
	int i = 0;
	int j = 0;
	for (i = 0; i < 15; i++) {
		for (j = 0; j < 15; j++) {
			if (situationPC[i][j] > count) {
				count = situationPC[i][j];
				row = i;
				col = j;
			}
			if (situationPlayer[i][j] > count) {
				count = situationPlayer[i][j];
				row = i;
				col = j;
			}
		}
	}

	printf("qinxingPC[%d][%d]:%d\n", row, col, situationPC[row][col]);
	printf("qinxingPlayer[%d][%d]:%d\n", row, col, situationPlayer[row][col]);

	if (map[row][col] == '+') {
		map[row][col] = BLACK_FLAG;
	}

}

//记分
void benefit() {
	int n = 0;
	int m = 0;
	for (n = 0; n < 15; n++) {
		for (m = 0; m < 15; m++) {
			situationPC[n][m] = horizontal(n, m, BLACK_FLAG) + vertical(n, m, BLACK_FLAG) + leftSlope(n, m, BLACK_FLAG) + rightSlope(n, m, BLACK_FLAG);
			situationPlayer[n][m] = horizontal(n, m, WHITE_FLAG) + vertical(n, m, WHITE_FLAG) + leftSlope(n, m, WHITE_FLAG) + rightSlope(n, m, WHITE_FLAG);
		}
	}

}

//玩家下子
int playerLoad(fd_stdin) {
	int x, y;
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

	while (x<0 || y<0 || x>HIGHT || y>WIDTH) {
		printf("Please input a valid coordinate!Input again:\n");
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
	}

	x--;
	y--;

	while (x < 0 || y < 0 || x >= HIGHT || y >= WIDTH || map[x][y] != '+')
	{
		printf("Please input a valid coordinate!Input again:\n");
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
		x--;
		y--;
	}

	if (player == WHITE) {
		map[x][y] = WHITE_FLAG;
	}
	return 1;
}

int win() {
	char m;
	int i, j;

	if (player == WHITE) m = WHITE_FLAG;
	else m = BLACK_FLAG;

	for (i = 0; i < HIGHT; i++) {
		for (j = 0; j < WIDTH; j++) {
			if (map[i][j] == m) {
				if ((i + 4) < HIGHT) {
					if (map[i + 1][j] == m && map[i + 2][j] == m && map[i + 3][j] == m && map[i + 4][j] == m) return 1;
				}
				if ((j + 4) < WIDTH) {
					if (map[i][j + 1] == m && map[i][j + 2] == m && map[i][j + 3] == m && map[i][j + 4] == m) return 1;
				}
				if ((i + 4) < HIGHT && (j + 4) < WIDTH) {
					if (map[i + 1][j + 1] == m && map[i + 2][j + 2] == m && map[i + 3][j + 3] == m && map[i + 4][j + 4] == m) return 1;
				}
				if ((i + 4) < HIGHT && (j - 4) >= 0) {
					if (map[i + 1][j - 1] == m && map[i + 2][j - 2] == m && map[i + 3][j - 3] == m && map[i + 4][j - 4] == m) return 1;
				}
			}
		}
	}
	return 0;
}