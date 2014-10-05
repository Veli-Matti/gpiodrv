
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>
 
static int gpiodrv_open(struct inode *inode, struct file *file)
{
    pr_info("I have been awoken\n");
    return 0;
}
 
static int gpiodrv_close(struct inode *inodep, struct file *filp)
{
    pr_info("Sleepy time\n");
    return 0;
}
 
static ssize_t gpiodrv_write(struct file *file, const char __user *buf,
		       size_t len, loff_t *ppos)
{
    pr_info("Yummy - I just ate %d bytes\n", len);
    return len; /* But we don't actually do anything with the data */
}
 
static ssize_t gpiodrv_read(struct file *file, char __user *buf,
		       size_t len, loff_t *ppos)
{
	char localData[] = "Jotakin Dataa\n";
	int datalen = strlen(localData);

    pr_info("Yummy - I asked to read %d bytes\n", len);

	if (*ppos > 0)
		return 0;

    if( copy_to_user(buf, localData, datalen) != 0 )
		return -EFAULT;	
		
	*ppos += datalen;
    return datalen;
}
 
static const struct file_operations gpiodrv_fops = {
    .owner			= THIS_MODULE,
    .write			= gpiodrv_write,
    .read			= gpiodrv_read,
    .open			= gpiodrv_open,
    .release		= gpiodrv_close,
    .llseek 		= no_llseek,
};
 
struct miscdevice gpiodrv_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "gpiodrv",
    .fops = &gpiodrv_fops,
};
 
static int __init gpiodrv_init(void)
{
    int error;
 
    error = misc_register(&gpiodrv_device);
    if (error) {
        pr_err("can't misc_register :(\n");
        return error;
    }
 
    pr_info("I'm in\n");
    return 0;
}
 
static void __exit gpiodrv_exit(void)
{
    misc_deregister(&gpiodrv_device);
    pr_info("I'm out\n");
}
 
module_init(gpiodrv_init)
module_exit(gpiodrv_exit)
 
MODULE_DESCRIPTION("Simple GPIO Driver");
MODULE_AUTHOR("Veli-Matti Kananen");
MODULE_LICENSE("GPL");
