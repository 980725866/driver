#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/leds.h>
#include <linux/platform_device.h>
// driver 
struct tiny4412_led_platdata {
	unsigned int	gpio;
	char			*name;
};

//device 1
struct tiny4412_led_platdata led_data1 = {
	.gpio = EXYNOS4X12_GPM4(0),
	.name = "led1",
};
static struct platform_device tiny4412_led_device1 = {
	.name		= "tiny4412_led",
	.id		= 0,
	.dev	= {
		.platform_data	= &led_data1,
	},
};

//device 2
struct tiny4412_led_platdata led_data2 = {
	.gpio = EXYNOS4X12_GPM4(1),
	.name = "led2",
};
static struct platform_device tiny4412_led_device2 = {
	.name		= "tiny4412_led",
	.id		= 1,
	.dev	= {
		.platform_data	= &led_data2,
	},
};

//device 3

struct tiny4412_led_platdata led_data3 = {
	.gpio = EXYNOS4X12_GPM4(2),
	.name = "led3",
};
static struct platform_device tiny4412_led_device3 = {
	.name		= "tiny4412_led",
	.id		= 2,
	.dev	= {
		.platform_data	= &led_data3,
	},
};

static int __init device_led_init(void) {
	platform_device_register(&tiny4412_led_device1);
	platform_device_register(&tiny4412_led_device2);
	platform_device_register(&tiny4412_led_device3);
	printk(KERN_INFO "device_led_init\n");
	return 0;
}

static void __exit device_led_exit(void) {
	
	platform_device_unregister(&tiny4412_led_device1);
	platform_device_unregister(&tiny4412_led_device2);
	platform_device_unregister(&tiny4412_led_device3);
	printk(KERN_INFO "device_led_exit\n");
}


module_init(device_led_init);
module_exit(device_led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ARM led platform_device");
