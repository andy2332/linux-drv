#include <linux/init.h>
#include <linux/module.h> //模块
#include <linux/platform_device.h>//平台设备头文件
#include <asm/io.h> //ioremap

static int led_drv_probe(struct platform_device *pdev)
{
		
}

static int led_drv_remove(struct platform_device *pdev)
{

}

struct platform_driver led_drv = {
		.probe	= led_drv_probe,
			.remove = led_drv_remove,
				.driver = {
							.name = "myled",
									},
};


static int __int led_plat_drv_init(void)
{
		printk("<kernel> call %s\n",__FUNCTION__);

			return platform_driver_register(led_drv)
}
module_init(led_plat_drv_init);

static void __exit led_plat_drv_exit(void)
{

}
module_exit(led_plat_drv_exit);

MODULE_LICENSE("GPL");




