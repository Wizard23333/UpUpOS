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

/*======================================================================*
							文件管理
 *======================================================================*/
#define MAX_FILE_PER_LAYER 10
#define MAX_FILE_NAME_LENGTH 20
#define MAX_CONTENT_ 50
#define MAX_FILE_NUM 100

//文件ID计数器
int fileIDCount = 0;
int currentFileID = 0;

struct fileBlock
{
	int fileID;
	char fileName[MAX_FILE_NAME_LENGTH];
	int fileType; //0 for txt, 1 for folder
	char content[MAX_CONTENT_];
	int fatherID;
	int children[MAX_FILE_PER_LAYER];
	int childrenNumber;
};
struct fileBlock blocks[MAX_FILE_NUM];
int IDLog[MAX_FILE_NUM];

//文件管理主函数
void runFileManage(int fd_stdin)
{
	char rdbuf[128];
	char cmd[8];
	char filename[120];
	char buf[1024];
	int m, n;
	char _name[MAX_FILE_NAME_LENGTH];
	FSInit();
	int len = ReadDisk();
	ShowMessage();

	while (1)
	{
		for (int i = 0; i <= 127; i++)
			rdbuf[i] = '\0';
		for (int i = 0; i < MAX_FILE_NAME_LENGTH; i++)
			_name[i] = '\0';
		printf("\n/%s:", blocks[currentFileID].fileName);

		int r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;
		assert(fd_stdin == 0);

		char target[10];
		for (int i = 0; i <= 1 && i < r; i++)
		{
			target[i] = '\0';
		}
		
		for (int i = 0; i <= 1 && i < r; i++)
		{
			target[i] = rdbuf[i];
		}
		//printf("command:%s", target);
		if (rdbuf[0] == 'c' && rdbuf[1] == 'r' && rdbuf[2] == 'e' && rdbuf[3] == 'a' && rdbuf[4] == 't' && rdbuf[5] == 'e')
		{
			if (rdbuf[6] != ' ')
			{
				printf("You should add the filename, like \"create XXX\".\n");
				printf("Please input [help] to know more.\n");
				continue;
			}
			for (int i = 0; i < MAX_FILE_NAME_LENGTH && i < r - 3; i++)
			{
				_name[i] = rdbuf[i + 6];
			}
			CreateFIle(_name, 0);
		}
		else if (rdbuf[0] == 'm' && rdbuf[1] == 'k' && rdbuf[2] == 'd' && rdbuf[3] == 'i' && rdbuf[4] == 'r')
		{
			if (rdbuf[5] != ' ')
			{
				printf("You should add the dirname, like \"mkdir XXX\".\n");
				printf("Please input [help] to know more.\n");
				continue;
			}
			char N[MAX_FILE_NAME_LENGTH];
			for (int i = 0; i < MAX_FILE_NAME_LENGTH && i < r - 3; i++)
			{
				_name[i] = rdbuf[i + 6];
			}
			CreateFIle(_name, 1);
		}
		else if (rdbuf[0] == 'r' && rdbuf[1] == 'e' && rdbuf[2] == 'a' && rdbuf[3] == 'd')
		{
			if (rdbuf[4] != ' ')
			{
				printf("You should add the file name, like \"read XXX\".\n");
				printf("Please input [help] to know more.\n");
				continue;
			}
			char N[MAX_FILE_NAME_LENGTH];
			for (int i = 0; i < MAX_FILE_NAME_LENGTH && i < r - 3; i++)
			{
				_name[i] = rdbuf[i +5];
			}
			ReadFIle(_name);
		}
		else if (rdbuf[0] == 'w' && rdbuf[1] == 'r' && rdbuf[2] == 'i' && rdbuf[3] == 't' && rdbuf[4] == 'e')
		{
			if (rdbuf[5] != ' ')
			{
				printf("You should add the dirname, like \"write XXX xxx\".\n");
				printf("Please input [help] to know more.\n");
				continue;
			}
			char N[MAX_FILE_NAME_LENGTH],temp[MAX_CONTENT_];
			for (int i = 0; i < MAX_CONTENT_; i++)
			      temp[i] = '\0';
			int i = 0,j=0;
			for (i=0; i < MAX_FILE_NAME_LENGTH && i < r - 3; i++)
			{
				if(rdbuf[i + 6]==' ')
				    break;
				_name[i] = rdbuf[i + 6];
			}
			for (i++,j=0; i < MAX_FILE_NAME_LENGTH && i < r - 3; i++,j++)
			{
				temp[j] = rdbuf[i+6];
			}
			//printf("%s  %s %s\n",_name, temp,rdbuf);
			WriteFIle(_name, temp);
		}
		else if (strcmp(rdbuf, "ls") == 0)
		{
			showFileList();
		}
		else if (strcmp(target, "cd") == 0)
		{
			if (rdbuf[2] == ' ' && rdbuf[3] == '.' && rdbuf[4] == '.')
			{
				ReturnFile(currentFileID);
				continue;
			}
			else if (rdbuf[2] != ' ')
			{
				printf("You should add the dirname, like \"cd XXX\".\n");
				printf("Please input [help] to know more.\n");

				continue;
			}
			for (int i = 0; i < MAX_FILE_NAME_LENGTH && i < r - 3; i++)
			{
				_name[i] = rdbuf[i + 3];
			}
			printf("name: %s\n", _name);
			int ID = SearchFile(_name);
			if (ID >= 0)
			{
				if (blocks[ID].fileType == 1)
				{
					currentFileID = ID;
					continue;
				}
				else if (blocks[ID].fileType == 0)
				{
					while (1)
					{
						printf("input the character representing the method you want to operate:"
							   "\nu --- update"
							   "\nd --- detail of the content"
							   "\nq --- quit\n");
						int r = read(fd_stdin, rdbuf, 70);
						rdbuf[r] = 0;
						if (strcmp(rdbuf, "u") == 0)
						{
							printf("input the text you want to write:\n");
							int r = read(fd_stdin, blocks[ID].content, MAX_CONTENT_);
							blocks[ID].content[r] = 0;
						}
						else if (strcmp(rdbuf, "d") == 0)
						{
							printf("--------------------------------------------"
								   "\n%s\n-------------------------------------\n",
								   blocks[ID].content);
						}
						else if (strcmp(rdbuf, "q") == 0)
						{
							printf("would you like to save the change? y/n");
							int r = read(fd_stdin, rdbuf, 70);
							rdbuf[r] = 0;
							if (strcmp(rdbuf, "y") == 0)
							{
								printf("save changes!");
							}
							else
							{
								printf("quit without changing");
							}
							break;
						}
					}
				}
			}
			else
				printf("No such file!");
		}
		else if (strcmp(target, "rm") == 0)
		{
			if (rdbuf[2] != ' ')
			{
				printf("You should add the filename or dirname, like \"rm XXX\".\n");
				printf("Please input [help] to know more.\n");
				continue;
			}
			for (int i = 0; i < MAX_FILE_NAME_LENGTH && i < r - 3; i++)
			{
				_name[i] = rdbuf[i + 3];
			}
			int ID = SearchFile(_name);
			if (ID >= 0)
			{
				printf("Delete successfully!\n");
				DeleteFile(ID);
				for (int i = 0; i < blocks[currentFileID].childrenNumber; i++)
				{
					if (ID == blocks[currentFileID].children[i])
					{
						for (int j = i + 1; j < blocks[currentFileID].childrenNumber; j++)
						{
							blocks[currentFileID].children[i] = blocks[currentFileID].children[j];
						}
						blocks[currentFileID].childrenNumber--;
						break;
					}
				}
			}
			else
				printf("No such file!\n");
		}
		else if (strcmp(target, "sv") == 0)
		{
			WriteDisk(1000);
			printf("Save to disk successfully!\n");
		}
		else if (strcmp(rdbuf, "help") == 0)
		{
			printf("\n");
			ShowMessage();
		}
		else if (strcmp(rdbuf, "quit") == 0)
		{
			clear();
			break;
		}
		else if (!strcmp(rdbuf, "clear"))
		{
			clear();
		}
		else
		{
			printf("Sorry, there no such command in the File Manager.\n");
			printf("You can input [help] to know more.\n");
		}
	}
}

