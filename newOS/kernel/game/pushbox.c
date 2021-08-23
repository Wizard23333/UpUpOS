
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

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						 push box
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

int pi = 0;
int pj = 0;
void draw_map(int map[9][11])
{
    int i;
    int j;
    for (i = 0; i < 9; i++)
    {
        for (int j = 0; j < 11; j++)
        {
            switch (map[i][j])
            {
            case 0:
                printf(" "); //道路
                break;
            case 1:
                printf("#"); //墙壁
                break;
            case 2:
                printf(" "); //游戏边框的空白部分
                break;
            case 3:
                printf("D"); //目的地
                break;
            case 4:
                printf("b"); //箱子
                break;
            case 7:
                printf("!"); //箱子进入目的地
                break;
            case 6:
                printf("p"); //人
                break;
            case 9:
                printf("^"); //人进入目的地
                break;
            }
        }
        printf("\n");
    }
}
//void draw_map(int map[9][11]);
//void boxMenu();
void boxMenu()
{
    printf("      ====================================================================\n");
    printf("      #         *******                         Welcome to               #\n");
    printf("      #            **                            cherryOS                #\n");
    printf("      #        ***  **                                                   #\n");
    printf("      #     ******   **                                                  #\n");
    printf("      #   ******      **                        Instruction              #\n");
    printf("      #   ****      **  **                set: p:People b:BOX            #\n");
    printf("      #            **    **                    #:Wall   D:Destination    #\n");
    printf("      #        ******     ****            operation:                     #\n");
    printf("      #      **********  ******                s:Down d:Right            #\n");
    printf("      #      **********   ****                 w:Up   a:Left  q:Quit     #\n");
    printf("      #        ******                                                    #\n");
    printf("      #                                   PS:you can enter'q' to quit    #\n");
    printf("      #                                      anytime you like.           #\n");
    printf("      #                                                                  #\n");
    printf("      #               Powered by doubleZ, budi, flyingfish               #\n");
    printf("      #                       ALL RIGHT REVERSED                         #\n");
    printf("      ====================================================================\n");
}
void Runpushbox(fd_stdin, fd_stdout)
{
    char rdbuf[128];
    int r;
    char control;

    int count = 0; //定义记分变量

    int map[9][11] = {
        {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
        {2, 1, 0, 0, 0, 1, 0, 0, 0, 1, 2},
        {2, 1, 0, 4, 4, 4, 4, 4, 0, 1, 2},
        {2, 1, 0, 4, 0, 4, 0, 4, 0, 0, 1},
        {2, 1, 0, 0, 0, 6, 0, 0, 4, 0, 1},
        {1, 1, 0, 1, 1, 1, 1, 0, 4, 0, 1},
        {1, 0, 3, 3, 3, 3, 3, 1, 0, 0, 1},
        {1, 0, 3, 3, 3, 3, 3, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    };
    while (1)
    {
        clear();
        printf("\n");
        boxMenu();//绘制菜单
        draw_map(map);//绘制整体地图
        printf("Current Score:%d\n", count);
        //找初始位置
        for (pi = 0; pi < 9; pi++)
        {
            for (pj = 0; pj < 11; pj++)
            {
                if (map[pi][pj] == 6 || map[pi][pj] == 9)
                    break;
            }
            if (map[pi][pj] == 6 || map[pi][pj] == 9)
                break;
        }
        printf("CURRENT LOCATION (%d,%d)", pi, pj);

        printf("\n");
        printf("Please input direction:");

        r = read(fd_stdin, rdbuf, 70);
        rdbuf[r] = 0;
        control = rdbuf[0];

        if (control == 'Q' || control == 'q')
        {
            break;
        }
        switch (control)
        {
        case 'w':
            //如果人前面是空地。
            if (map[pi - 1][pj] == 0)
            {
                map[pi - 1][pj] = 6 + 0;
                if (map[pi][pj] == 9)
                    map[pi][pj] = 3;
                else
                    map[pi][pj] = 0;
            }
            //如果人前面是目的地。
            else if ((map[pi - 1][pj] == 3) || (map[pi - 1][pj] == 9))
            {
                map[pi - 1][pj] = 6 + 3;
                if (map[pi][pj] == 9)
                    map[pi][pj] = 3;
                else
                    map[pi][pj] = 0;
            }
            //如果人前面是箱子。
            else if (map[pi - 1][pj] == 4)
            {
                if (map[pi - 2][pj] == 0)
                {
                    map[pi - 2][pj] = 4;
                    if (map[pi - 1][pj] == 7)
                        map[pi - 1][pj] = 9;
                    else
                        map[pi - 1][pj] = 6;
                    if (map[pi][pj] == 9)
                        map[pi][pj] = 3;
                    else
                        map[pi][pj] = 0;
                }
                //如果人的前面是箱子，而箱子前面是目的地。
                else if (map[pi - 2][pj] == 3)
                {
                    map[pi - 2][pj] = 7;
                    count++;
                    if (map[pi - 1][pj] == 7)
                        map[pi - 1][pj] = 9;
                    else
                        map[pi - 1][pj] = 6;
                    if (map[pi][pj] == 9)
                        map[pi][pj] = 3;
                    else
                        map[pi][pj] = 0;
                }
            }
            //如果人前面是已经进入某目的地的箱子
            else if (map[pi - 1][pj] == 7)
            {
                //如果人前面是已经进入某目的地的箱子,箱子前面是空地。
                if (map[pi - 2][pj] == 0)
                {
                    count--;
                    map[pi - 2][pj] = 4;
                    map[pi - 1][pj] = 9;
                    if (map[pi][pj] == 9)
                        map[pi][pj] = 3;
                    else
                        map[pi][pj] = 0;
                }
                //如果人前面是已经进入某目的地的箱子，箱子前面是另一目的地。
                if (map[pi - 2][pj] == 3)
                {
                    map[pi - 2][pj] = 7;
                    map[pi - 1][pj] = 9;
                    if (map[pi][pj] == 9)
                        map[pi][pj] = 3;
                    else
                        map[pi][pj] = 0;
                }
            }
            break;
        case 's':
            //如果人前面是空地。
            if (map[pi + 1][pj] == 0)
            {
                map[pi + 1][pj] = 6 + 0;
                if (map[pi][pj] == 9)
                    map[pi][pj] = 3;
                else
                    map[pi][pj] = 0;
            }
            //如果人前面是目的地。
            else if (map[pi + 1][pj] == 3)
            {
                map[pi + 1][pj] = 6 + 3;
                if (map[pi][pj] == 9)
                    map[pi][pj] = 3;
                else
                    map[pi][pj] = 0;
            }
            //如果人前面是箱子。
            else if (map[pi + 1][pj] == 4)
            {
                //如果人前面是箱子，而箱子前面是空地。
                if (map[pi + 2][pj] == 0)
                {
                    map[pi + 2][pj] = 4;
                    if (map[pi + 1][pj] == 7)
                        map[pi + 1][pj] = 9;
                    else
                        map[pi + 1][pj] = 6;
                    if (map[pi][pj] == 9)
                        map[pi][pj] = 3;
                    else
                        map[pi][pj] = 0;
                }
                //如果人的前面是箱子，而箱子前面是目的地。
                else if (map[pi + 2][pj] == 3)
                {
                    map[pi + 2][pj] = 7;
                    count++;
                    if (map[pi + 1][pj] == 7)
                        map[pi + 1][pj] = 9;
                    else
                        map[pi + 1][pj] = 6;
                    if (map[pi][pj] == 9)
                        map[pi][pj] = 3;
                    else
                        map[pi][pj] = 0;
                }
            }
            else if (map[pi + 1][pj] == 7)
            {
                if (map[pi + 2][pj] == 0)
                {
                    count--;
                    map[pi + 2][pj] = 4;
                    map[pi + 1][pj] = 9;
                    if (map[pi][pj] == 9)
                        map[pi][pj] = 3;
                    else
                        map[pi][pj] = 0;
                }
                if (map[pi + 2][pj] == 3)
                {
                    map[pi + 2][pj] = 7;
                    map[pi + 1][pj] = 9;
                    if (map[pi][pj] == 9)
                        map[pi][pj] = 3;
                    else
                        map[pi][pj] = 0;
                }
            }
            break;
        case 'a':
            if (map[pi][pj - 1] == 0)
            {
                map[pi][pj - 1] = 6 + 0;
                if (map[pi][pj] == 9)
                    map[pi][pj] = 3;
                else
                    map[pi][pj] = 0;
            }
            else if (map[pi][pj - 1] == 3)
            {
                map[pi][pj - 1] = 6 + 3;
                if (map[pi][pj] == 9)
                    map[pi][pj] = 3;
                else
                    map[pi][pj] = 0;
            }
            else if (map[pi][pj - 1] == 4)
            {
                if (map[pi][pj - 2] == 0)
                {
                    map[pi][pj - 2] = 4;
                    if (map[pi][pj - 1] == 7)
                        map[pi][pj - 1] = 9;
                    else
                        map[pi][pj - 1] = 6;
                    if (map[pi][pj] == 9)
                        map[pi][pj] = 3;
                    else
                        map[pi][pj] = 0;
                }
                else if (map[pi][pj - 2] == 3)
                {
                    count++;
                    map[pi][pj - 2] = 7;
                    if (map[pi][pj - 1] == 7)
                        map[pi][pj - 1] = 9;
                    else
                        map[pi][pj - 1] = 6;
                    if (map[pi][pj] == 9)
                        map[pi][pj] = 3;
                    else
                        map[pi][pj] = 0;
                }
            }
            else if (map[pi][pj - 1] == 7)
            {
                if (map[pi][pj - 2] == 0)
                {
                    count--;
                    map[pi][pj - 2] = 4;
                    map[pi][pj - 1] = 9;
                    if (map[pi][pj] == 9)
                        map[pi][pj] = 3;
                    else
                        map[pi][pj] = 0;
                }
                if (map[pi][pj - 2] == 3)
                {
                    map[pi][pj - 2] = 7;
                    map[pi][pj - 1] = 9;
                    if (map[pi][pj] == 9)
                        map[pi][pj] = 3;
                    else
                        map[pi][pj] = 0;
                }
            }
            break;
        case 'd':
            if (map[pi][pj + 1] == 0)
            {
                map[pi][pj + 1] = 6 + 0;
                if (map[pi][pj] == 9)
                    map[pi][pj] = 3;
                else
                    map[pi][pj] = 0;
            }
            else if (map[pi][pj + 1] == 3)
            {
                map[pi][pj + 1] = 6 + 3;
                if (map[pi][pj] == 9)
                    map[pi][pj] = 3;
                else
                    map[pi][pj] = 0;
            }
            else if (map[pi][pj + 1] == 4)
            {
                if (map[pi][pj + 2] == 0)
                {
                    map[pi][pj + 2] = 4;
                    if (map[pi][pj + 1] == 7)
                        map[pi][pj + 1] = 9;
                    else
                        map[pi][pj + 1] = 6;
                    if (map[pi][pj] == 9)
                        map[pi][pj] = 3;
                    else
                        map[pi][pj] = 0;
                }
                else if (map[pi][pj + 2] == 3)
                {
                    count++;
                    map[pi][pj + 2] = 7;
                    if (map[pi][pj + 1] == 7)
                        map[pi][pj + 1] = 9;
                    else
                        map[pi][pj + 1] = 6;
                    if (map[pi][pj] == 9)
                        map[pi][pj] = 3;
                    else
                        map[pi][pj] = 0;
                }
            }
            else if (map[pi][pj + 1] == 7)
            {
                if (map[pi][pj + 2] == 0)
                {
                    count--;
                    map[pi][pj + 2] = 4;
                    map[pi][pj + 1] = 9;
                    if (map[pi][pj] == 9)
                        map[pi][pj] = 3;
                    else
                        map[pi][pj] = 0;
                }
                if (map[pi][pj + 2] == 3)
                {
                    map[pi][pj + 2] = 7;
                    map[pi][pj + 1] = 9;
                    if (map[pi][pj] == 9)
                        map[pi][pj] = 3;
                    else
                        map[pi][pj] = 0;
                }
            }
            break;
        }
        if (count == 8)
        {
            draw_map(map);
            printf("\nCongratulations!!\n");
            break; //退出死循环
        }
    }
}
