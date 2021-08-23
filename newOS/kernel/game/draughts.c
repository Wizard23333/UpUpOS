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

#define false 0
#define true 1
typedef int bool;

#define BOARD_SIZE 8
#define EMPTY 0
#define MY_FLAG 2
#define MY_KING 4
#define ENEMY_FLAG 1
#define ENEMY_KING 3
#define MAX_STEP 15
#define MAX_MEMORY 20
#define START "START"
#define PLACE "PLACE"
#define TURN "TURN"
#define END "END"
#define DEPTH 6
typedef struct Commands
{
    int x[MAX_STEP];
    int y[MAX_STEP];
    int numStep;
} Command;
typedef struct statistic
{
    int x[MAX_MEMORY];
    int y[MAX_MEMORY];
    int flag[MAX_MEMORY];
    int lenth;
    int num1; //我方棋子数增减
    int num2; //敌方
} mem;
char board[BOARD_SIZE][BOARD_SIZE] = {0};
int myFlag = 0;
int moveDir[4][2] = {{1, -1}, {1, 1}, {-1, -1}, {-1, 1}};
int jumpDir[4][2] = {{2, -2}, {2, 2}, {-2, -2}, {-2, 2}};
int numMyFlag;
int numEneFlag;
Command moveCmd = {.x = {0}, .y = {0}, .numStep = 2};
Command jumpCmd = {.x = {0}, .y = {0}, .numStep = 0};
Command longestJumpCmd = {.x = {0}, .y = {0}, .numStep = 1};
Command longestEneCmd = {.x = {0}, .y = {0}, .numStep = 1};
Command longestMyCmd = {.x = {0}, .y = {0}, .numStep = 1};
Command eneMoveCmd = {.x = {0}, .y = {0}, .numStep = 2};
Command myMoveCmd = {.x = {0}, .y = {0}, .numStep = 2};

void turn(void);
void end(int x);
void loop(void);
void initAI(int me);
void start(int flag);
void printBoard(void);
void place(Command cmd);
void rotateCommand(Command *cmd);
void replace(const Command *cmd, mem *p);
void remember(const Command *cmd, mem *p);
bool isInBound(int x, int y);
bool tryToJump(int x, int y, int currentStep);
bool eneTryToJump(int x, int y, int currentStep);
int valueCase(void);
int tryToMove(int x, int y);
int myTryToMove(int x, int y);
int eneTryToMove(int x, int y);
int myTry(int depth, const Command *cmd);
int enemyTry(int depth, const Command *cmd);
Command aiTurn(const char board[BOARD_SIZE][BOARD_SIZE], int me);

void turn()
{
    Command command = aiTurn((const char(*)[BOARD_SIZE])board, myFlag);
    place(command);
    rotateCommand(&command); //旋转指令
    printf("AI STEP:%d", command.numStep);
    for (int i = 0; i < command.numStep; i++)
    {
        printf(" %d,%d", command.x[i], command.y[i]);
    }
    printf("\n\n");
}

