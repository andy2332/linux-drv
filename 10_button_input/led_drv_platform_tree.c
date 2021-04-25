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
#include <linux/interrupt.h>



#define MIN(a, b) (a < b ? a : b)



struct device_node * key_node;
unsigned int  gpio_num;
int irq;



//中断函数
static irqreturn_t test_key(int irq,void *args)
{
	printk(KERN_INFO "test  key\n");
	return IRQ_HANDLED;
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
	irq = gpio_to_irq(gpio_num);
	//
	request_irq(irq,test_key,IRQF_TRIGGER_RISING,"test_key",NULL);
	//获取硬件资源
	return 0;
}

static int led_drv_remove(struct platform_device *pdev)
{
	
	printk(KERN_INFO "%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	free_irq(irq,NULL);
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

