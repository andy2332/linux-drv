#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/cdev.h>
#include <linux/fs.h>

struct gpio_led_data{
	const char *desc;
	int gpio;
	int active_high;
};

struct gpio_leds_platform_data
{
	struct gpio_led_data *leds;
	int nleds;
};



struct cdev *dt_test_dev;
dev_t gpio_led_dev_t;
struct class *myclass;
struct gpio_leds_platform_data *pdata;


static struct 	gpio_leds_platform_data *gpio_leds_get_devtree_pdata(struct device *dev)
{
	struct device_node *node,*pp;
	struct gpio_leds_platform_data *pdata;
	struct gpio_led_data * led;
	int nleds;
	int i;

	node = dev->of_node;
	if(!node)
		return ERR_PTR(-ENODEV);

	nleds = of_get_child_count(node);
	if(nleds == 0)
		return ERR_PTR(-ENODEV);

	pdata = devm_kzalloc(dev,sizeof(*pdata) + nleds * sizeof(*led),GFP_KERNEL);
	if(!pdata)
		return ERR_PTR(-ENOMEM);
	pdata->leds = (struct gpio_led_data *)(pdata + 1);
	pdata->nleds = nleds;
	i = 0;
	for_each_child_of_node(node,pp)
	{
		enum of_gpio_flags flags;
		led = (struct gpio_led_data *)&pdata->leds[i++];
		led->desc = of_get_property(pp,"label", NULL);
		led->gpio = of_get_gpio_flags(pp,0,&flags);
		led->active_high = flags & OF_GPIO_ACTIVE_LOW;
		
	}
	
}


static const struct of_device_id ptr[] = 
{
	{.compatible = "gpio=leds"  },
	{},
};



static int gpio_leds_open(struct inode *inode,struct file *file)
{
	printk("<kernel   %s\n>",__FUNCTION__);
	return 0;
}


static const struct file_operations gpio_led_fops = 
{
	.owner =THIS_MODULE,
	.open = gpio_leds_open,
};


static int led_tree_proble(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device *mydevice;
	int ret;
	pdata = gpio_leds_get_devtree_pdata(dev);
	if(IS_ERR(pdata))
		return PTR_ERR(pdata);
	
	dt_test_dev = cdev_alloc();
	if(IS_ERR(dt_test_dev))
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

static int led_tree_remove(struct platform_device *pdev)
{
	device_destroy(myclass,gpio_led_dev_t);
	class_destroy(myclass);
	cdev_del(dt_test_dev);
	unregister_chrdev_region(gpio_led_dev_t,pdata->nleds);

	return 0;
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




