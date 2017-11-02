// ************************************************************
// * Program        : pname.c
// * Description    : Demo program to detect product name
// * Boards Supp.   : Advantech product
// *
// * Revision       : 1.0
// * Date           : 01/16/2006          Advantech Co., Ltd.
// *************************************************************
#include <linux/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#define ADSPNAME_MAGIC                  'p'
#define GETPNAME                        _IO(ADSPNAME_MAGIC, 1)
#define CHKADVBOARD                     _IO(ADSPNAME_MAGIC, 2)

#define PNAME_LENGTH			64
#define BNAME_LENGTH			32


int main ( int argc, char **argv )
{
	int count = 0;
	int fd = 0;
	int ret;
	unsigned long status;
	unsigned char pname[PNAME_LENGTH];
	char*  pos = NULL;
	unsigned char padvboard[BNAME_LENGTH];
	char opt;
	int loopc;
	char* p = NULL;
	char p1[PNAME_LENGTH];
	int isADV = 0;
	
	//open
	if ((fd = open("/dev/adspname", O_RDWR)) < 0 )
	{
		printf("cannot open adspname!\n");
		return -1;
	}

	//read name
	while((opt = getopt(argc,argv,"pby")) != -1)
	{
		switch(opt)
		{
		case 'p':
			status = ioctl( fd, GETPNAME, pname );
			printf("%s\n", pname );
			break;
		case 'b':
			status = ioctl( fd, GETPNAME, pname );
			printf("%s\n", pname );
			break;
		case 'y':
			status = ioctl( fd, CHKADVBOARD, padvboard );
			if(strncmp(padvboard, "yes", 4) == 0)
				isADV = 1;
			if(isADV)
				printf("yes\n");
			else
				printf("no\n");
                	break;
		default:
			printf("help:\n");
			printf("\t-b\tprint name of board\n");
			printf("\t-p\tprint name of device\n");
			printf("\t-y\twhether is advantech product\n");
                        break;
		}	
	}

	//close
  	if ( close( fd ))
	{
		printf("close device file error!\n");
		return -1;
	}

	return 1;
}

 
