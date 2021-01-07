#include<linux/init.h>
#include<linux/module.h>

static int __init hello_init(void)
{
	printk(KERN_INFO "hello word enter \n");
	return 0;
}
module_init(hello_init);
static void __exit hello_exit(void)
{
	printk(KERN_INFO "hello word exit\n");
}
module_exit(hello_exit);

MODULE_AUTHOR("Barry Song<>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("A simple Hello Word Mdule");
MODULE_ALIAS("a simplest module");