void initFileBlock(int fileID, char *fileName, int fileType)
{
	blocks[fileID].fileID = fileID;
	strcpy(blocks[fileID].fileName, fileName);
	blocks[fileID].fileType = fileType;
	blocks[fileID].fatherID = currentFileID;
	blocks[fileID].childrenNumber = 0;
}

void toStr3(char *temp, int i)
{
	if (i / 100 < 0)
		temp[0] = (char)48;
	else
		temp[0] = (char)(i / 100 + 48);
	if ((i % 100) / 10 < 0)
		temp[1] = '0';
	else
		temp[1] = (char)((i % 100) / 10 + 48);
	temp[2] = (char)(i % 10 + 48);
}

void WriteDisk(int len)
{
	char temp[MAX_FILE_NUM * 150 + 103];
	int i = 0;
	temp[i] = '^';
	i++;
	toStr3(temp + i, fileIDCount);
	i = i + 3;
	temp[i] = '^';
	i++;
	for (int j = 0; j < MAX_FILE_NUM; j++)
	{
		if (IDLog[j] == 1)
		{
			toStr3(temp + i, blocks[j].fileID);
			i = i + 3;
			temp[i] = '^';
			i++;
			for (int h = 0; h < strlen(blocks[j].fileName); h++)
			{
				temp[i + h] = blocks[j].fileName[h];
				if (blocks[j].fileName[h] == '^')
					temp[i + h] = (char)1;
			}
			i = i + strlen(blocks[j].fileName);
			temp[i] = '^';
			i++;
			temp[i] = (char)(blocks[j].fileType + 48);
			i++;
			temp[i] = '^';
			i++;
			for (int h = 0; h < strlen(blocks[j].content); h++)
			{
				temp[i + h] = blocks[j].content[h];
				if (blocks[j].content[h] == '^')
					temp[i + h] = (char)1;
			}
			i = i + strlen(blocks[j].content);
			temp[i] = '^';
			i++;
			toStr3(temp + i, blocks[j].fatherID);
			i = i + 3;
			temp[i] = '^';
			i++;
			for (int m = 0; m < MAX_FILE_PER_LAYER; m++)
			{
				toStr3(temp + i, blocks[j].children[m]);
				i = i + 3;
			}
			temp[i] = '^';
			i++;
			toStr3(temp + i, blocks[j].childrenNumber);
			i = i + 3;
			temp[i] = '^';
			i++;
		}
	}
	int fd = 0;
	int n1 = 0;
	fd = open("ss", O_RDWR);
	assert(fd != -1);
	n1 = write(fd, temp, strlen(temp));
	assert(n1 == strlen(temp));
	close(fd);
}

