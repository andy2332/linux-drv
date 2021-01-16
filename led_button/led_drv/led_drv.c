
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include <linux/cdev.h>
#include <linux/device.h>

static dev_t dev_num = 0;
static struct cdev *cdevice = NULL;
static struct class *sys_class = NULL;
static struct device *class_device = NULL;

static volatile unsigned int *gpio_data;

int led_open (struct inode *pinode , struct file *pfile)
{
	printk(KERN_INFO " [%s]\n" ,__func__);

	return 0;
}


ssize_t led_read(struct file *pfile, char __user *userbuf , size_t size, loff_t *loff)
{
	printk(KERN_INFO " [%s]\n", __func__);

	return 0;
}


ssize_t led_write(struct file *pfile, const char __user *userbuf, size_t size, loff_t *loff)
{
	int ret = -1;
	int val = -1;
	printk(KERN_INFO " [%s]\n" ,__func__);

	ret = copy_from_user(&val,userbuf, size);
	
	if(ret == 0 && val == 1)
	{
		printk("led_on : [%s] 1\n",__func__);
		//开灯
	}
	else
	{
		printk("led_off : [%s] 0\n",__func__);
		//关灯
	}

	return size;

}



static struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.open  = led_open,
	.write = led_write,
	.read  = led_read,

};

int led_probe(struct platform_device *pdev)
{

	struct resource *res = NULL;
	unsigned int led_pin;
	res = platform_get_resource(pdev,IORESOURCE_MEM,0);
	if(res){
		led_pin = res->start;
	}
	else{
		of_property_read_u8(pdev->dev.of_node,"reg",(u8 *)&led_pin);	
	}
	printk("led_pin = %d\n",led_pin);
	if(!led_pin){
		return -EINVAL;
	}
	gpio_data = (int *)ioremap(led_pin,1);

	printk(KERN_INFO "led_probe found led %s\n",__func__);
	cdev_init(cdevice,&led_fops);

	alloc_chrdev_region(&dev_num, 0, 1, "yangbkLed");
	cdev_add(cdevice,dev_num,1);

	sys_class = class_create(THIS_MODULE, "yangbkLed");
	class_device = device_create(sys_class, NULL, dev_num, NULL, "yangbkDevice");
	return 0 ;
	
}


int led_remove(struct platform_device *pdev)
{
	printk(KERN_INFO "led remove, remove led %s\n",__func__);

	device_destroy(sys_class,dev_num);
	class_destroy(sys_class);

	unregister_chrdev_region(dev_num, 1);
	cdev_del(cdevice);
	iounmap(gpio_data);
	return 0;
}

static struct of_device_id of_match_table={
	.compatible = "ybk_led",
	.data = NULL,
};

struct platform_driver  led_drv={
	.probe = led_probe,
	.remove = led_remove,
	.driver = {
		.name = "idle",
		.of_match_table = &of_match_table,
	}
};

static int __init led_drv_init(void)
{
	printk(KERN_INFO "led_drv init %s\n",__func__);
	platform_driver_register(&led_drv);
	return 0 ;
}
static void __exit led_drv_exit(void)
{
	printk(KERN_INFO "led_drv exit %s \n" ,__func__);
	platform_driver_unregister(&led_drv);
}

module_init(led_drv_init);
module_exit(led_drv_exit);

MODULE_AUTHOR("yangbkGIT");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("plantform driver module");
MODULE_ALIAS("module");




