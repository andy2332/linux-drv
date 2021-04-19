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
#include <linux/gpio.h>
#include <linux/of_gpio.h>





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
	int led_gpio;			  /* led所使用的GPIO编号*/
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
	struct dtsled_dev *dev;
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	err = copy_from_user(kernel_buf, buf, MIN(1024, size));
	dev = file->private_data;

	if(kernel_buf[0] == '1')
	{
		gpio_set_value(dev->led_gpio, 0);	/* 打开LED灯 */
		printk("%s %s  open led %d\n", __FILE__, __FUNCTION__, __LINE__);
	}
	else
	{
		gpio_set_value(dev->led_gpio, 1);	/* 关闭LED灯 */
		printk("%s %s close led %d\n", __FILE__, __FUNCTION__, __LINE__);
	}

	return MIN(1024, size);
}

static int led_drv_open (struct inode *node, struct file *file)
{

	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	file->private_data = &dtsled; /* 设置私有数据 */
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
	//2.获取led-gpio属性内容
	dtsled.led_gpio = of_get_named_gpio(dtsled.nd, "led-gpio", 0);
	if(dtsled.led_gpio < 0)
	{
		printk("of_get_named_gpio err\r\n");
		return -EINVAL;
	}
	else
	{
		printk("led-gpio num = %d\r\n",dtsled.led_gpio);
	}

	err = gpio_direction_output(dtsled.led_gpio, 1);
	if(err < 0)
	{
		printk("can't set gpio \r\n");
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



	return 0;
}

/* 6. 有入口函数就应该有出口函数：卸载驱动程序时，就会去调用这个出口函数           */
static void __exit led_exit(void)
{

	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	device_destroy(led_class, MKDEV(major, 0));
	class_destroy(led_class);
	unregister_chrdev(major, "andy_led");
}


/* 7. 其他完善：提供设备信息，自动创建设备节点                                     */

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");


