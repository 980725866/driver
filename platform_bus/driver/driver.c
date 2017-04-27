#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/leds.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
struct tiny4412_led_platdata {
	unsigned int	gpio;
	char			*name;
};

struct tiny4412_gpio_led {
	struct led_classdev		 cdev;
	struct tiny4412_led_platdata *pdata;
};

static inline struct tiny4412_gpio_led *pdev_to_gpio(struct platform_device *dev)
{
	return platform_get_drvdata(dev);//通过平台设备里内嵌的dev(dev->dev)找到led_class(led_class->dev)最终找到tiny4412_gpio_led
}

static inline struct tiny4412_gpio_led *to_gpio(struct led_classdev *led_cdev)
{
	return container_of(led_cdev, struct tiny4412_gpio_led, cdev);
}

static int tiny4412_led_remove(struct platform_device *dev)
{
	struct tiny4412_gpio_led *led = pdev_to_gpio(dev);
	struct tiny4412_led_platdata *pd = led->pdata;
	
	led_classdev_unregister(&led->cdev);
	gpio_free(pd->gpio);
	kfree(led);
	printk(KERN_INFO "tiny4412_led_remove\n");
	return 0;
}

static void tiny4412_led_set(struct led_classdev *led_cdev,
			    enum led_brightness value)
{
	struct tiny4412_gpio_led *led = to_gpio(led_cdev);
	struct tiny4412_led_platdata *pd = led->pdata;
	if(value > 0)
		gpio_set_value(pd->gpio, 0);
	else
		gpio_set_value(pd->gpio, 1);
	
}

static int tiny4412_led_probe(struct platform_device *dev)
{
	
	struct tiny4412_gpio_led *led;
	struct tiny4412_led_platdata *pdata = dev->dev.platform_data;/*1 取出device信息*/
	
	led = kzalloc(sizeof(struct tiny4412_gpio_led), GFP_KERNEL);
	if (led == NULL) {
		dev_err(&dev->dev, "No memory for device\n");
		return -ENOMEM;
	}
	platform_set_drvdata(dev, led);//驱动和设备绑定

	led->cdev.brightness_set = tiny4412_led_set;
	led->cdev.name = pdata->name;
	led->pdata = pdata;
	
	/* 2 */
	gpio_request(pdata->gpio, pdata->name);
	s3c_gpio_cfgpin(pdata->gpio, S3C_GPIO_OUTPUT);
	
	/* 3 led设备的注册 led_classdev_register */
	//led_classdev_register(NULL, &led->cdev);
	led_classdev_register(&dev->dev, &led->cdev);
	
	printk(KERN_INFO "tiny4412_led_probe %s\n",pdata->name);
	return 0;
}

static struct platform_driver tiny4412_led_driver = {
	.probe		= tiny4412_led_probe,
	.remove		= tiny4412_led_remove,
	.driver		= {
		.name		= "tiny4412_led",
		.owner		= THIS_MODULE,
	},
};

static int __init driver_led_init(void) {
	platform_driver_register(&tiny4412_led_driver);
	printk(KERN_INFO "driver_led_init\n");
	return 0;
}

static void __exit driver_led_exit(void) {
	
	platform_driver_register(&tiny4412_led_driver);
	printk(KERN_INFO "driver_led_exit\n");
}


module_init(driver_led_init);
module_exit(driver_led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ARM led platform_driver");
