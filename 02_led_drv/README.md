# 基于imx6ull 最基础的设备驱动，直接操作寄存器

**硬件操作**
初始化硬件：

     CCM_CCGR1                               = ioremap(0x20C406C, 4);
     IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 = ioremap(0x2290014,4);
     GPIO5_GDIR                              = ioremap(0x020AC000 + 0x4,4);
     GPIO5_DR                                = ioremap(0x020AC000+0, 4);

参考imx6ull 的 datasheet 
	参考如下图:
	《截图待补充》
####1.使能GPIO5_3
 
	*CCM_CCGR1 |= (3<<30);
####2.设置GPIO5_3为GPIO模式

	val &= ~(0xf);
	val |=(5);*
	IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 = val;

####3.将GPIO5_3设置为output输出

	*GPIO5_GDIR |= (1<<3)

####4.输出高电平/低电平

	*GPIO5_DR &= ~(1<<3); //低电平
	*GPIO5_DR |= (1<<3); //高电平

####5.参考原理图
![avatar](https://github.com/andy2332/linux-drv/blob/main/02_led_drv/GPIO5_3.png)