void runDraghts(fd_stdin, fd_stdout)
{

    myFlag = 0;
    Command command =
        {
            .x = {0},
            .y = {0},
            .numStep = 0};
    int status;

    printf("Draughts Is Now Start!\n");
    // printf("myflag%d\n", myFlag);

    while (myFlag == 0)
    {
        char rdbuf[128]; //读取信息
        if (myFlag == 0)
        {
            printf("Do You Want To Start First[Y/n]?");
            int r = read(fd_stdin, rdbuf, 70);
            rdbuf[r] = 0;
            while (r < 1)
            {
                r = read(fd_stdin, rdbuf, 70);
                rdbuf[r] = 0;
            }
        }
        if (strcmp(rdbuf, "quit") == 0 || strcmp(rdbuf, "q") == 0 || strcmp(rdbuf, "Q") == 0)
        {
            return;
        }
        else if (strcmp(rdbuf, "Y") == 0 || strcmp(rdbuf, "y") == 0)
        {
            myFlag = 2;
            printf("your chess symbol is \" O \"\n");
            start(myFlag);
        }
        else if (strcmp(rdbuf, "N") == 0 || strcmp(rdbuf, "n") == 0)
        {
            myFlag = 1;
            printf("your chess symbol is \" O \"\n");
            printf("Please Wait for the AI to Think...\n");
            start(myFlag);
            turn();
        }
    }
    // printf("myflag%d\n", myFlag);
    
    printBoard();

    while (true)
    {

        printf("Please Enter Your Step like \"place 2 2,1 3,0 \":\n");
        char rdbuf2[128];
        int r2 = read(fd_stdin, rdbuf2, 70);
        rdbuf2[r2] = 0;
        while (r2 < 1)
        {
            r2 = read(fd_stdin, rdbuf2, 70);
            rdbuf2[r2] = 0;
        }

        char command1[10], command2[40];
        memset(command1, 0, sizeof(command1));
        memset(command2, 0, sizeof(command2));

        mystrncpy(command1, rdbuf2, 5);
        // printf("command1:%s\n", command1);

        if (strcmp(rdbuf2, "quit") == 0 || strcmp(rdbuf2, "q") == 0 || strcmp(rdbuf2, "Q") == 0)
        {
            return;
        }
        else if (strcmp(command1, "place") == 0)
        {
            char temp[3];
            temp[0] = rdbuf2[6];
            // printf("rdbuf2[6]:%c\n", rdbuf2[6]);
            // printf("temp:%s\n", temp);
            atoi(temp, &command.numStep);
            // printf("command.numStep: %d\n", command.numStep);
            for (int i = 0; i < command.numStep; i++)
            {
                temp[0] = rdbuf2[i * 4 + 8];
                atoi(temp, &command.x[i]);
                temp[0] = rdbuf2[i * 4 + 10];
                atoi(temp, &command.y[i]);
                // printf("x: %d y: %d", command.x[i], command.y[i]);
            }
            rotateCommand(&command); //根据情况旋转place指令
            place(command);          //执行PLACE指令
            printBoard();
            printf("Please Wait for the AI to Think...\n");
            turn();
        }
        else
        {
            printf("No this command! Please input again.\n");
            continue;
        }

        printBoard(); //在start、turn、place指令后输出当前棋盘
    }
}

void initAI(int me)
{
    numMyFlag = 12;
    numEneFlag = 12;
}

void start(int flag) //初始化棋盘
{
    memset(board, 0, sizeof(board));
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 8; j += 2)
        {
            board[i][j + (i + 1) % 2] = MY_FLAG;
        }
    }
    for (int i = 5; i < 8; i++)
    {
        for (int j = 0; j < 8; j += 2)
        {
            board[i][j + (i + 1) % 2] = ENEMY_FLAG;
        }
    }

    initAI(flag);
}

