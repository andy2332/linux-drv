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
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/interrupt.h>

//等待队列的头文件
#include<linux/wait.h>
#include<linux/sched.h>


static int major = 0;
static struct class *led_class;



struct device_node * key_node;
unsigned int  gpio_num;
int my_irq;

int value = 0;

int wq_flags = 0;
//使用宏定义等待队列并初始化
DECLARE_WAIT_QUEUE_HEAD(key_wq);



//中断函数
static irqreturn_t test_key(int irq,void *args)
{
	printk( "test  key\n");

	value = !value;

	//唤醒等待队列前 ，需要将等待队列标志位更新为1 否则唤醒后也不能继续向下执行。
	wq_flags = 1;
	wake_up(&key_wq);

	return IRQ_HANDLED;
	
}


/* 3. 实现对应的open/read/write等函数，填入file_operations结构体                   */
static ssize_t led_drv_read (struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	printk(KERN_INFO "%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

	//使用等待队列       wq_flags 为标志位当 wq_flags为1 是才能向下执行
	wait_event_interruptible(key_wq, wq_flags);

	if(copy_to_user(buf,&value, sizeof(value)) != 0)
	{
		printk(KERN_INFO"copy_to_user err\n");
		return -1;
	}


	wq_flags = 0;
	return 0;
}

static ssize_t led_drv_write (struct file *file, const char __user *buf, size_t size, loff_t *offset)
{

	 printk(KERN_INFO "%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	 return 0;
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
	int err;

	printk(KERN_INFO "%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

	key_node = of_find_node_by_path("/100ask_key");
	if(key_node == NULL)
	{
		printk(KERN_INFO "of_find_node_by_path \n");
		return -1;
	}
	gpio_num = of_get_named_gpio(key_node,"gpios", 0);
	if(gpio_num < 0)
	{
		printk(KERN_INFO "err of_get_named_gpio\n");
		return -1;
	}
	//将gpio_num设置为输入模式
	err = gpio_direction_input(gpio_num);
	//获得gpio的中断号
	my_irq = gpio_to_irq(gpio_num);
	//
	err = request_irq(my_irq,test_key,IRQF_TRIGGER_RISING,"test_key",NULL);


	led_register();

	return 0;
}

static int led_drv_remove(struct platform_device *pdev)
{
	
	printk(KERN_INFO "%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	device_destroy(led_class, MKDEV(major, 0));
	class_destroy(led_class);
	unregister_chrdev(major, "andy_led");
	free_irq(my_irq,NULL);
	return 0;
}

static const struct of_device_id led_id_table[] = {
	{ .compatible = "100ask_key,key" },
	{ },
};

struct platform_driver pdrv = {	
	.probe = led_drv_probe,
	.remove = led_drv_remove,
	.driver = {
		.name = "mykey",
		.of_match_table = led_id_table,
	},

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