int toInt(char *temp)
{
	int result = 0;
	for (int i = 0; i < 3; i++)
		result = result * 10 + (int)temp[i] - 48;
	return result;
}

int ReadDisk()
{
	char bufr[1000];
	int fd = 0;
	int n1 = 0;
	fd = open("ss", O_RDWR);
	assert(fd != -1);
	n1 = read(fd, bufr, 1000);
	assert(n1 == 1000);
	bufr[n1] = 0;
	int r = 1;
	fileIDCount = toInt(bufr + r);
	r = r + 4;
	for (int i = 0; i < fileIDCount; i++)
	{
		int ID = toInt(bufr + r);
		IDLog[ID] = 1;
		blocks[ID].fileID = ID;
		r = r + 4;
		for (int i = 0; i < MAX_FILE_NAME_LENGTH; i++)
		{
			if (bufr[r] == '^')
				break;
			else if (bufr[r] == (char)1)
				blocks[ID].fileName[i] = '^';
			else
				blocks[ID].fileName[i] = bufr[r];
			r++;
		}
		r++;
		blocks[ID].fileType = (int)bufr[r] - 48;
		r = r + 2;
		for (int j = 0; j < MAX_CONTENT_; j++)
		{
			if (bufr[r] == '^')
				break;
			else if (bufr[r] == (char)1)
				blocks[ID].content[j] = '^';
			else
				blocks[ID].content[j] = bufr[r];
			r++;
		}
		r++;
		blocks[ID].fatherID = toInt(bufr + r);
		r = r + 4;
		for (int j = 0; j < MAX_FILE_PER_LAYER; j++)
		{
			blocks[ID].children[j] = toInt(bufr + r);
			r = r + 3;
		}
		r++;
		blocks[ID].childrenNumber = toInt(bufr + r);
		r = r + 4;
	}
	return n1;
}

void FSInit()
{

	for (int i = 0; i < MAX_FILE_NUM; i++)
	{
		blocks[i].childrenNumber = 0;
		blocks[i].fileID = -2;
		IDLog[i] = '\0';
	}
	IDLog[0] = 1;
	blocks[0].fileID = 0;
	strcpy(blocks[0].fileName, "home");
	strcpy(blocks[0].content, "welcome to use file system!");
	blocks[0].fileType = 2;
	blocks[0].fatherID = 0;
	blocks[0].childrenNumber = 0;
	currentFileID = 0;
	fileIDCount = 1;
}

int CreateFIle(char *fileName, int fileType)
{
	if (blocks[currentFileID].childrenNumber == MAX_FILE_PER_LAYER)
	{
		printf("Sorry you cannot add more files in this layer.\n");
		return 0;
	}
	else
	{
		for (int i = 0; i < blocks[currentFileID].childrenNumber; i++)
		{
			if (strcmp(blocks[blocks[currentFileID].children[i]].fileName, fileName) == 0)
			{
				if (fileType)
				{
					printf("You have a folder of same name!\n");
				}
				else
				{
					printf("You have a file of same name!\n");
				}
				return 0;
			}
		}
		fileIDCount++;
		int target = 0;
		for (int i = 0; i < MAX_FILE_NUM; i++)
		{
			if (IDLog[i] == 0)
			{
				target = i;
				break;
			}
		}
		initFileBlock(target, fileName, fileType);
		blocks[currentFileID].children[blocks[currentFileID].childrenNumber] = target;
		blocks[currentFileID].childrenNumber++;
		if (fileType)
		{
			printf("Create directory %s successful!\n", fileName);
		}
		else
		{
			printf("Create file %s successful!\n", fileName);
		}
		IDLog[target] = 1;
		return 1;
	}
}


