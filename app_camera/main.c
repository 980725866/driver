#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/mman.h>

#include <linux/videodev2.h>


#include <video.h>
#include <fb.h>
#include <color.h>

/**
	int bufferPut(struct vdIn *vd);
	int bufferGet(struct vdIn *vd);
	int close_v4l2(struct vdIn *vd);
	int video_disable(struct vdIn *vd);
	int video_enable(struct vdIn *vd);
	int video_init();

	void draw_back(unsigned int width, unsigned int height, unsigned int color);
	void draw_pic(unsigned int x, unsigned int y, unsigned int color);
	int fb_close(void);
	int fb_init(void);
	
	void initLut(void);
	void freeLut(void);
	unsigned int Pyuv422torgb24(unsigned char * input_ptr, unsigned char * output_ptr, unsigned int image_width, unsigned int image_height);


*/

void draw_lcd(char *output_ptr)
{
	int i,j,color;
	char r,g,b;
	
	for(i=0;i<480;i++)
	{
		for(j=0;j<640;j++)
		{
			r = *output_ptr++;
			g = *output_ptr++;
			b = *output_ptr++;
			color = ( r << 16 ) | ( g << 8 ) | b ; 
			draw_pic(j,i,color);
		}
	}
}

int main(int argc , char **argv)
{
	int i;
	int ret;
	struct vdIn vd;
	int width,height;
	
#if 1
	/* fb init */
	fb_init();
#endif

	/* video init */
	memset(&vd,0,sizeof(struct vdIn));
	video_init(&vd);
	video_enable(&vd);
	width  = vd.width;
	height = vd.height;
	/* clor init */
	initLut();
	vd.tmpbuffer   = (unsigned char *)malloc( width * height * 4 );
	vd.framebuffer = (unsigned char *)malloc( width * height * 4 );
	if(!vd.tmpbuffer){
		return -1;
	}
	if(!vd.framebuffer){
		return -1;
	}
	memset(vd.tmpbuffer,0,sizeof(width * height * 4));
	memset(vd.framebuffer,0,sizeof(width * height * 4));
#if 1
	
	while(1)
	{
		
		bufferGet(&vd);

		printf("vd.mem length = %d\n",vd.buf.length);
		printf("index = %d use = %d\n",vd.buf.index,vd.buf.bytesused);
		
		memcpy(vd.tmpbuffer,vd.mem[vd.buf.index],vd.buf.bytesused);
		
		Pyuv422torgb24(vd.tmpbuffer,vd.framebuffer,width,height);

		for(i=0;i<10;i++){
			printf("%02x ",*(vd.tmpbuffer+i));
		}
		printf("\n");
		draw_lcd(vd.framebuffer);
		bufferPut(&vd);
	}
	
#endif	
	freeLut();
	video_disable(&vd);
	close_v4l2(&vd);
	free(vd.tmpbuffer);
	free(vd.framebuffer);
	return 0;
}