void printBoard()
{
    char visualBoard[BOARD_SIZE][BOARD_SIZE + 1] = {0};
    if (myFlag == ENEMY_FLAG)
    {
        for (int i = 0; i < BOARD_SIZE; i++)
        {
            for (int j = 0; j < BOARD_SIZE; j++)
            {
                switch (board[i][j])
                {
                case EMPTY:
                    visualBoard[i][j] = '.';
                    break;
                case ENEMY_FLAG:
                    visualBoard[i][j] = 'O';
                    break;
                case MY_FLAG:
                    visualBoard[i][j] = 'X';
                    break;
                case ENEMY_KING:
                    visualBoard[i][j] = '@';
                    break;
                case MY_KING:
                    visualBoard[i][j] = '*';
                    break;
                default:
                    break;
                }
            }
            printf("%d--%s\n", BOARD_SIZE - i - 1, visualBoard[i]);
        }
        printf("   ");
        for (int i = 0; i < BOARD_SIZE; i++)
            printf("|");
        printf("\n   ");
        for (int i = BOARD_SIZE - 1; i >= 0; i--)
            printf("%d", i);
        printf("\n");
    }
    else if (myFlag == MY_FLAG)
    {
        for (int i = BOARD_SIZE - 1; i >= 0; i--)
        {
            for (int j = BOARD_SIZE - 1; j >= 0; j--)
            {
                switch (board[i][j])
                {
                case EMPTY:
                    visualBoard[i][j] = '.';
                    break;
                case ENEMY_FLAG:
                    visualBoard[i][j] = 'O';
                    break;
                case MY_FLAG:
                    visualBoard[i][j] = 'X';
                    break;
                case ENEMY_KING:
                    visualBoard[i][j] = '@';
                    break;
                case MY_KING:
                    visualBoard[i][j] = '*';
                    break;
                default:
                    break;
                }
            }
            for (int k = 0; k < BOARD_SIZE / 2; k++)
            {
                int temp = visualBoard[i][k];
                visualBoard[i][k] = visualBoard[i][BOARD_SIZE - 1 - k];
                visualBoard[i][BOARD_SIZE - 1 - k] = temp;
            }
            printf("%d--%s\n", i, visualBoard[i]);
        }
        printf("   ");
        for (int i = 0; i < BOARD_SIZE; i++)
            printf("|");
        printf("\n   ");
        for (int i = BOARD_SIZE - 1; i >= 0; i--)
            printf("%d", i);
        printf("\n");
    }
}

void place(Command cmd)
{
    int midX, midY, curFlag;
    curFlag = board[cmd.x[0]][cmd.y[0]]; //确定当前棋子的位置
    for (int i = 0; i < cmd.numStep - 1; i++)
    {
        board[cmd.x[i]][cmd.y[i]] = EMPTY;
        board[cmd.x[i + 1]][cmd.y[i + 1]] = curFlag;
        int abstemp = cmd.x[i] - cmd.x[i + 1] > 0 ? cmd.x[i] - cmd.x[i + 1] : cmd.x[i + 1] - cmd.x[i];
        if (abstemp == 2) //如果绝对值为2，即对方的棋子在跳棋
        {
            midX = (cmd.x[i] + cmd.x[i + 1]) / 2;
            midY = (cmd.y[i] + cmd.y[i + 1]) / 2; //确定被吃掉的棋子的坐标
            if ((board[midX][midY] & 1) == 0)     //如果被吃掉棋子不是敌方棋子我的棋子数减一
            {
                numMyFlag--;
            }
            else if (board[midX][midY] == 1 || board[midX][midY] == 3)
            {
                numEneFlag--;
            }
            board[midX][midY] = EMPTY;
        }
    }
    for (int i = 0; i < BOARD_SIZE; i++) //确定王棋
    {
        if (board[0][i] == ENEMY_FLAG) //敌方王棋
        {
            board[0][i] = ENEMY_KING;
        }
        if (board[BOARD_SIZE - 1][i] == MY_FLAG) //我方王棋
        {
            board[BOARD_SIZE - 1][i] = MY_KING;
        }
    }
}

void rotateCommand(Command *cmd)
{
    if (myFlag == ENEMY_FLAG)
    {
        for (int i = 0; i < cmd->numStep; i++)
        {
            cmd->x[i] = BOARD_SIZE - 1 - cmd->x[i];
            cmd->y[i] = BOARD_SIZE - 1 - cmd->y[i];
        }
    }
}

void replace(const Command *cmd, mem *p)
{
    for (int i = 0; i < p->lenth; i++)
    {
        board[p->x[i]][p->y[i]] = p->flag[i];
    }
    numMyFlag += p->num1;
    numEneFlag += p->num2;
}

