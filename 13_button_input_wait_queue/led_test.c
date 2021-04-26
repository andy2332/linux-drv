#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argv,char * arge[])
{
	int fd;
	int value;
	int err;

	fd = open("/dev/andy_led",O_RDWR);
	if(fd < 0)
	{
		perror("err open \n");
		return fd;
	}
	while(1)
	{
		read(fd,&value,sizeof(value));
		printf("value is %d\n",value);
	}

	close(fd);


	return 0;
}
