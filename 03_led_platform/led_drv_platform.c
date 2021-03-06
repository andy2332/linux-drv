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

//硬件资源

static volatile unsigned int *CCM_CCGR									;		
static volatile unsigned int *IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3	;
static volatile unsigned int *GPIO5_GDIR								;
static volatile unsigned int *GPIO5_DR									;



/* 1. 确定主设备号                                                                 */
static int major = 0;
static char kernel_buf[1024];
static struct class *led_class;

//开灯

#define MIN(a, b) (a < b ? a : b)

/* 3. 实现对应的open/read/write等函数，填入file_operations结构体                   */
static ssize_t led_drv_read (struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	int err;
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	err = copy_to_user(buf, kernel_buf, MIN(1024, size));
	return MIN(1024, size);
}

static ssize_t led_drv_write (struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
	int err;
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	err = copy_from_user(kernel_buf, buf, MIN(1024, size));

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

	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

static int led_drv_close (struct inode *node, struct file *file)
{

	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
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

//获取平台资源
static int led_drv_probe(struct platform_device *pdev)
{
	int err;
	unsigned int val;
	struct resource *addr_res;
	addr_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	printk("driver addr 0x%x/n",addr_res->start);
	//已经映射到本地资源
	CCM_CCGR = ioremap(addr_res->start,resource_size(addr_res));
	
	addr_res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	printk("driver addr 0x%x/n",addr_res->start);
	//已经映射到本地资源
	IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 = ioremap(addr_res->start,resource_size(addr_res));
	
	addr_res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	printk("driver addr 0x%x/n",addr_res->start);
	//已经映射到本地资源
	GPIO5_GDIR = ioremap(addr_res->start,resource_size(addr_res));	

	addr_res = platform_get_resource(pdev, IORESOURCE_MEM, 3);
	printk("driver addr 0x%x/n",addr_res->start);
	//已经映射到本地资源
	GPIO5_DR = ioremap(addr_res->start,resource_size(addr_res));

	/*与基础版设置方法相同
	  将GPIO5_3使能
	  将GPIO5_3设置为GPIO（通用输入输出）
	  将GPIO5_3设置为输出模式
	  */

#if 1

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



	//使能GPIO5_3
	*CCM_CCGR |= (3<<30);


	//设置GPIO5_3为GPIO输出
	val = *IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3;
	val &= ~(0xf);
	val |=(5);
	*IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 = val;

	//将GPIO5_3设置为output输出
	*GPIO5_GDIR |= (1<<3);


	return 0;
#endif

}

static int led_drv_remove(struct platform_device *pdev)
{
	iounmap(GPIO5_GDIR);
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	device_destroy(led_class, MKDEV(major, 0));
	class_destroy(led_class);
	unregister_chrdev(major, "andy_led");
	return 0;
}



struct platform_driver drv = {	
	.probe = led_drv_probe,
	.remove = led_drv_remove,
	.driver = {
		.name = "myled",	
	},

};



/* 4. 把file_operations结构体告诉内核：注册驱动程序                                */
/* 5. 谁来注册驱动程序啊？得有一个入口函数：安装驱动程序时，就会去调用这个入口函数 */
static int __init led_init(void)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return platform_driver_register(&drv);

}

/* 6. 有入口函数就应该有出口函数：卸载驱动程序时，就会去调用这个出口函数           */
static void __exit led_exit(void)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);


}


/* 7. 其他完善：提供设备信息，自动创建设备节点                                     */

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");


