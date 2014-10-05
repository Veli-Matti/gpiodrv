
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/string.h>

struct gpio_dev {
	int led_pin;
} my_gpio_dev;

#define INPUT_MAX_LEN	30

static int setPin(const int pin, const int mode, const int value);

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
	char workbuf[INPUT_MAX_LEN];
	char *token = NULL;
	char *work_ptr = workbuf;

    pr_info("Yummy - I just ate %d bytes\n", len);
	
	if (len > INPUT_MAX_LEN)
		len = INPUT_MAX_LEN;
		
	memset(workbuf, '\0', len);
	
	/* Get the data from user space */
	if (copy_from_user(workbuf, buf, len))
		return 0;
	
	pr_info("workbuf =='%s'\n", workbuf);
	
	/* Parse gpio#, pinmode and value (output only) */
	token = strsep(&work_ptr, " ");
	
	/* TODO: parsing handling for bad data (trimming, data validation etc)*/
	if (token) {
		// Port
		long pin = -1;
		if (kstrtol(token, 10, &pin))
			return -1;
		
		pr_info("token (1) =='%s'\n", token);

		// Direction
		token = strsep(&work_ptr, " ");
		if (token) {
			pr_info("token (3) =='%s'\n", token);
			if (strcmp(token, "output")==0) {
				// Set pin as output
				if (work_ptr) {
					int value = 1;
					pr_info("work_ptr (4) =='%s'\n", work_ptr);
					if (*work_ptr == '0')
						value = 0;
					
					setPin(pin, 1, value);
				}
				
			} else if (strcmp(token, "input")) {
				// Set the pin as input
				setPin(pin, 0, 0);
				
			} else {
				// Error
			}
			
		}
	}
	
    return len;
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

static int setPin(const int pin, const int mode, const int value)
{
	/* Request the pin */
    if (gpio_request(pin, "gpiodrv_pin")) {
		pr_info("gpio reservation failed!\n");
		return 1;
	}
	/* Set the direction */
	if (mode == 1) {
		
		if (gpio_direction_output(pin, value)) {
			gpio_free(pin);
			pr_info("gpio output setting failed!\n");
			return 2;
		}
	} else {
		if (gpio_direction_input(pin)) {
			gpio_free(pin);
			pr_info("gpio input setting failed!\n");
			return 3;
		}
	}
	
	/* Release the used resources */
	gpio_free(pin);
	return 0;

}
 
module_init(gpiodrv_init)
module_exit(gpiodrv_exit)
 
MODULE_DESCRIPTION("Simple GPIO Driver");
MODULE_AUTHOR("Veli-Matti Kananen");
MODULE_LICENSE("GPL");
