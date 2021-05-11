#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

/*
用法:
at24c02_test w val 
at24c02_test r

"0x12"=> strtoul
"12"=>12 atoi

器件地址: 0x50
片内地址: 0x7
/dev/i2c-e2prom

了解i2c读写时序

*/

void print_usage(char *str)
{
	printf("%s r: read at24c02 address0\n",str);
	printf("%s w val: write at24c02 address0",str);
}

int main(int argc,char **argv)
{
	int fd;
	int ret;
	char data_addr = 0x07;//片内地址 随便指定 0-255
	char rbuf[10];
	char wbuf[10];
	unsigned char val;//字节变量
	if(argc<2){
		print_usage(argv[0]);
		exit(1);
	}

	//1.打开驱动创建的设备节点 
	fd = open("/dev/i2c_e2prom",O_RDWR);
	if(fd<0){
		perror("open fail\n");
		exit(1);
	}

	//2.根据命令行参数决定读 写 片内地址,数据
	if(strcmp(argv[1],"r")==0){
		if(write(fd,&data_addr,1)!=1){
			perror("e2prom write fail\n");
			exit(1);
		}
		//读e2prom的字节数据
		if(read(fd,rbuf,1)!=1){
			perror("e2prom read fail\n");
			exit(1);
		}
		//读成功将数据打印
		printf("rbuf[0]=0x%x\n",rbuf[0]);
		
	}
	else if (argc==3 && (strcmp(argv[1],"w")==0)){
		val = strtoul(argv[2],NULL,0);//字符串转换为整数
		wbuf[0] = data_addr; //片内地址
		wbuf[1] = val; //要写入e2prom的字节数据
		if(write(fd,wbuf,2)!=2){
			perror("e2prom write fail\n");
			exit(1);
		}
		printf("write data ok\n");
	}
	close(fd);
	return 0;

}
