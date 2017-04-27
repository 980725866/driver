#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>

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


static const struct file_operations tiny4412_led_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= tiny4412_leds_ioctl,
};

static struct miscdevice tiny4412_led_dev = {
	.minor			= MISC_DYNAMIC_MINOR,
	.name			= DEVICE_NAME,
	.fops			= &tiny4412_led_fops,
};

static int __init tiny4412_led_init(void) {
	int i;
	/*1 gpio 的申请*/
	for(i=0;i<LED_NUM;i++){
		gpio_request(led_desc[i].gpio, led_desc[i].name);
		s3c_gpio_cfgpin(led_desc[i].gpio, S3C_GPIO_OUTPUT);
	}
	/* 2 misc设备的注册 */
	/* 3 类的创建 设备的创建 misc 帮我们做了*/
	misc_register(&tiny4412_led_dev);
	
	return 0;
}

static void __exit tiny4412_led_exit(void) {
	int i;
	
	misc_deregister(&tiny4412_led_dev);
	for(i=0;i<LED_NUM;i++){
		gpio_free(led_desc[i].gpio);
	}
	printk(KERN_INFO "tiny4412_led_exit\n");
}


module_init(tiny4412_led_init);
module_exit(tiny4412_led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ARM led4 misc");
