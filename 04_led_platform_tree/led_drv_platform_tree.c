#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/cdev.h>
#include <linux/fs.h>

struct cdev *dt_test_dev;
dev_t gpio_led_dev_t;
struct class *myclass;


struct 	gpio_leds_platform_data *pdata;


static const struct of_device_id ptr[] = 
{
	{.compatible = "gpio=leds"  },
	{},
};


struct gpio_leds_platform_data{
	struct gpio_led_data *leds;
	int nleds;
};



static const struct file_operations gpio_led_fops = 
{
	.owner =THIS_MODULE,
	.oen
};

static int led_tree_proble(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device *mydevice;
	int ret;
	pdata = gpio_led_get_devtree_pdata(dev);
	if(IS_ERR(pdata))
		return PTR_ERR(pdata);
	
	dt_test_dev = cdev_alloc();
	if(IS_ERR(dt_test_dev())
		return PTR_ERR(dt_test_dev);
	cdev_init(dt_test_dev, &gpio_led_fops);
	ret = alloc_chrdev_region(gpio_led_dev_t,0,pdata->nleds,"gpio-leds");
	if(ret)
		goto fail3;
	ret = cdev_add(dt_test_dev,gpio_led_dev_t,pdata->nleds);
	if(ret)
		goto fail2;
	myclass = class_create(THIS_MODULE,"gpio_leds");
	if(IS_ERR(myclass))
		goto fail1;
	mydevice  = device_create(myclass,NULL,gpio_led_dev_t,NULL,"gpio_leds");
	if(IS_ERR(myclass))
		goto fail;
	return 0;

fail:
	class_destroy(myclass);
fail1:
	cdev_del(dt_test_dev);
fail2:
	unregister_chrdev_region(gpio_led_dev_t,pdata->nleds);
fail3:
	return -1;
}

static struct platform_driver drv = 
{
	.probe = led_tree_proble,
	.remove = led_tree_remove,
	.driver = 
	{
		.name = "led_tree",
		.of_match_table = of_match_ptr(ptr),
	},

};

static int __init led_tree_init(void)
{
	return platform_driver_register(&drv);	
}

static void __init led_tree_exit(void)
{
	platform_driver_unregister(&drv);
}

module_init(led_tree_init);
module_exit(led_tree_exit);
MODULE_LICENSE("GPL");




