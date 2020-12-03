//customized virtual file system

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>

void RestoreFile();

#define MAXINODE 50

#define MAXFILES 100
#define FILESIZE 1024

#define READ 4
#define WRITE 2

#define REGULAR 1
#define SPECIAL 2

struct SuperBlock
{
	int TotalInodes;
	int FreeInode;
}Obj_Super;

typedef SuperBlock SUPERBLOCK;

struct inode
{
	char Fname[50];
	int Inode_number;
	int FileSize;
	int FileType;
	int ActualSize;
	int Link_Count;
	int Reference_Count;
	char *Data;
	struct inode *next;
};

typedef struct inode INODE;
typedef struct inode * PINODE;
typedef struct inode ** PPINODE;

struct FileTable
{
	int ReadOffset;
	int WriteOffset;
	int Count;
	PINODE iptr;
	int Mode;
};

typedef FileTable FILETABLE;
typedef FileTable *PFILETABLE;

struct UFDT
{
	PFILETABLE ufdt[MAXFILES];
}UFDTObj;

SUPERBLOCK SUPERBLOCKobj;
PINODE Head = NULL; //global pointer

bool CheckFile(char *Name)
{
	PINODE temp = Head;
	while(temp != NULL)
	{
		if(temp->FileType != 0)
		{
			if(strcmp(temp->Fname,Name) == 0)
			{
				break;
			}
		}
		temp = temp->next;
	}
	if(temp == NULL)
	{
		return false;
	}
	else
	{
		return true;
	}
}
void CreateUFDT()
{
	int i = 0;
	for(i = 0; i < MAXFILES; i++)
	{
		UFDTObj.ufdt[i] = NULL;
	}
	printf("UFDT is set....\n");
}
void CreateDILB() //creates LinkedList of iNode
{
	int i = 1;
	PINODE newn = NULL;
	PINODE temp = Head;

	while(i < MAXFILES) // iterates 100 times
	{
		newn = (PINODE)malloc(sizeof(INODE));

		newn->Inode_number = i;
		newn->FileSize = FILESIZE;
		newn->FileType = 0;
		newn->ActualSize = 0;
		newn->Link_Count = 0;
		newn->Reference_Count = 0;
		newn->Data = NULL;
		newn->next = NULL;

		if(Head == NULL) //First iNode
		{
			Head = newn;
			temp = Head;
		}
		else
		{
			temp->next = newn;
			temp = temp->next;
		}
		i++;
	}
	printf("DILB created Successfully\n");
}

void CreateSuperBlock()
{
	Obj_Super.TotalInodes = MAXFILES;
	Obj_Super.FreeInode = MAXFILES;

	printf("SuperBlock created successfully\n");
}

void SetEnvironment()
{
	CreateDILB();
	CreateSuperBlock();
	CreateUFDT();
	printf("Environment For Virtual File system is set...\n");
	//RestoreFile();
}

int CreateFile(char *name,int permission)
{
	bool bret = false;
	if((name == NULL) || (permission > READ + WRITE) || (permission < WRITE))
	{
		return -1;
	}

	bret = CheckFile(name);

	if(bret == true)
	{
		printf("File already present\n");
		return -1;
	}

	if(Obj_Super.FreeInode == 0)
	{
		printf("No inode to create file\n");
		return -1;
	}
	int i = 0;
	for(i = 0; i < MAXFILES;i++)
	{
		if(UFDTObj.ufdt[i] == NULL)
		{
			break;
		}
	}

	//Allocate Memory for FileTable
	UFDTObj.ufdt[i] = (PFILETABLE)malloc(sizeof(FILETABLE));

	//Initialise the filetable
	UFDTObj.ufdt[i]->ReadOffset = 0;
	UFDTObj.ufdt[i]->WriteOffset = 0;
	UFDTObj.ufdt[i]->Mode = permission;
	UFDTObj.ufdt[i]->Count = 1;

	//Search empty inode
	PINODE temp = Head;
	while(temp != NULL)
	{
		if(temp->FileType == 0)
		{
			break;
		}
		temp = temp->next;
	}

	UFDTObj.ufdt[i]->iptr = temp;
	strcpy(UFDTObj.ufdt[i]->iptr->Fname,name);
	UFDTObj.ufdt[i]->iptr->FileSize = FILESIZE;
	UFDTObj.ufdt[i]->iptr->FileType = REGULAR;
	UFDTObj.ufdt[i]->iptr->ActualSize = 0;
	UFDTObj.ufdt[i]->iptr->Link_Count = 1;
	UFDTObj.ufdt[i]->iptr->Reference_Count = 1;
	UFDTObj.ufdt[i]->iptr->Data = (char *)malloc(FILESIZE);

	return i;
}
void LS()
{
	int i = 0;
	PINODE temp = Head;

	if(SUPERBLOCKobj.FreeInode == MAXINODE)
	{
		printf("ERROR : There are no files");
		return;
	}
	while(temp != NULL)
	{
		if(temp->FileType != 0)
		{
			printf("%s\n",temp->Fname);
		}
		temp = temp->next;
	}
}

