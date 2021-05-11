#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

/*
�÷�:
at24c02_test w val 
at24c02_test r

"0x12"=> strtoul
"12"=>12 atoi

������ַ: 0x50
Ƭ�ڵ�ַ: 0x7
/dev/i2c-e2prom

�˽�i2c��дʱ��

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
	char data_addr = 0x07;//Ƭ�ڵ�ַ ���ָ�� 0-255
	char rbuf[10];
	char wbuf[10];
	unsigned char val;//�ֽڱ���
	if(argc<2){
		print_usage(argv[0]);
		exit(1);
	}

	//1.�������������豸�ڵ� 
	fd = open("/dev/i2c_e2prom",O_RDWR);
	if(fd<0){
		perror("open fail\n");
		exit(1);
	}

	//2.���������в��������� д Ƭ�ڵ�ַ,����
	if(strcmp(argv[1],"r")==0){
		if(write(fd,&data_addr,1)!=1){
			perror("e2prom write fail\n");
			exit(1);
		}
		//��e2prom���ֽ�����
		if(read(fd,rbuf,1)!=1){
			perror("e2prom read fail\n");
			exit(1);
		}
		//���ɹ������ݴ�ӡ
		printf("rbuf[0]=0x%x\n",rbuf[0]);
		
	}
	else if (argc==3 && (strcmp(argv[1],"w")==0)){
		val = strtoul(argv[2],NULL,0);//�ַ���ת��Ϊ����
		wbuf[0] = data_addr; //Ƭ�ڵ�ַ
		wbuf[1] = val; //Ҫд��e2prom���ֽ�����
		if(write(fd,wbuf,2)!=2){
			perror("e2prom write fail\n");
			exit(1);
		}
		printf("write data ok\n");
	}
	close(fd);
	return 0;

}
