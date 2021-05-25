
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <errno.h>
 
#define LOGI printf
#define LOGE printf
//14byte文件头
typedef struct
{
	char cfType[2];//文件类型，"BM"(0x4D42)
	long cfSize;//文件大小（字节）
	long cfReserved;//保留，值为0
	long cfoffBits;//数据区相对于文件头的偏移量（字节）
}__attribute__((packed)) BITMAPFILEHEADER;
//__attribute__((packed))的作用是告诉编译器取消结构在编译过程中的优化对齐
 
//40byte信息头
typedef struct
{
	char ciSize[4];//BITMAPFILEHEADER所占的字节数
	long ciWidth;//宽度
	long ciHeight;//高度
	char ciPlanes[2];//目标设备的位平面数，值为1
	int ciBitCount;//每个像素的位数
	char ciCompress[4];//压缩说明
	char ciSizeImage[4];//用字节表示的图像大小，该数据必须是4的倍数
	char ciXPelsPerMeter[4];//目标设备的水平像素数/米
	char ciYPelsPerMeter[4];//目标设备的垂直像素数/米
	char ciClrUsed[4]; //位图使用调色板的颜色数
	char ciClrImportant[4]; //指定重要的颜色数，当该域的值等于颜色数时（或者等于0时），表示所有颜色都一样重要
}__attribute__((packed)) BITMAPINFOHEADER;
 
typedef struct
{
	unsigned short blue;
	unsigned short green;
	unsigned short red;
	unsigned short reserved;
}__attribute__((packed)) PIXEL;//颜色模式RGB
 
BITMAPFILEHEADER FileHead;
BITMAPINFOHEADER InfoHead;
 
static char *fbp = 0;
static int xres = 0;
static int yres = 0;
static int bits_per_pixel = 0;
 
int show_bmp();
int show_bmp2();
int fbfd = 0;
static void fb_update(struct fb_var_screeninfo *vi)   //将要渲染的图形缓冲区的内容绘制到设备显示屏来
{  
    vi->yoffset = 1;  
    ioctl(fbfd, FBIOPUT_VSCREENINFO, vi);  
    vi->yoffset = 0;  
    ioctl(fbfd, FBIOPUT_VSCREENINFO, vi);  
}  
 
int width, height;
 
static int cursor_bitmpa_format_convert(char *dst,char *src){
	int i ,j ;
	char *psrc = src ;
	char *pdst = dst;
	char *p = psrc;
	int value = 0x00;
	int lcd_h,lcd_w;
	lcd_h = yres;
	lcd_w = xres;
	/* 由于bmp存储是从后面往前面，所以需要倒序进行转换 */
	pdst += (lcd_w * height * 4);
	for(i=0;i<height;i++){
		p = psrc + (i+1) * width * 3;
		pdst = pdst - (lcd_w - width)*4;
		for(j=0;j<width;j++){
			pdst -= 4;
			p -= 3;
			pdst[0] = p[0];
			pdst[1] = p[1];
			pdst[2] = p[2];
			pdst[3] = 0x00;
 /*
			value = *((int*)pdst);
			value = pdst[0];
			if(value == 0x00){
				pdst[3] = 0x00;
			}else{
				pdst[3] = 0xff;
			}
*/	
	}
	}
 
	return 0;
}