/*int DeleteFile(char *name)
{
	int fd = 0;

	fd = GetFDFromName(name);
	if(fd == -1)
	{
		return -1;
	}

	(ufdt[fd].iptr)
}*/

int WriteFile(int fd,char *arr,int size)
{
	if(UFDTObj.ufdt[fd] == NULL)
	{
		printf("Invalid file descriptor\n");
		return -1;
	}

	if(UFDTObj.ufdt[fd]->Mode == READ)
	{
		printf("There is no write permission");
		return -1;
	}

	strncpy(((UFDTObj.ufdt[fd]->iptr->Data)+(UFDTObj.ufdt[fd]->WriteOffset)),arr,size);
	UFDTObj.ufdt[fd]->WriteOffset = UFDTObj.ufdt[fd]->WriteOffset+size;

	return size;
}
void DisplayHelp()
{
	printf("\n--------------------------------------------\n");
	printf("Open: It is used to open existing file\n");
	printf("close: It is used to close opened file\n");
	printf("read: It is used to read the contents of file\n");
	printf("write: It is used to write data in file\n");
	printf("lseek: It is used to change offset of file\n");
	printf("stat: It is used to get information of all files\n");
	printf("fstat: It is used to get info of regular files\n");
	printf("\n--------------------------------------------\n");
}

void ManPage(char *str)
{
	if(strcmp(str,"open") == 0)
	{
		printf("Description : It is used to open exiting file\n");
		printf("usage : open File_name Mode\n");
	}
	else if(strcmp(str,"close") == 0)
	{
		printf("Description : It is used to close existing file\n");
		printf("usage : close File_name\n");
	}
	else if(strcmp(str,"ls") == 0)
	{
		printf("Description : used to show file names\n");
		printf("usage : ls\n");
	}
	else if(strcmp(str,"creat") == 0)
	{
		printf("Description : used to create file names\n");
		printf("usage : creat Filename permission\n");
	}
	else
	{
		printf("Man entry not found\n");
	}
}

/*void BackupFS()
{
    int fd = 0;
    // Store the address if linkelist head
    PINODE temp = Head;

    // Create new file for backup
    fd = creat("Marvellous.txt",0777);
    if(fd == -1)
    {
        printf("Unable to create the file");
        return -1;
    }

    // Travel the inodes linkedlist
    while(temp != NULL)
    {
        // Check whether file is existing
        if(temp->FileType != 0)
        {
            // Write the inode into the backp files
            write(fd,temp,sizeof(INODE));
            // Write the data of file into the backup file
            write(fd,temp->Data, 1024);
        }
        // Incerment the pointer by one
        temp = temp - > next;
    }
}

void RestoreFile()
{
    int fd = 0;
    INODE iobj;

    char Data[1024];

    fd = open("Marvellous.txt".O_RDONLY);
    if(fd == -1)
    {
        printf("Unable to open tyhe file\n");
        return;
    }

    while(ret = (read(fd,iobj,sizeof(iobj))) != 0)
    {
        // Copy the contents from Buffer into the inode of IIT
        // Only copy the information
        // Example
        temp->permission = iobj.permission;

        // read the files data
        read(fd,Data,1024);
        // Copy the data of file
        memcpy(temp->Buffer,Data, 1024);
    }
    close(fd);
    printf("BAckup done succesfully")
}
*/

int main()
{
	char str[80];
	char command[4][80];
	int count = 0;
	printf("Customised Virtual File System\n");
	SetEnvironment();
	while(1)
	{
		printf("Marvellous VFS >");
		fgets(str,80,stdin);
		fflush(stdin);

		count = sscanf(str,"%s %s %s %s",command[0],command[1],command[2],command[3]);

		if(count == 1)
		{
			if(strcmp(command[0],"help") == 0)
			{
				DisplayHelp();
			}
			else if(strcmp(command[0],"exit") == 0)
			{
				//BackupFS();
				printf("Thank you for using VFS\n");
				break;
			}
			else if(strcmp(command[0],"clear") == 0)
			{
				system("cls");
			}
			else if(strcmp(command[0],"ls") == 0)
			{
				LS();
			}
		}
		else if(count == 2)
		{
			if(strcmp(command[0],"man") == 0)
			{
				ManPage(command[1]);
			}
			else if(strcmp(command[0],"rm") == 0)
			{

			}
			else if(strcmp(command[0],"write") == 0)
			{
				char arr[1024];

                printf("Please enter data to write\n");
                fgets(arr,1024,stdin);

                fflush(stdin);

                int ret = WriteFile(atoi(command[1]),arr,strlen(arr)-1);
                if(ret != -1)
                {
                    printf("%d bytes gets written succesfully in the file\n",ret);
				}
			}
			else
			{
				printf("Command Not Found\n");
			}
		}
		else if(count == 3)
		{
			if(strcmp(command[0],"creat") == 0)
			{
				int fd = 0;
				fd = CreateFile(command[1],atoi(command[2]));

				if(fd == -1)
				{
					printf("Unable to create file\n");
				}
				else
				{
					printf("File Creation Successful\n");
				}
			}
		}
		else if(count == 4)
		{

		}
		else
		{
			printf("Bad Command or file name\n");
		}
	}
	return 0;
}