int ReadFIle(char *fileName)
{
	
		for (int i = 0; i < blocks[currentFileID].childrenNumber; i++)
		{
			if (strcmp(blocks[blocks[currentFileID].children[i]].fileName, fileName) == 0)
			{
				if (!blocks[blocks[currentFileID].children[i]].fileType)
				{
                    printf("content: %s",blocks[blocks[currentFileID].children[i]].content);
					return 1;
				}
				
			}
		}
        printf("You need input a file  name!\n");
		return 0;
	
}
int WriteFIle(char *fileName, char *fileContent)
{
	
		for (int i = 0; i < blocks[currentFileID].childrenNumber; i++)
		{
			if (strcmp(blocks[blocks[currentFileID].children[i]].fileName, fileName) == 0)
			{
				if (!blocks[blocks[currentFileID].children[i]].fileType)
				{
                    strcpy(blocks[blocks[currentFileID].children[i]].content,fileContent);
					return 1;
				}
				
			}
		}
        printf("You need input a file  name!\n");
		return 0;
	
}
void showFileList()
{
	printf("The elements in %s.\n", blocks[currentFileID].fileName); //通过currentFileID获取当前路径s

    printf("\n#==================================================================#\n");
	printf("#                          Welcome to UpUpOS                       #\n");
	printf("#            --------------- File    List ---------------          #\n");
	printf("#                                                                  #\n");
	printf("#  filename                type               id                   #\n");
	
	for (int i = 0; i < blocks[currentFileID].childrenNumber; i++)
	{ //遍历每个孩子
	    printf("#");
		printf("%10s", blocks[blocks[currentFileID].children[i]].fileName);
		if (blocks[blocks[currentFileID].children[i]].fileType == 0)
		{
			printf("             .txt file  ");
		}
		else
		{
			printf("               folder   ");
		}
		printf("            %d                   #\n", blocks[blocks[currentFileID].children[i]].fileID);
	}
	printf("#==================================================================#\n");
	printf("#====|create|mkdir|read|write|ls |rm |sv | cd |help|clear|quit|====#\n");
	printf("#==================================================================#\n\n");
}

int SearchFile(char *name)
{
	for (int i = 0; i < blocks[currentFileID].childrenNumber; i++)
	{
		if (strcmp(name, blocks[blocks[currentFileID].children[i]].fileName) == 0)
		{
			return blocks[currentFileID].children[i];
		}
	}
	return -2;
}

void ReturnFile(int ID)
{
	currentFileID = blocks[ID].fatherID;
}

void DeleteFile(int ID)
{
	if (blocks[ID].childrenNumber > 0)
	{
		for (int i = 0; i < blocks[ID].childrenNumber; i++)
		{
			DeleteFile(blocks[blocks[ID].children[i]].fileID);
		}
	}
	IDLog[ID] = 0;
	blocks[ID].fileID = -2;
	blocks[ID].childrenNumber = 0;
	for (int i = 0; i < MAX_CONTENT_; i++)
		blocks[ID].content[i] = '\0';
	for (int i = 0; i < MAX_FILE_NAME_LENGTH; i++)
		blocks[ID].fileName[i] = '\0';
	blocks[ID].fileType = -1;
	for (int i = 0; i < MAX_FILE_PER_LAYER; i++)
		blocks[ID].children[i] = -1;
	blocks[ID].fatherID = -2;
	fileIDCount--;
}

void ShowMessage()
{
	

	printf("================================================================================");
	printf("                                                                                ");
	printf("  ***     ***                                  UpUpOS                           ");
	printf("  ***     ***                                                                   ");
	printf("  ***     ***  *********     DESCRIPTION                                        ");
	printf("  ***     ***  **       **       cd [dirname]    switch work path to this       ");
	printf("   *********   **       **                       directory                      ");
	printf("               **       **       read [filename]    read a .txt file            ");
	printf("               **********        create [filename]    create a new .txt file    ");
	printf("               **                rm [name]    delete a file or directory        ");
	printf("  ***     ***  **                mkdir [dirname]    create a new folder         ");
	printf("  ***     ***                    sv    save a file                              ");
	printf("  ***     ***                    ls    list the elements in this level          ");
	printf("  ***     *** *********          help    show command list of file system       ");
	printf("   *********  **       **        quit    quit systemmonth                       ");
	printf("              **       **        clear    clear the page                        ");
	printf("              **       **        write [filename] [content]  write a .txt file  ");
	printf("              **********                                                        ");
	printf("              **                                                                ");
	printf("              **                                                                ");
	printf("================================================================================");

	printf("\n\n");
}
