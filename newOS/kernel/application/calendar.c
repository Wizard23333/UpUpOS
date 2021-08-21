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


void calMain(char *option)
{
	/*判断附加的选项是何种命令*/
	char str_list[3][10] = {"-month", "-week", "-date"};
	int flag[3] = {1, 1, 1};
	for (int i = 0; i < 3; ++i)
	{
		int j = 0;
		while (option[j] != ' ' && option[j] != '\0')
		{
			if (option[j] != str_list[i][j])
			{
				flag[i] = 0;
				break;
			}
			++j;
		}
	}

	//char value[20] = "\0";
	int year, month, day;
	char year_str[5] = "\0", month_str[3] = "\0", day_str[3] = "\0";

	if (!strcmp(option, "NULL"))
	{
		printf("Sorry, you should add an option.\n");
	}
	else if (flag[0])
	{ //-month
		if (strlen(option) > 7)
		{
			char *value = option + 7;

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

			atoi(year_str, &year);
			atoi(month_str, &month);

			if (year > 0 && month > 0 && month < 13)
			{
				display_month(year, month);
			}
			else
			{
				if (year < 0)
				{
					printf("The [year] you input should greater than 0.\n");
				}
				if (month < 1 || month > 12)
				{
					printf("The [month] you input should between 1 to 12.\n");
				}
				printf("Please input again.\n");
			}
		}
		else
		{
			printf("Sorry, you should add [Y/M].\n");
			printf("You can input [man cal] to know more.\n");
		}
	}
	else if (flag[1])
	{ //-week
		if (strlen(option) > 6)
		{
			char *value = option + 6;

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

			atoi(year_str, &year);
			atoi(month_str, &month);
			atoi(day_str, &day);

			if (year > 0 && month > 0 && month < 13 && day > 0 && day < 32)
			{
				if (Isleap(year) == 1 && month == 2 && day > 29)
				{
					printf("%d is a leap year, the [day] you input should not greater than 29.\n", year);
				}
				else if (Isleap(year) == 0 && month == 2 && day > 28)
				{
					printf("%d is a normal year, the [day] you input should not greater than 28.\n", year);
				}
				else
				{
					display_week(year, month, day);
				}
			}
			else
			{
				if (year < 0)
				{
					printf("The [year] you input should greater than 0.\n");
				}
				if (month < 1 || month > 12)
				{
					printf("The [month] you input should between 1 to 12.\n");
				}
				if (day < 1 || day > 31)
				{
					printf("The day you input should between 1 to 31.\n");
				}
				printf("Please input again.\n");
			}
		}
		else
		{
			printf("Sorry, you should add .\n");
			printf("You can input [man cal] to know more.\n");
		}
	}
	else if (flag[2])
	{ //-date
		if (strlen(option) > 6)
		{
			char *value = option + 6;

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

			atoi(year_str, &year);
			atoi(month_str, &month);
			atoi(day_str, &day);

			if (year > 0 && month > 0 && month < 13 && day > 0 && day < 32)
			{
				if (Isleap(year) == 1 && month == 2 && day > 29)
				{
					printf("%d is a leap year, the [day] you input should not greater than 29.\n", year);
				}
				else if (Isleap(year) == 0 && month == 2 && day > 28)
				{
					printf("%d is a normal year, the [day] you input should not greater than 28.\n", year);
				}
				else
				{
					printf("\n%d/%d/%d is day No.%d of the year.\n", year, month, day, Total_day(year, month, day));
				}
			}
			else
			{
				if (year < 0)
				{
					printf("The [year] you input should greater than 0.\n");
				}
				if (month < 1 || month > 12)
				{
					printf("The [month] you input should between 1 to 12.\n");
				}
				if (day < 1 || day > 31)
				{
					printf("The day you input should between 1 to 31.\n");
				}
				printf("Please input again.\n");
			}
		}
		else
		{
			printf("Sorry, you should add .\n");
			printf("You can input [man cal] to know more.\n");
		}
	}
	else
	{
		printf("Sorry, there no such option for cal.\n");
		printf("You can input [man cal] to know more.\n");
	}

	printf("\n");
}

/*****************************************************************************
 *                                calendar
 *****************************************************************************/
int Isleap(int year)
{
	if ((year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0)))
		return 1;
	else
		return 0;
}

/*判断输入年份二月份的天数
返回值:month的天数*/
int Max_day(int year, int month)
{
	int Day[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
	if (Isleap(year) == 1)
		Day[1] = 29;
	return Day[month - 1];
}

/*计算输入的日期是这一年的多少天*/
int Total_day(int year, int month, int day)
{
	int sum = 0;
	int i = 1;
	for (i = 1; i < month; i++)
		sum = sum + Max_day(year, i);
	sum = sum + day;
	return sum;
}

/*由输入的日期判断当天是星期几
**返回值:count,0～6，分别表示星期日～星期六
*/
int Weekday(int year, int month, int day)
{
	int count;
	count = (year - 1) + (year - 1) / 4 - (year - 1) / 100 + (year - 1) / 400 + Total_day(year, month, day);
	count = count % 7;
	return count;
}


/*显示输入的日期是星期几*/
void display_week(int year, int month, int day)
{
	int count;
	count = Weekday(year, month, day);
	switch (count)
	{
	case 0:printf("\n%d-%d-%d is Sunday\n", year, month, day); break;
	case 1:printf("\n%d-%d-%d is Monday\n", year, month, day); break;
	case 2:printf("\n%d-%d-%d is Tuesday\n", year, month, day); break;
	case 3:printf("\n%d-%d-%d is Wednesday\n", year, month, day); break;
	case 4:printf("\n%d-%d-%d is Thursday\n", year, month, day); break;
	case 5:printf("\n%d-%d-%d is Friday\n", year, month, day); break;
	case 6:printf("\n%d-%d-%d is Saturday\n", year, month, day); break;
	}
}

/*显示输入的日期的当月日历*/
void display_month(int year, int month)
{
	int i = 0, j = 1;
	int week, max;
	week = Weekday(year, month, 1); //由每月1号确定打印制表符的个数
	max = Max_day(year, month);
	printf("\n                   %d/%d\n", year, month);
	printf("Sun    Mon    Tue    Wed    Thu    Fri    Sat\n");
	for (i = 0; i < week; i++)
		printf("       ");
	for (j = 1; j <= max; j++)
	{
		printf("%d     ", j);
		if (j < 10)
			printf(" ");
		if (i % 7 == 6)
			printf("\n");
		i++;
	}
	printf("\n");
}