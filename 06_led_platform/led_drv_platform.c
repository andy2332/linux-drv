#include <linux/module.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include <linux/io.h>
#include <linux/platform_device.h>//平台设备头文件


#define MIN(a, b) (a < b ? a : b)


static int major = 0;
static struct class *led_class;
struct resource *res[4];
static char kernel_buf[1024];
static volatile unsigned int *CCM_CCGR1									;		
static volatile unsigned int *IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3	;
static volatile unsigned int *GPIO5_GDIR								;
static volatile unsigned int *GPIO5_DR									;



/* 3. 实现对应的open/read/write等函数，填入file_operations结构体                   */
static ssize_t led_drv_read (struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	return 0;
}

static ssize_t led_drv_write (struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
	int err;
	 printk(KERN_INFO "%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	 err = copy_from_user(kernel_buf, buf, MIN(1024, size));
	 printk(KERN_INFO " %c\n",kernel_buf[0]);
	 if(kernel_buf[0] == '1')
	 {
		 *GPIO5_DR &= ~(1<<3);
	 }
	 else
	 {
		 *GPIO5_DR |= (1<<3);
	 }
	 
	
	 return MIN(1024, size);
}

static int led_drv_open (struct inode *node, struct file *file)
{

	printk(KERN_INFO "%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	
	return 0;
}

static int led_drv_close (struct inode *node, struct file *file)
{

	printk(KERN_INFO "%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

/* 2. 定义自己的file_operations结构体                                              */
static struct file_operations led_drv = {
	.owner	 = THIS_MODULE,
	.open    = led_drv_open,
	.read    = led_drv_read,
	.write   = led_drv_write,
	.release = led_drv_close,
};


static int led_register(void)
{
	int err;	
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	major = register_chrdev(0, "andy_led", &led_drv);  /* /dev/andy_led */
	
	led_class = class_create(THIS_MODULE, "led_class");
	
	err = PTR_ERR(led_class);
	if (IS_ERR(led_class)) {
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		unregister_chrdev(major, "andy_led");
		return -1;
	}
	device_create(led_class, NULL, MKDEV(major, 0), NULL, "andy_led"); /* /dev/andy_led */
	return 0;

	
}

//获取平台资源
static int led_drv_probe(struct platform_device *pdev)
{
	int val ;

	led_register();
	//获取硬件资源
	res[0] = platform_get_resource(pdev,IORESOURCE_MEM, 0);
	res[1] = platform_get_resource(pdev,IORESOURCE_MEM, 1);
	res[2] = platform_get_resource(pdev,IORESOURCE_MEM, 2);
	res[3] = platform_get_resource(pdev,IORESOURCE_MEM, 3);
	printk(KERN_INFO "%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	printk(KERN_INFO "%x ,%d\n",res[0]->start,resource_size(res[0]));
	printk(KERN_INFO "%x ,%d\n",res[1]->start,resource_size(res[1]));
	printk(KERN_INFO "%x ,%d\n",res[2]->start,resource_size(res[2]));
	printk(KERN_INFO "%x ,%d\n",res[3]->start,resource_size(res[3]));

	//对硬件资源进行操作

	CCM_CCGR1 								= 	ioremap(res[0]->start,resource_size(res[0]));
	IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 =	ioremap(res[1]->start,resource_size(res[1]));
	GPIO5_GDIR  							=	ioremap(res[2]->start,resource_size(res[2]));
	GPIO5_DR								=	ioremap(res[3]->start,resource_size(res[3]));
	
	/*

	CCM_CCGR1 								= 	ioremap(0x20C406C,resource_size(res[0]));
	IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 =	ioremap(0x2290014,resource_size(res[1]));
	GPIO5_GDIR  							=	ioremap(0x020AC000+0x4,resource_size(res[2]));
	GPIO5_DR								=	ioremap(0x020AC000,resource_size(res[3]));
	


	CCM_CCGR1 								= ioremap(0x20C406C, 4);
	IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 = ioremap(0x2290014,4);
	GPIO5_GDIR								= ioremap(0x020AC000 + 0x4,4);
	GPIO5_DR 								= ioremap(0x020AC000+0, 4);
*/

	printk(KERN_INFO "biaozhi 1\n");



    //使能GPIO5_3
    (*CCM_CCGR1) |= (3<<30);

	printk(KERN_INFO "biaozhi 2\n");


    //设置GPIO5_3为GPIO模式
    val = (*IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3);
    val &= ~(0xf);
    val |=(5);
    (*IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3) = val;
	printk(KERN_INFO "biaozhi 3\n");

    //将GPIO5_3设置为output输出
    (*GPIO5_GDIR) |= (1<<3);

	printk(KERN_INFO "biaozhi 4\n");

	
	return 0;
}

static int led_drv_remove(struct platform_device *pdev)
{
	
	printk(KERN_INFO "%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	//device_destroy(struct class * class, dev_t devt)(led_class, dev_t devt);
	
	return 0;
}

const struct platform_device_id led_id_table[]={
	{"imx_led",0x444},
};

struct platform_driver pdrv = {	
	.probe = led_drv_probe,
	.remove = led_drv_remove,
	.driver = {
		.name = "myled",	
	},
	.id_table = led_id_table,
};

/* 4. 把file_operations结构体告诉内核：注册驱动程序                                */
/* 5. 谁来注册驱动程序啊？得有一个入口函数：安装驱动程序时，就会去调用这个入口函数 */
static int __init led_init(void)
{
	printk(KERN_INFO "%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	platform_driver_register(&pdrv);
	return 0;
}

/* 6. 有入口函数就应该有出口函数：卸载驱动程序时，就会去调用这个出口函数           */
static void __exit led_exit(void)
{
	printk(KERN_INFO "%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	platform_driver_unregister(&pdrv);
}

/* 7. 其他完善：提供设备信息，自动创建设备节点                                     */
module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");

