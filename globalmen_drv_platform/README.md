使用make 进行编译  
编译完成后将    *.ko 文件上传到开发板 根文件系统中
我这里使用的是100ask_imx6ull 开发板  vm使用桥接搭建的ubunru 使用命令: mount -t nfs -o nolock,vers=3 192.168.8.111:/opt/nfs /mnt 
	其中/mnt为开发板路径

上传后 使用 insmod *.ko 挂在模块 
使用cat /proc/devices 查看 发现多出了主设备号为230的“globalmem”字符设备驱动。

2021-1-13
 基于平台总线搭建测试代码

