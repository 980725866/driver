#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <fb.h>
// 宏定义
#define FBDEVICE	"/dev/fb0"
#define WIDTH		800	
#define HEIGHT		480
#define WHITE		0xffffffff			// test ok

// 全局变量
unsigned int *pfb = NULL;
int fd ;

int fb_init(void)
{
	int ret = -1;
	
	struct fb_fix_screeninfo finfo = {0};
	struct fb_var_screeninfo vinfo = {0};
	
	// 第1步：打开设备
	fd = open(FBDEVICE, O_RDWR);
	if (fd < 0)
	{
		printf("haha \n");
		perror("open");
		return -1;
	}
	printf("open %s success.\n", FBDEVICE);
	
	// 第2步：获取设备的硬件信息
	ret = ioctl(fd, FBIOGET_FSCREENINFO, &finfo);
	if (ret < 0)
	{
		perror("ioctl");
		return -1;
	}
	printf("smem_start = %u, smem_len = %u.\n", finfo.smem_start, finfo.smem_len);
	
	ret = ioctl(fd, FBIOGET_VSCREENINFO, &vinfo);
	if (ret < 0)
	{
		perror("ioctl");
		return -1;
	}
	printf("xres = %u, yres = %u.\n", vinfo.xres, vinfo.yres);
	printf("xres_virtual = %u, yres_virtual = %u.\n", vinfo.xres_virtual, vinfo.yres_virtual);
	printf("bpp = %u.\n", vinfo.bits_per_pixel);

	// 第3步：进行mmap
	unsigned long len = vinfo.xres_virtual * vinfo.yres_virtual * vinfo.bits_per_pixel / 8;
	printf("len = %ld\n", len);
	pfb = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (NULL == pfb)
	{
		perror("mmap");
		return -1;
	}
	printf("pfb = %p.\n", pfb);
	
	draw_back(WIDTH, HEIGHT, WHITE);
	
	return 0;
}

int fb_close()
{
	return close(fd);
}

void draw_pic(unsigned int x, unsigned int y, unsigned int color)
{
	*(pfb + y * WIDTH + x) = color;
}

void draw_back(unsigned int width, unsigned int height, unsigned int color)
{
	unsigned int x, y;
	
	for (y=0; y<height; y++)
	{
		for (x=0; x<width; x++)
		{
			*(pfb + y * WIDTH + x) = color;
		}
	}
}















