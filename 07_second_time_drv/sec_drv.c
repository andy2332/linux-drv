#include <linux/module.h>
#include <lincx/fs.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define SECOND_MAJOR 248
static int second_major = SECOND_MAJOR;

struct second_dev
{
	struct cdev cdev;
	atomic_t counter;
	struct timer_list s_timer;
};
static struct second_dev *second_devp;

static int second_open(struct inode *inode,struct file *filep)
{
	init_timer(&second_)
}

static const struct file_operations second_fops = 
{
	.owner 		= 	THIS_MODULE,
	.open 		= 	second_open,
	.release 	=	second_close,
	.read		=	second_read,
};

static void second_setup_cdev(struct second_dev *dev,int index)
{
	int err,devno = MKDEV(second_major,index);

	cdev_init(&dev->cdev,&second_fops);
	dev->cdev.owner = THIS_MODULE;
	err = cdev_add(&dev->cdev,devno,1);
	if(err)
		printk(KERN_ERR"Failed to add second device\n");
}


static int __init senond_init(void)
{
	int ret;
	dev_t devno = MKDEV(second_major,0);

	if (second_major)
		ret = register_chrdev_region(devno, 1,"second");
	else
	{
		ret = alloc_chrdev_region(&devno,0, 1, "second");
		second_major = MAJOR(devno);
	}
	if(ret < 0)
	{	
		return ret;
	}
	second_devp = kzalloc(sizeof(*second_devp),GFP_KERNEL);
	if(!second_devp)
	{
		ret = -ENOMEM;
		goto fail_malloc;
	}
	second_setup_cdev(second_devp,0);

	return 0;
	
fail_malloc:
	unregister_chrdev_region(devno,1);

	return ret;

}

/* 6. 有入口函数就应该有出口函数：卸载驱动程序时，就会去调用这个出口函数           */
static void __exit second_exit(void)
{

}

/* 7. 其他完善：提供设备信息，自动创建设备节点                                     */
module_init(senond_init);
module_exit(second_exit);

MODULE_AUTHOR("andy2333");


MODULE_LICENSE("GPL");

