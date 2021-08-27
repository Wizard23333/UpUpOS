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

void runGame(fd_stdin, fd_stdout)
{
    clear();

    while (1)
    {
        printf("UpUpOS has the games below to play:\n");
        printf("1. colorball\n");
        printf("2. fivechess\n");
        printf("3. draughts\n");
        printf("5. Quit now\n");
        printf("Please choose one: [4] ");

        char rdbuf[128];
        int r = read(fd_stdin, rdbuf, 70);
        rdbuf[r] = 0;
        while (r < 1)
        {
            r = read(fd_stdin, rdbuf, 70);
            rdbuf[r] = 0;
        }

        if (strcmp(rdbuf, "5") == 0)
        {
            clear();
            return;
        }
        else if (strcmp(rdbuf, "1") == 0)
        {
            clear();
            colorBall(fd_stdin);
        }
        else if (strcmp(rdbuf, "2") == 0)
        {
            clear();
            fiveChess(fd_stdin);
        }
        else if (strcmp(rdbuf, "3") == 0)
        {
            clear();
            runDraghts(fd_stdin, fd_stdout);
        }
        else if (strcmp(rdbuf, "4") == 0)
        {
        }
        else
        {
            printf("Please input a valid number!\nPress ANY key to continue!");
            int r = read(fd_stdin, rdbuf, 70);
            clear();
            continue;
        }
    }
}

void runApp(fd_stdin, fd_stdout)
{
    clear();

    while (1)
    {
        printf("UpUpOS has the applications below, please choose one to run:\n");
        printf("1. processmanager\n");
        printf("2. file system\n");
        printf("3. calculator\n");
        printf("4. calendar\n");
        printf("6. Quit now");

        char rdbuf[128];
        int r = read(fd_stdin, rdbuf, 70);
        rdbuf[r] = 0;
        while (r < 1)
        {
            r = read(fd_stdin, rdbuf, 70);
            rdbuf[r] = 0;
        }

        if (strcmp(rdbuf, "6") == 0)
        {
            clear();
            return;
        }
        else if (strcmp(rdbuf, "1") == 0)
        {
            clear();
        }
        else if (strcmp(rdbuf, "2") == 0)
        {
            clear();
        }
        else if (strcmp(rdbuf, "3") == 0)
        {
            clear();
            manMain("math");

            printf("\nPress ANY key to continue!");
            int r = read(fd_stdin, rdbuf, 70);
            return;
        }
        else if (strcmp(rdbuf, "4") == 0)
        {
        }
        else
        {
            printf("Please input a valid number!\nPress ANY key to continue!");
            int r = read(fd_stdin, rdbuf, 70);
            clear();
            continue;
        }
    }
}

void systemUsage(fd_stdin, fd_stdout)
{
    printf("This operating system has 2 system level applications:\n1. process manager\n2. file manager\n");
    printf("7 user-level applications which includes 3 applications and 4 games:\n");
    printf("applications: \n1. calculator\n2.calendar");
    printf("games: \n1. colorball\n2. fivechess\n3.draughts\n4. \n");
    printf("To run these applications, you can input the command straight away, \nor if you are not familiar with the commands, try using the guide！\n");
    
    printf("\nPress ANY key to continue!");
    char rdbuf[128];
    int r = read(fd_stdin, rdbuf, 70);
    clear();
}

void runSystemGuide(fd_stdin, fd_stdout)
{
    while (1)
    {
        printf("Welcome to UpUpOS's system guide! Here you can choose what you want to do or look up.\n\n");
        printf("1. Usage for system partterns\n");
        printf("2. Choose a game to play\n");
        printf("3. Run an application\n");
        printf("4. UpUpOS's information\n");
        printf("5. Quit now\n\n");
        printf("Please choose one: [4] ");

        char rdbuf[128];
        int r = read(fd_stdin, rdbuf, 70);
        rdbuf[r] = 0;
        while (r < 1)
        {
            r = read(fd_stdin, rdbuf, 70);
            rdbuf[r] = 0;
        }

        if (strcmp(rdbuf, "5") == 0)
        {
            clear();
            return;
        }
        else if (strcmp(rdbuf, "1") == 0)
        {
            clear();
            systemUsage(fd_stdin, fd_stdout);
        }
        else if (strcmp(rdbuf, "2") == 0)
        {
            clear();
            runGame(fd_stdin, fd_stdout);
        }
        else if (strcmp(rdbuf, "3") == 0)
        {
            clear();
            runApp(fd_stdin, fd_stdout);
            return;
        }
        else if (strcmp(rdbuf, "4") == 0)
        {
        }
        else
        {
            printf("Please input a valid number!\nPress ANY key to continue!");
            int r = read(fd_stdin, rdbuf, 70);
            clear();
            continue;
        }
    }
}
