#include <linux/busfreq-imx.h>
#include <linux/console.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/pm_qos.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/pinctrl/consumer.h>
#include <linux/fb.h>
#include <linux/mxcfb.h>
#include <linux/regulator/consumer.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <video/of_display_timing.h>
#include <video/videomode.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
 
static  struct fb_info *my_fb_info;

static struct fb_ops myfb_ops = {
	.owner		= THIS_MODULE,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};


static int __init lcd_imx6ull_mod_init(void)
{

	dma_addr_t phy_addr;
	//1.分配fb_info
	my_fb_info = framebuffer_alloc(0,NULL);

	//2.设置fb_info结构体
		//a. var lcd的分辨率 颜色格式
	my_fb_info->var.xres = 1024;
	my_fb_info->var.yres = 600;
	my_fb_info->var.bits_per_pixel = 16 //rgb565
	my_fb_info->var.red.offset = 15;
	my_fb_info->var.red.length = 5;

	my_fb_info->var.green.offset = 5;
	my_fb_info->var.green.length = 6;

	my_fb_info->var.blue.offset = 0;
	my_fb_info->var.blue.length = 5;

	
		//b.fix
	//在内存中申请一块内存作为显存

	my_fb_info->fix.smem_len = my_fb_info->var.xres * my_fb_info->var.yres * my_fb_info->var.bits_per_pixel / 8
	if(my_fb_info->var.bits_per_pixel == 24 )
		{
			my_fb_info->fix.smem_len = my_fb_info->var.xres * my_fb_info->var.yres *4;
		}

	//dma_alloc_wc 作为为将虚拟地址转化物理地址，并将地址保存在第三个参数中
	my_fb_info->screen_base = dma_alloc_wc(NULL, my_fb_info->fix.smem_len, &phy_addr,GFP_KERNEL);//显存的虚拟地址
	my_fb_info->fix.smem_start = phy_addr;//显存的物理地址
	my_fb_info->fix.type  = FB_TYPE_INTERLEAVED_PLANES;
	my_fb_info->fix.visual = FB_VISUAL_TRUECOLOR;
		
	

		//c.
	my_fb_info->fbops = &myfb_ops;

	//3.注册结构体
	register_framebuffer(my_fb_info);
	return 0;
}
module_init(lcd_imx6ull_mod_init);

static void __exit lcd_imx6ull_mod_exit(void)
{
	//注销fb_info
	unregister_framebuffer(my_fb_info);
	
	//释放fb_info
	framebuffer_release(my_fb_info);
}
module_exit(lcd_imx6ull_mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andy2333");



