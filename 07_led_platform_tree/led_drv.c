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
#include <linux/of.h>


//硬件资源

static volatile unsigned int *CCM_CCGR1									;		
static volatile unsigned int *IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3	;
static volatile unsigned int *GPIO5_GDIR								;
static volatile unsigned int *GPIO5_DR									;


/* 1. 确定主设备号                                                                 */
static int major = 0;
static char kernel_buf[1024];
static struct class *led_class;

struct dtsled_dev{
	dev_t devid;			/* 设备号 	 */
	struct cdev cdev;		/* cdev 	*/
	struct class *class;		/* 类 		*/
	struct device *device;	/* 设备 	 */
	int major;				/* 主设备号	  */
	int minor;				/* 次设备号   */
	struct device_node	*nd; /* 设备节点 */
};

struct dtsled_dev dtsled;	/* led设备 */



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

/* 4. 把file_operations结构体告诉内核：注册驱动程序                                */
/* 5. 谁来注册驱动程序啊？得有一个入口函数：安装驱动程序时，就会去调用这个入口函数 */
static int __init led_init(void)
{
	int err;
	unsigned int val;
	u32 regdata[14];
	struct property *proper;
	const char *str;

	//获取设备节点
	dtsled.nd = of_find_node_by_path("/100ask_led");
	if(dtsled.nd == NULL)
	{
		printk("seedling_led node not find\r\n");
		return -EINVAL;
	}
	else
	{
		printk("seedling_led node find \r\n");
	}
	//2.获取compatible属性内容
	proper = of_find_property(dtsled.nd,"compatible",NULL);
	if(proper == NULL)
	{
		printk("compatible proper find failed \r\n");
	}
	else
	{
		printk("compatible = %s \r\n",(char *)proper->value);
	}
	//3.获取status的属性值
	err = of_property_read_string(dtsled.nd,"status",&str);
	if(err < 0)
	{
		printk("status read failed \r\n");
	}
	else 
	{
		printk("status = %s\r\n",str);
	}

	//4.获取reg内容
	err = of_property_read_u32_array(dtsled.nd,"reg", regdata,10);
	if(err < 0)
	{
		printk("reg property read failed \r\n");
	}
	else
	{
		u8 i =0 ;
		printk("reg data:\r\n");
		for(i = 0;i < 10; i++)
			printk("%#X",regdata[i]);
		printk(" \r\n");
	}
	
	
	
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


	/*
	//初始化硬件资源
	CCM_CCGR1 								= ioremap(0x20C406C, 4);
	IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 = ioremap(0x2290014,4);
	GPIO5_GDIR								= ioremap(0x020AC000 + 0x4,4);
	GPIO5_DR 								= ioremap(0x020AC000+0, 4);
	*/

	CCM_CCGR1 								= ioremap(regdata[0], regdata[1]);
	IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 = ioremap(regdata[2],regdata[3]);
	GPIO5_GDIR								= ioremap(regdata[8],regdata[9]);
	GPIO5_DR 								= ioremap(regdata[6], regdata[7]);

	//使能GPIO5_3
	*CCM_CCGR1 |= (3<<30);


	//设置GPIO5_3为GPIO模式
	val = *IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3;
	val &= ~(0xf);
	val |=(5);
	*IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 = val;

	//将GPIO5_3设置为output输出
	*GPIO5_GDIR |= (1<<3);
	
	
	return 0;
}

/* 6. 有入口函数就应该有出口函数：卸载驱动程序时，就会去调用这个出口函数           */
static void __exit led_exit(void)
{

	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	iounmap(CCM_CCGR1);
	iounmap(IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3);
	iounmap(GPIO5_GDIR);
	iounmap(GPIO5_DR);
	device_destroy(led_class, MKDEV(major, 0));
	class_destroy(led_class);
	unregister_chrdev(major, "andy_led");
}


/* 7. 其他完善：提供设备信息，自动创建设备节点                                     */

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");


