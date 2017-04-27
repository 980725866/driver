#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>

#include <linux/gpio.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>

#define 	DEVICE_NAME 	"leds"
#define 	CLASS_NAME 		"ledclass"

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
static struct class *led_class;
static int major ;

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

static struct file_operations tiny4412_led_fops = {
	.owner			= THIS_MODULE,
	.unlocked_ioctl	= tiny4412_leds_ioctl,
};

static int __init tiny4412_led_init(void) {
	int i;
	/*1 gpio 的申请 设置为输出模式*/
	printk(KERN_INFO "tiny4412_led_init start\n");
	for(i=0;i<LED_NUM;i++){
		gpio_request(led_desc[i].gpio, led_desc[i].name);
		s3c_gpio_cfgpin(led_desc[i].gpio, S3C_GPIO_OUTPUT);
	}
	/* 2 设备的注册 最老的注册方法包过了 cdev_init、cdev_add*/
	major = register_chrdev(0, DEVICE_NAME, &tiny4412_led_fops);
	printk(KERN_INFO "major = %d\n", major);
	/* 3 类的创建*/
	led_class = class_create(THIS_MODULE, CLASS_NAME);
	/* 4 设备的创建*/
	device_create(led_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
	printk(KERN_INFO "tiny4412_led_init end\n");
	return 0;
}

static void __exit tiny4412_led_exit(void) {
	int i;
	printk(KERN_INFO "tiny4412_led_exit start\n");
	device_destroy(led_class,MKDEV(major, 0));
	class_destroy(led_class);
	unregister_chrdev(major, DEVICE_NAME);
	for(i=0;i<LED_NUM;i++){
		gpio_free(led_desc[i].gpio);
	}
	printk(KERN_INFO "tiny4412_led_exit end\n");
}


module_init(tiny4412_led_init);
module_exit(tiny4412_led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ARM led1.");
