#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>//平台设备头文件
#include <linux/ioport.h>

#define LED_CCM_CCGR	0x20C406C
#define LED_IOMUXC		0x2290014
#define LED_GPIO5_GDIR	0x020AC0000 + 0x4
#define LED_GPIO5_DR	0x020AC0000 + 0
//硬件资源

//static volatile unsigned int *CCM_CCGR1									;		
//static volatile unsigned int *IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3	;
//static volatile unsigned int *GPIO5_GDIR								;
//static volatile unsigned int *GPIO5_DR									;


static void led_dev_release(struct device *dev)
{
	printk("<kernel>  %s()\n",__FUNCTION__);
}

struct resource led_res[]=
{
	[0] = 
	{
		.start	 = LED_CCM_CCGR,
		.end	 = LED_CCM_CCGR + 0x4,
		.flags	 = IORESOURCE_MEM,
	},
	[1] = 
	{
		.start	= LED_IOMUXC,
		.end	 = LED_IOMUXC + 0x4,
		.flags	= IORESOURCE_MEM,
	},
	[2] = 
	{
		.start  = LED_GPIO5_GDIR,
		.end	 = LED_GPIO5_GDIR + 0x4,
		.flags  = IORESOURCE_MEM,
	},
	[3] = 
	{
		.start  = LED_GPIO5_DR,
		.end	 = LED_GPIO5_DR + 0x4-1,
		.flags  = IORESOURCE_MEM,
	},
};

struct platform_device  led_dev = 
{
	.name			=	"imx_led",
	.id				=	-1,
	.num_resources	=	ARRAY_SIZE(led_res),
	.resource		=	led_res,
	.dev			=	
		{
			.release = led_dev_release, 
		},
};

static int __init led_init(void)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return platform_device_register(&led_dev);
}

static void __exit led_exit(void)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	platform_device_unregister(&led_dev);
}

/* 7. 其他完善：提供设备信息，自动创建设备节点                                     */

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
