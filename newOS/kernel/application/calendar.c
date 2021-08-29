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

int leapYear(int year)//闰年判断
{
	if ((year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0)))
		return 1;
	else
		return 0;
}
int monthDay(int year, int month)//月份日期数
{
	int Day[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
	if (leapYear(year) == 1)
		Day[1] = 29;
	return Day[month - 1];
}
int countDay(int year, int month, int day)//日期是一年的第几天
{
	int sum = 0;
	int i = 1;
	for (i = 1; i < month; i++)
		sum = sum + monthDay(year, i);
	sum = sum + day;
	return sum;
}
int Weekday(int year, int month, int day)//返回星期信息0-6
{
    int count;
    count = (year - 1) + (year - 1) / 4 - (year - 1) / 100 + (year - 1) / 400 + countDay(year, month, day);//日期是一年的第几天
    count = count % 7;
    return count;
}
int festival(int month, int day)//节日判断
{
    switch (month)
    {
    case 1:switch (day)
    {
    case 1:printf("New Year's Day"); break;
    default:return 0;
    }break;
    case 2:switch (day)
    {
    case 14:printf("Valentines Day"); break;
    default:return 0;
    }break;
    case 3:switch (day)
    {
    case 8:printf("Women's Day"); break;
    case 12:printf("Arbor Day"); break;
    default:return 0;
    }break;
    case 4:switch (day)
    {
    case 1:printf("April Fools Day"); break;
    case 5:printf("Tombsweeping Day"); break;
    default:return 0;
    }break;
    case 5:switch (day)
    {
    case 1:printf("Labor Day"); break;
    case 4:printf("Chinese Youth Day");
    default:return 0;
    }break;
    case 6:switch (day)
    {
    case 1:printf("Children's Day"); break;
    default:return 0;
    }break;
    case 7:switch (day)
    {
    default:return 0;
    }break;
    case 8:switch (day)
    {
    case 1:printf("the Army's Day"); break;
    default:return 0;
    }break;
    case 9:switch (day)
    {
    case 10:printf("Teacher's Day"); break;
    default:return 0;
    }break;
    case 10:switch (day)
    {
    case 1:printf("National Day"); break;
    case 31:printf("Helloween Day"); break;
    default:return 0;
    }break;
    case 11:switch (day)
    {
    default:return 0;
    }break;
    case 12:switch (day)
    {
    case 25:printf("Christmas Day"); break;
    default:return 0;
    }break;
    }
    return 1;
}
void printWeek(int year, int month, int day)//输出星期
{
    int count;
    count = Weekday(year, month, day);
    switch (count)
    {
    case 0:printf("Sunday"); break;
    case 1:printf("Monday"); break;
    case 2:printf("Tuesday"); break;
    case 3:printf("Wednesday"); break;
    case 4:printf("Thursday"); break;
    case 5:printf("Friday"); break;
    case 6:printf("Saturday"); break;
    }
}

void printMonth(int year, int month)//输出月份
{
    printf("\n  |-----------Welcome to the calendar--------------|");
    int i = 0, j = 1, k=1;
    int n = 0;
    int week, max;
    week = Weekday(year, month, 1);
    max = monthDay(year, month);
    printf("\n  |                  %d/",year);
    if(month<10){printf("%d                        |\n",month);}
    else{printf("%d                       |\n",month);}
    printf("  |  Sun |  Mon |  Tue |  Wed |  Thu |  Fri |  Sat |\n");
    printf("  |------|------|------|------|------|------|------|\n");
    printf("  |  ");
    for (i = 0; i < week; i++)
        
        printf("    |  ");
    for (j = 1; j <= max; j++)
    {
        if (n == 1) { printf("  |  "); n = 0; }
        if (j < 10)
        {
            printf(" ");
        }
        printf("%d  |  ", j);
        if (i % 7 == 6)
        {
            printf("  ");
            while (k <= max)
            {
                if (festival(month, k) == 1) { printf(" of %d/%d", month,k), k++;; break; }
                k++;
            }
            printf("\n");
            printf("  |------|------|------|------|------|------|------|\n");
            n = 1;            
        }
        i++;
    }
    for (int l = 1; l <= 7 - i % 7 && i % 7 != 0; l++)
    {
        if(l==1)printf("    |");
        else printf("      |");
    }
    if (i % 7 != 0) { printf("\n  |------|------|------|------|------|------|------|\n"); }
}
void printDay(int year, int month,int day)//输出某一天的信息
{
    printf("\n%d-%d-%d is ", year, month, day);
    printWeek(year, month, day);
    printf("\nand is the %d day of the year\n", countDay(year, month,day));
    festival(month, day);
}

void calendar(char* option, int* year, int *month, int *day)
{
	clear();
	printMonth(*year, *month);
	char str_list[7][10] = { "-y", "-m", "-d","-pre","-next","-pre-y","-next-y" };
	int flag[7] = { 1, 1, 1 ,1 ,1 ,1 ,1 };
	for (int i = 0; i < 7; ++i)
	{
		int j = 0;
		while (option[j] != ' ' && option[j] != '\0')
		{
			if (option[j] != str_list[i][j])
			{
				flag[i] = 0;
				break;
			}
			j++;
		}
	}
	char year_str[5] = "\0", month_str[3] = "\0", day_str[3] = "\0";
	if (!strcmp(option, "NULL"))
		{
		clear();
		printMonth(*year, *month);
		return;
		}
	else if (flag[0])
	{//-y
		if (strlen(option) > 3)
		{
			char* value = option + 3;
			for (int j = 0; j < strlen(value) && value[j] != ' '; j++)
			{
				year_str[j] = value[j];
			}
			atoi(year_str, year);
			if (*year > 0)
			{
				*month = 1;
				clear();
				printMonth(*year, *month);
			}
			else
			{
				if (*year < 0)
				{
					printf("The year input error.\n");
				}
				printf("Please input again.\n");
			}
		}
		else
		{
			printf("Sorry, you should add Y.\n");
			printf("You can input [man cal] to learn more.\n");
		}
	}
	else if (flag[1])
	{ //-m
		if (strlen(option) > 3)
		{
			char* value = option + 3;

			int i = 0;
			for (int j = 0; i < strlen(value) && value[i] != '/' && value[i] != ' '; ++i, ++j)
			{
				year_str[i] = value[i];
			}
			++i;
			for (int j = 0; i < strlen(value) && value[i] != ' '; ++i, ++j)
			{
				month_str[j] = value[i];
			}

			atoi(year_str, year);
			atoi(month_str, month);

			if (*year > 0 && *month > 0 && *month < 13)
			{
				clear();
				printMonth(*year, *month);
			}
			else
			{
				if (*year < 0)
				{
					printf("The year input error.\n");
				}
				if (*month < 1 || *month > 12)
				{
					printf("The month input error.\n");
				}
				printf("Please input again.\n");
			}
		}
		else
		{
			printf("Sorry, you should add Y/M.\n");
			printf("You can input [man cal] to learn more.\n");
		}
	}
	else if (flag[2])
	{ //-d
		if (strlen(option) > 3)
		{
			char* value = option + 3;

			int i = 0;
			for (int j = 0; i < strlen(value) && value[i] != '/' && value[i] != ' '; ++i, ++j)
			{
				year_str[i] = value[i];
			}
			++i;
			for (int j = 0; i < strlen(value) && value[i] != '/' && value[i] != ' '; ++i, ++j)
			{
				month_str[j] = value[i];
			}
			++i;
			for (int j = 0; i < strlen(value) && value[i] != ' '; ++i, ++j)
			{
				day_str[j] = value[i];
			}

			atoi(year_str, year);
			atoi(month_str, month);
			atoi(day_str, day);

			if (*year > 0 && *month > 0 && *month < 13 && *day > 0 && *day < 32)
			{
				if (leapYear(*year) == 1 && *month == 2 && *day > 29)
				{
					printf("%d is a leap year, the day you input should not greater than 29.\n", *year);
				}
				else if (leapYear(*year) == 0 && *month == 2 && *day > 28)
				{
					printf("%d is a normal year, the day you input should not greater than 28.\n", *year);
				}
				else if((*month==4&&*day==31)||(*month==6&&*day==31)||(*month==9&&*day==31)||(*month==11&&*day==31))
				{
					printf("The day you input should between 1 to 30.");
				}
				else
				{
					clear();
					printMonth(*year, *month);
					printDay(*year, *month, *day);
				}
			}
			else
			{
				if (*year < 0)
				{
					printf("The year you input should greater than 0.\n");
				}
				if (*month < 1 || *month > 12)
				{
					printf("The month you input should between 1 to 12.\n");
				}
				if (*day < 1 || *day > 31)
				{
					printf("The day you input should between 1 to 31.\n");
				}
				printf("Please input again.\n");
			}
		}
		else
		{
			printf("Sorry, you should add Y/M/D.\n");
			printf("You can input [man cal] to know more.\n");
		}
	}
	else if (flag[3])
	{
		*month=*month-1;
		if (*month == 0) { *month = 12; *year=*year-1; }
		clear();
		if (*year < 0) { printf("year must greater than 0.\n"); *year = 0; *month = 1; }
		printMonth(*year, *month);
    }
	else if (flag[4])
	{
	*month=*month+1;
	if (*month == 13) { *month = 1; *year=*year+1; }
	clear();
	printMonth(*year, *month);
	}
	else if (flag[5])
	{
	*year=*year-1;
	clear();
	if (*year < 0) { printf("year must greater than 0.\n"); *year = 0;}
	printMonth(*year, *month);
	}
	else if (flag[6])
	{
	*year=*year+1;
	clear();
	printMonth(*year, *month);
	}
	else
	{
		printf("Sorry, there no such option for cal.\n");
		printf("You can input [man cal] to know more.\n");
	}

	printf("\n");
}


