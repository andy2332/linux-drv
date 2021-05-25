#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h> 
#include <linux/fs.h> 
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/i2c.h>


struct at24c02_e2prom_dev{
	int major;
	struct class *cls;
	struct device *dev;
	struct i2c_client *client; //方便读写e2prom
};

static struct at24c02_e2prom_dev *at24c02_dev;

static int i2c_read_bytes(const struct i2c_client *client,char *buf,int count)
{
	int ret;
	struct i2c_adapter *adap;
	struct i2c_msg msg;
	printk("<kernel> call %s() \n",__FUNCTION__);
	adap = client->adapter;

	msg.addr = client->addr;
	msg.flags |= I2C_M_RD;
	msg.len = count;
	msg.buf = buf;
	ret = i2c_transfer(adap,&msg,1);
	printk("<kernel> call %s() end\n",__FUNCTION__);
	
	return (ret == 1)?count:ret;
}

static ssize_t at24c02_read (struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
	int ret;
	char * tmp;
	printk("<kernel> call %s()\n",__FUNCTION__);
	tmp = kzalloc(size,GFP_KERNEL);
	if(tmp == NULL)
	{
		printk("err kzalloc\n");
		return -ENOMEM;
	}
	ret = i2c_read_bytes(at24c02_dev->client,tmp,size);
	if(ret < 0)
	{
		printk("err i2c_read_bytes\n");
		ret = -EINVAL;
		goto err_i2c_read_bytes;
	}

	ret = copy_to_user(buf,tmp,size);
	if(ret > 0)
	{
		printk("err copy_to_user\n");
		goto err_i2c_read_bytes;
	}
	printk("<kernel> call %s() end\n",__FUNCTION__);
	return size;
err_i2c_read_bytes:
	kfree(tmp);
	return ret;
}


//向 i2c中写字节
static int i2c_write_bytes(const struct i2c_client *client,char *buf,int count)
{
	int ret;
	
	struct i2c_adapter *adap; //从clint中获得
	struct i2c_msg msg; //自己进行打包
	printk("<kernel> call %s() \n",__FUNCTION__);
	adap = client->adapter;
	msg.addr = client->addr;
	msg.flags = 0; //写标志位为0
	msg.len = count;
	msg.buf = buf;

	/*
		i2c设备收发数据  
		int i2c_transfer(struct i2c_adapter * adap, struct i2c_msg * msgs, int num)
		参数1 适配器
		参数2 数据包
		参数3 数据包个数
	*/
	ret = i2c_transfer(adap,&msg,1);
	printk("<kernel> call %s() end\n",__FUNCTION__);
	return (ret == 1)? count : ret; 
}

static ssize_t at24c02_write (struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
	int ret;
	char *tmp;
	printk("<kernel> call %s()\n",__FUNCTION__);
	
	tmp = kzalloc(size,GFP_KERNEL);
	if(tmp == NULL)
	{
		printk("err kzalloc\n");
		return -ENOMEM;
	}

	ret = copy_from_user(tmp,buf,size);
	if(ret < 0)
	{
		printk("err copy_from_user\n");
		goto err_copy_from_user;
	}

	//将数据写入从设备e2prom中
	ret = i2c_write_bytes(at24c02_dev->client,tmp,size);

	if(ret < 0)
	{
		printk("err i2c_write_bytes\n");
		ret = -EINVAL;
		goto err_copy_from_user;
	}

	
	printk("<kernel> call %s() end\n",__FUNCTION__);
	return size;

err_copy_from_user:
	kfree(tmp);
	return ret;
}


static int at24c02_open (struct inode *inode, struct file *filp)
{
	printk("<kernel> call %s()\n",__FUNCTION__);
	return 0;
}

static int at24c02_close(struct inode *inode, struct file *filp)
{
	printk("<kernel> call %s()\n",__FUNCTION__);
	return 0;
}





const struct file_operations  at24c02_fops = {
	.owner 		= THIS_MODULE,
	.open 		= at24c02_open,
	.release 	= at24c02_close,
	.read 		= at24c02_read,
	.write		= at24c02_write,
};



struct of_device_id	at24c02_id[] = {
	{.compatible = "at24c02",0,},
	{}
};

struct i2c_device_id at24c02_id_ts[] = {
	{"xxx",0},
	{}
};


int at24c02_probe(struct i2c_client *i2c_client, const struct i2c_device_id *i2c_device_id)
{
	int ret;
	printk("at24c02_probe\n");
	/*注册杂项设备或字符设备*/

	//实例化结构体，在结构体中包含：字符设备信息和i2c设备信息 
	at24c02_dev = kzalloc(sizeof(struct at24c02_e2prom_dev),GFP_KERNEL);
	if(!at24c02_dev)
	{
		printk("err kzalloc\n");
		return -ENOMEM;
	}
	//申请主设备号
	at24c02_dev->major = register_chrdev(0,"at24c02",&at24c02_fops);
	if(at24c02_dev->major < 0)
	{
		printk("err register_chrdev\n");
		ret = -EBUSY;
		goto err_register_chrdev;
	}

	//创建设备节点
	at24c02_dev->cls = class_create(THIS_MODULE,"at24c02");
	if(IS_ERR(at24c02_dev->cls))
	{
		printk("err class_create\n");
		ret = -EBUSY;
		goto err_class_create;
	}

	at24c02_dev->dev = device_create(at24c02_dev->cls,NULL,MKDEV(at24c02_dev->major, 0),NULL,"i2c_e2prom");
	if(IS_ERR(at24c02_dev->cls))
	{
		printk("err device_create\n");
		ret = PTR_ERR(at24c02_dev->dev);
		goto err_device_create;
	}
		
	//获取i2c适配器资源
	at24c02_dev->client = i2c_client;

	return 0;
err_register_chrdev:
	kfree(at24c02_dev);
err_class_create:
	unregister_chrdev(at24c02_dev->major,"at24c02");
err_device_create:		
	class_destroy(at24c02_dev->cls);

	return ret;




}
int at24c02_remove(struct i2c_client *i2c_client)
{
	printk("at24c02_remove\n");
	
	device_destroy(at24c02_dev->cls,MKDEV(at24c02_dev->major, 0));

	
	class_destroy(at24c02_dev->cls);


	unregister_chrdev(at24c02_dev->major,"at24c02");

	kfree(at24c02_dev);
	
	return 0;
}


struct i2c_driver   at24c02_driver = {

	.driver = {
		.owner = THIS_MODULE,
		.name = "at24c02",
		.of_match_table = at24c02_id,
	},
	.probe = at24c02_probe,
	.remove = at24c02_remove,

	.id_table = at24c02_id_ts,
};

/*
static int at24c02_driver_init(void)
{

	int ret;
	printk("at24c02_driver_init\n");

	//	注册到内核中
	ret = i2c_add_driver(&at24c02_driver);

	if(ret < 0)
	{
		printk("err i2c_add_driver\n");
		return ret;
	}
	return 0;
}
static void at24c02_driver_exit(void)
{

	printk("at24c02_driver_exit\n");
	i2c_del_driver(&at24c02_driver);

}

module_init(at24c02_driver_init);
module_exit(at24c02_driver_exit);
*/

module_i2c_driver(at24c02_driver);



MODULE_LICENSE("GPL");



