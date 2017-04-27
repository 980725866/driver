#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/leds.h>
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

struct led_classdev led_cdev;

static void tiny4412_led_set(struct led_classdev *led_cdev,
			    enum led_brightness value)
{
	if(value == 0) {
		gpio_set_value(led_desc[0].gpio,1);
	}else{
		gpio_set_value(led_desc[0].gpio,0);
	}
}

static int __init tiny4412_led_init(void) {
	int i;
	/*1 gpio 的申请*/
	for(i=0;i<LED_NUM;i++){
		gpio_request(led_desc[i].gpio, led_desc[i].name);
		s3c_gpio_cfgpin(led_desc[i].gpio, S3C_GPIO_OUTPUT);
	}
	/* 2 led设备的注册 led_classdev_register */
	led_cdev.brightness_set = tiny4412_led_set;
	led_cdev.name = "led1";
	led_classdev_register(NULL, &led_cdev);
	printk(KERN_INFO "tiny4412_led_init\n");
	
	return 0;
}

static void __exit tiny4412_led_exit(void) {
	int i;
	led_classdev_unregister(&led_cdev);
	for(i=0;i<LED_NUM;i++){
		gpio_free(led_desc[i].gpio);
	}
	printk(KERN_INFO "tiny4412_led_exit\n");
}


module_init(tiny4412_led_init);
module_exit(tiny4412_led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ARM led3");