static int cursor_bitmpa_format_convert_andy(char *dst,char *src){
	int i,j;
	char *lcd_p_init = dst;
	char *bmp_p_init = src;
	char *lcd_p = lcd_p_init;
	char *bmp_p = bmp_p_init;
	int value = 0x00;
	int lcd_h,lcd_w;
	lcd_h = yres;
	lcd_w = xres;
	
	printf("height : %d \n width : %d \n lcd_h : %d\n lcd_w : %d\n",height,width,lcd_h,lcd_w);
	
	for(i = 0 ; i < height ; i++)
	{
		lcd_p = lcd_p_init + (height - 1 - i) * lcd_w *4;
		bmp_p = bmp_p_init + (i) * width *3;
		printf("%d ",i);
		for(j = 0; j < width; j++)
		{
			lcd_p[0] = bmp_p[0];
			lcd_p[1] = bmp_p[1];
			lcd_p[2] = bmp_p[2];
			lcd_p[3] = 0x00;
			/*
			value = *((int*)lcd_p);
			
			value = lcd_p[0];
			if(value == 0x00){
				lcd_p[3] = 0x00;
			}else{
				lcd_p[3] = 0xff;
			}
			*/
			lcd_p += 4;
			bmp_p +=3;
		}			
	}
	printf("\n");
	return 0;
}


 
int show_bmp(char *path)
{
	FILE *fp;
	int rc;
	int line_x, line_y;
	long int location = 0, BytesPerLine = 0;
	char *bmp_buf = NULL;
	char *bmp_buf_dst = NULL;
	char * buf = NULL;
	int flen = 0;
	int ret = -1;
	int total_length = 0;
	
	LOGI("into show_bmp function_____________________________________________________________________________________\n");
	if(path == NULL)
		{
			LOGE("path Error,return");
			return -1;
		}
	LOGI("path = %s", path);
	fp = fopen( path, "rb" );
	if(fp == NULL){
		LOGE("load > cursor file open failed");
		return -1;
	}
	/* 求解文件长度 */
	//fseek(fp,0,SEEK_SET);
	fseek(fp,0,SEEK_END);
	//ftell用于得到当前指针位置相对于文件首地址的偏移字节数
	flen = ftell(fp);
 
	//
	bmp_buf = (char*)calloc(1,flen - 54);
	LOGI(" bmp_buf = %d\n",bmp_buf);
	if(bmp_buf == NULL){
		LOGE("load > malloc bmp out of memory!");
		return -1;
	}
 
	/* 再移位到文件头部 */
	fseek(fp,0,SEEK_SET);
	//获取文件头信息 长度为14个字节
	rc = fread(&FileHead, sizeof(BITMAPFILEHEADER),1, fp);
	if ( rc != 1)
	{
		LOGI("read header error!\n");
		fclose( fp );
		return( -2 );
	}
	
	//检测是否是bmp图像，长度40个字节
	if (memcmp(FileHead.cfType, "BM", 2) != 0)
	{
		LOGI("it's not a BMP file\n");
		fclose( fp );
		return( -3 );
	}
	//获取信息头文件信息
	rc = fread( (char *)&InfoHead, sizeof(BITMAPINFOHEADER),1, fp );
	if ( rc != 1)
	{
		LOGI("read infoheader error!\n");
		fclose( fp );
		return( -4 );
	}
	width = InfoHead.ciWidth;
	height = InfoHead.ciHeight;
	LOGI("FileHead.cfSize =%d byte\n",FileHead.cfSize);
	LOGI("flen = %d", flen);	
	LOGI("width = %d, height = %d", width, height);
	total_length = width * height *3;
	
	LOGI("total_length = %d", total_length);
	
	//跳转的数据区FileHead.cfoffBits
	fseek(fp, FileHead.cfoffBits, SEEK_SET);
	LOGI(" FileHead.cfoffBits = %d\n",  FileHead.cfoffBits);
	LOGI(" InfoHead.ciBitCount = %d\n",  InfoHead.ciBitCount);	
	//指向申请的空间位置
	buf = bmp_buf;
	LOGI(" buf = %d\n",sizeof(buf));	
	while ((ret = fread(buf,1,total_length,fp)) >= 0) {
		if (ret == 0) {
			usleep(100);
			continue;
		}
		LOGI("ret = %d\n", ret);
		buf = ((char*) buf) + ret;
		total_length = total_length - ret;
		if(total_length == 0)break;
	}
	
	total_length = width * height * 4;
	LOGI("total_length = %d\n", total_length);
	bmp_buf_dst = (char*)calloc(1,total_length);
	if(bmp_buf_dst == NULL){
		LOGE("load > malloc bmp out of memory!\n");
		return -1;
	}
	
	cursor_bitmpa_format_convert(bmp_buf_dst, bmp_buf);
	//cursor_bitmpa_format_convert_andy(bmp_buf_dst, bmp_buf);
	memcpy(fbp,bmp_buf_dst,total_length);
	
	LOGI("show logo return 0\n");
	return 0;
}
int show_picture(int fd, char *path)
{
	
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	long int screensize = 0;
	struct fb_bitfield red;
	struct fb_bitfield green;
	struct fb_bitfield blue;
	LOGI("Enter show_logo");
	//打开显示设备
	retry1:
	fbfd = fd;//open("/dev/graphics/fb0", O_RDWR);
	LOGI("fbfd = %d", fbfd);
	if (fbfd == -1)
	{
		LOGE("Error opening frame buffer errno=%d (%s)",
             errno, strerror(errno));
		goto retry1;
	}
 
	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo))
	{
		LOGI("Error：reading fixed information.\n");
		return -1;
	}
 
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo))
	{
		LOGI("Error: reading variable information.\n");
		return -1;
	}
 
	LOGI("R:%d,G:%d,B:%d \n", vinfo.red, vinfo.green, vinfo.blue );
 
	LOGI("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel );
	xres = vinfo.xres;
	yres = vinfo.yres;
	bits_per_pixel = vinfo.bits_per_pixel;
 
	//计算屏幕的总大小（字节）
	screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
	LOGI("screensize=%d byte\n",screensize);
 
	//对象映射
	fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	if ((int)fbp == -1)
	{
		printf("Error: failed to map framebuffer device to memory.\n");
		return -1;
	}
 
	LOGI("sizeof file header=%d\n", sizeof(BITMAPFILEHEADER));
 
	
	//显示图像
//	while(1){
	show_bmp(path);
	sleep(10);
	fb_update(&vinfo);
//}
 
	//删除对象映射
	munmap(fbp, screensize);
	//close(fbfd);
	LOGI("Exit show_logo");
	return 0;
}
int main()
{
    int fbfd = 0;

    fbfd = open("/dev/fb0", O_RDWR);  
    if (!fbfd)  
    {  
        printf("Error: cannot open framebuffer device.\n");  
        exit(1);  
    } 
    show_picture(fbfd, "./2.bmp");

    close(fbfd);
}
