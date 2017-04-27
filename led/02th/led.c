#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#define 	DEVICE_NAME 	"leds"

typedef struct gpio_desc
{
	long gpio;
	char name[10];
}gpio_desc_t;

static gpio_desc_t led_desc[] = {
	{EXYNOS4X12_GPM4(0),"led1"},
	{EXYNOS4X12_GPM4(1),"led2"},
	{EXYNOS4X12_GPM4(2),"led3"},
	{EXYNOS4X12_GPM4(3),"led4"},
};

#define 	LED_NUM			ARRAY_SIZE(led_desc)
static dev_t dev_t_led;
struct cdev  cdev_led;
static struct class *led_class;
static int major;
static int read;

static long tiny4412_leds_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg)
{
	switch(cmd) {
		case 0:
		case 1:
			if (arg > LED_NUM) {
				return -EINVAL;
			}
			gpio_set_value(led_desc[arg].gpio, !cmd);
			printk(KERN_INFO "cmd = %d,arg = %ld\n", cmd, arg);
			break;

		default:
			return -EINVAL;
	}
	return 0;
}

static int leds_open(struct inode *inode, struct file *file) {
	printk(KERN_INFO "leds_open\n");
	return 0;
}

static int leds_release(struct inode *inode, struct file *file) {
	printk(KERN_INFO "leds_release\n");
	return 0;
}

static ssize_t leds_read(struct file *file, char __user *buf,
			size_t count, loff_t *ppos) 
{
	/* copy_to_user(void __user *to, const void *from, unsigned long n) */
	char ker_buf[512];
	sprintf(ker_buf,"%d",read++);
	copy_to_user(buf,ker_buf,strlen(ker_buf));
	printk(KERN_INFO "leds_read\n");
	return strlen(ker_buf);
}
			
static ssize_t leds_write(struct file *file, const char __user *buf,
			 size_t count, loff_t *ppos)
{
	/* copy_from_user(void *to, const void __user *from, unsigned long n)*/
	char ker_buf[512];
	copy_from_user(ker_buf,buf,count);
	printk(KERN_INFO "leds_write = %s\n",ker_buf);
	return 0;
}

static const struct file_operations tiny4412_led_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= tiny4412_leds_ioctl,
	.read		= leds_read,
	.write		= leds_write,
	.open		= leds_open,
	.release	= leds_release,
};

static int __init tiny4412_led_init(void) {
	int i,ret;
	/*1 gpio 的申请*/
	printk(KERN_INFO "tiny4412_led_init start2\n");
	for(i=0;i<LED_NUM;i++){
		gpio_request(led_desc[i].gpio, led_desc[i].name);
		s3c_gpio_cfgpin(led_desc[i].gpio, S3C_GPIO_OUTPUT);
	}
	/* 2 设备的注册 alloc_chrdev_region cdev_init、cdev_add*/
	ret = alloc_chrdev_region(&dev_t_led, 0, 1,DEVICE_NAME);
	if(ret){
		printk(KERN_INFO "alloc_chrdev_region error\n");
		return -1;
	}
	major = MAJOR(dev_t_led);
	printk(KERN_INFO "major = %d\n", major);
	cdev_init(&cdev_led, &tiny4412_led_fops);//cdev_led.owner = THIS_MODULE;
	
	cdev_led.owner = THIS_MODULE;
	
	ret = cdev_add(&cdev_led, MKDEV(major, 0), 1);
	if(ret){
		printk(KERN_INFO "cdev_add error\n");
		unregister_chrdev_region(MKDEV(major, 0),1);
	}
	
	
	/* 3 类的创建*/
	led_class = class_create(THIS_MODULE, "myleds");
	/* 4 设备的创建*/
	device_create(led_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
	printk(KERN_INFO "tiny4412_led_init end2\n");
	return 0;
}

static void __exit tiny4412_led_exit(void) {
	int i;
	printk(KERN_INFO "tiny4412_led_exit start2\n");
	device_destroy(led_class,MKDEV(major, 0));
	class_destroy(led_class);
	cdev_del(&cdev_led);
	unregister_chrdev_region(MKDEV(major, 0),1);
	for(i=0;i<LED_NUM;i++){
		gpio_free(led_desc[i].gpio);
	}
	printk(KERN_INFO "tiny4412_led_exit end2\n");
}


module_init(tiny4412_led_init);
module_exit(tiny4412_led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ARM led2");