void remember(const Command *cmd, mem *p)
{
    p->num2 = p->num1 = p->lenth = 0;
    for (int i = 0; i < cmd->numStep; i++)
    {
        p->x[p->lenth] = cmd->x[i];
        p->y[p->lenth] = cmd->y[i];
        p->flag[p->lenth++] = board[cmd->x[i]][cmd->y[i]];
        int abstemp = cmd->x[i] - cmd->x[i - 1] > 0 ? cmd->x[i] - cmd->x[i - 1] : cmd->x[i - 1] - cmd->x[i];
        if (i != 0 && abstemp == 2)
        {
            p->x[p->lenth] = (cmd->x[i] + cmd->x[i - 1]) / 2;
            p->y[p->lenth] = (cmd->y[i] + cmd->y[i - 1]) / 2;
            p->flag[p->lenth] = board[p->x[p->lenth]][p->y[p->lenth]];
            if (p->flag[p->lenth] == 2 || p->flag[p->lenth] == 4)
            {
                p->num1++;
            }
            else if (p->flag[p->lenth] == 1 || p->flag[p->lenth] == 3)
            {
                p->num2++;
            }
            p->lenth++;
        }
    }
}

bool isInBound(int x, int y)
{
    return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

bool tryToJump(int x, int y, int currentStep) //跳跃尝试
{
    bool jumpNot = false;
    char tmpFlag; //中间被吃敌方棋子
    int newX, newY, midX, midY;
    jumpCmd.x[currentStep] = x;
    jumpCmd.y[currentStep] = y;
    jumpCmd.numStep++;          //坐标步数同步
    for (int i = 0; i < 4; i++) //四个方向
    {
        newX = x + jumpDir[i][0];
        newY = y + jumpDir[i][1];
        midX = (x + newX) / 2;
        midY = (y + newY) / 2;                                                                //确定中点坐标
        if (isInBound(newX, newY) && (board[midX][midY] & 1) && (board[newX][newY] == EMPTY)) //判断跳跃条件
        {
            board[newX][newY] = board[x][y];
            board[x][y] = EMPTY;
            tmpFlag = board[midX][midY];
            board[midX][midY] = EMPTY;
            tryToJump(newX, newY, currentStep + 1); //递归并取走被吃的子
            board[x][y] = board[newX][newY];
            board[newX][newY] = EMPTY;
            board[midX][midY] = tmpFlag; //还原棋盘//确保每一步试走时不缺棋子
            jumpNot = true;
        }
    }
    if (jumpCmd.numStep >= longestJumpCmd.numStep) //如果跳的步数大于当前最大步数
    {
        longestJumpCmd = jumpCmd;
    }
    jumpCmd.numStep--; //全局变量回溯到跳前
    return jumpNot;
}

bool eneTryToJump(int x, int y, int currentStep) //跳跃尝试
{
    char tmpFlag; //中间被吃敌方棋子
    bool jumpNot = false;
    int newX, newY, midX, midY;
    jumpCmd.x[currentStep] = x;
    jumpCmd.y[currentStep] = y;
    jumpCmd.numStep++;          //坐标步数同步
    for (int i = 0; i < 4; i++) //四个方向
    {
        newX = x + jumpDir[i][0];
        newY = y + jumpDir[i][1];
        midX = (x + newX) / 2;
        midY = (y + newY) / 2;                                                                                           //确定中点坐标
        if (isInBound(newX, newY) && (board[midX][midY] == 2 || board[midX][midY] == 4) && (board[newX][newY] == EMPTY)) //判断跳跃条件
        {
            board[newX][newY] = board[x][y];
            board[x][y] = EMPTY;
            tmpFlag = board[midX][midY];
            board[midX][midY] = EMPTY;
            eneTryToJump(newX, newY, currentStep + 1); //递归并取走被吃的子
            board[x][y] = board[newX][newY];
            board[newX][newY] = EMPTY;
            board[midX][midY] = tmpFlag; //还原棋盘//确保每一步试走时不缺棋子
            jumpNot = true;
        }
    }
    if (jumpCmd.numStep >= longestEneCmd.numStep) //如果跳的步数大于当前最大步数
    {
        longestEneCmd = jumpCmd;
    }
    jumpCmd.numStep--; //全局变量回溯到跳前
    return jumpNot;
}

int valueCase()
{
    int valueOfCase = 0;
    int k1 = -10, k2 = 10, k3 = 30, k6 = 1, k7 = -20;
    int numMyKing = 0, numEneKing = 0;
    for (int i1 = 0; i1 <= BOARD_SIZE - 1; i1++) //前进
    {
        for (int i2 = 0; i2 <= BOARD_SIZE - 1; i2++)
        {
            if (board[i1][i2] == MY_FLAG)
            {
                valueOfCase += k6 * i1;
            }
            else if (board[i1][i2] == MY_KING)
            {
                numMyKing++;
            }
            else if (board[i1][i2] == ENEMY_KING)
            {
                numEneKing++;
            }
        }
    }
    valueOfCase += numEneKing * k7 + numMyKing * k3 + numMyFlag * k2 + numEneFlag * k1;
    return valueOfCase;
}

int tryToMove(int x, int y) //己方move
{
    int newX, newY;
    for (int i = 0; i < board[x][y]; i++)
    {
        newX = x + moveDir[i][0];
        newY = y + moveDir[i][1];
        if (isInBound(newX, newY) && board[newX][newY] == EMPTY)
        {
            moveCmd.x[0] = x;
            moveCmd.y[0] = y;
            moveCmd.x[1] = newX;
            moveCmd.y[1] = newY;
            return i;
        }
    }
    return -1;
}

int myTryToMove(int x, int y) //己方move
{
    int newX, newY;
    for (int i = 0; i < board[x][y]; i++)
    {
        newX = x + moveDir[i][0];
        newY = y + moveDir[i][1];
        if (isInBound(newX, newY) && board[newX][newY] == EMPTY)
        {
            myMoveCmd.x[0] = x;
            myMoveCmd.y[0] = y;
            myMoveCmd.x[1] = newX;
            myMoveCmd.y[1] = newY;
            return i;
        }
    }
    return -1;
}

int eneTryToMove(int x, int y)
{
    int newX, newY;
    for (int i = 0; i < (board[x][y] == 1 ? 2 : 4); i++)
    {
        newX = x + moveDir[3 - i][0];
        newY = y + moveDir[3 - i][1];
        if (isInBound(newX, newY) && board[newX][newY] == EMPTY)
        {
            eneMoveCmd.x[0] = x;
            eneMoveCmd.y[0] = y;
            eneMoveCmd.x[1] = newX;
            eneMoveCmd.y[1] = newY;
            return i;
        }
    }
    return -1;
}

int enemyTry(int depth, const Command *cmd)
{
    int value = 0;
    mem memory;
    remember(cmd, &memory);
    place(*cmd);
    if (depth >= 1)
    {
        int minValue = 1000;
        int numChecked = 0;
        int maxStep = 1;
        bool jumpOrNot = false;
        Command command = {.x = {0}, .y = {0}, .numStep = 0};
        for (int i = 0; i < BOARD_SIZE; i++)
        {
            for (int j = 0; j < BOARD_SIZE; j++)
            {
                if (board[i][j] == 1 || board[i][j] == 3)
                {
                    numChecked++;
                    longestEneCmd.numStep = 1;
                    bool jumpNot = eneTryToJump(i, j, 0);
                    if (longestEneCmd.numStep >= maxStep && jumpNot)
                    {
                        int tempValue = 0;
                        jumpOrNot = true;
                        Command temp = longestEneCmd;
                        tempValue = myTry(depth - 1, &longestEneCmd);
                        if (tempValue < minValue || temp.numStep > maxStep)
                        {
                            minValue = tempValue;
                            maxStep = temp.numStep;
                            command = temp;
                        }
                    }
                    if (jumpOrNot == false && eneTryToMove(i, j) >= 0)
                    {
                        int tempValue = 0;
                        Command temp = eneMoveCmd;
                        tempValue = myTry(depth - 1, &eneMoveCmd);
                        if (tempValue < minValue)
                        {
                            minValue = tempValue;
                            command = temp;
                        }
                    }
                }
                if (numChecked >= numEneFlag)
                {
                    value = minValue;
                    replace(cmd, &memory);
                    return value;
                }
            }
        }
    }
    else
    {
        value = valueCase();
        replace(cmd, &memory);
        return value;
    }
    return -10000;
}

int myTry(int depth, const Command *cmd)
{
    int value = 0;
    mem memory;
    remember(cmd, &memory);
    place(*cmd);
    if (depth >= 1)
    {
        Command command = {.x = {0}, .y = {0}, .numStep = 0};
        int maxValue = -1000;
        int numChecked = 0;
        int maxStep = 1;
        bool jumpOrNot = false;
        for (int i = 0; i < BOARD_SIZE; i++)
        {
            for (int j = 0; j < BOARD_SIZE; j++)
            {
                if (board[i][j] > 0 && (board[i][j] & 1) == 0)
                {
                    numChecked++;
                    longestJumpCmd.numStep = 1;
                    bool jumpNot = tryToJump(i, j, 0);
                    if (longestJumpCmd.numStep >= maxStep && jumpNot)
                    {
                        jumpOrNot = true;
                        int tempValue = 0;
                        Command temp = longestJumpCmd;
                        tempValue = enemyTry(depth - 1, &longestJumpCmd);
                        if (tempValue > maxValue || temp.numStep > maxStep)
                        {
                            maxValue = tempValue;
                            maxStep = temp.numStep;
                            command = temp;
                        }
                    }
                    if (jumpOrNot == false && myTryToMove(i, j) >= 0)
                    {
                        int tempValue = 0;
                        Command temp = myMoveCmd;
                        tempValue = enemyTry(depth - 1, &myMoveCmd);
                        if (tempValue > maxValue)
                        {
                            maxValue = tempValue;
                            command = temp;
                        }
                    }
                }
                if (numChecked >= numMyFlag)
                {
                    value = maxValue;
                    replace(cmd, &memory);
                    return value;
                }
            }
        }
    }
    else
    {
        value = valueCase();
        replace(cmd, &memory);
        return value;
    }
    return -10000;
}

Command aiTurn(const char board[BOARD_SIZE][BOARD_SIZE], int me)
{
    int maxValue = -10000;
    int numChecked = 0;
    int maxStep = 1;
    bool jumpOrNot = false;
    Command command = {.x = {0}, .y = {0}, .numStep = 0};
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] > 0 && (board[i][j] & 1) == 0) //如果是己方棋子
            {
                numChecked++;
                longestJumpCmd.numStep = 1;
                bool jumpNot = tryToJump(i, j, 0);
                if (longestJumpCmd.numStep >= maxStep && jumpNot) //把最长跳方法储存到command//判断在该点能不能跳
                {
                    jumpOrNot = true;
                    int tempValue = 0;
                    Command temp = longestJumpCmd;
                    tempValue = enemyTry(DEPTH, &longestJumpCmd);
                    if (tempValue > maxValue || temp.numStep > maxStep)
                    {
                        maxValue = tempValue;
                        maxStep = temp.numStep;
                        command = temp;
                    }
                }
                if (jumpOrNot == false && tryToMove(i, j) >= 0) //如果没跳成
                {
                    int tempValue = 0;
                    Command temp = moveCmd;
                    tempValue = enemyTry(DEPTH, &moveCmd);
                    if (tempValue > maxValue)
                    {
                        maxValue = tempValue;
                        command = temp;
                    }
                }
            }
            if (numChecked >= numMyFlag) //如果我的棋子数完返回
            {
                return command;
            }
        }
    }
    return command;
}
