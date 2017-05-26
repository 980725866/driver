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
/**
	pixelformat = 'YUYV', description = 'YUV 4:2:2 (YUYV) == V4L2_PIX_FMT_YUYV
	{ pixelformat = 'MJPG', description = 'MJPEG' }

*/

#define DEVICE_NAME "/dev/video15"

int video_init(struct vdIn *vd)
{
	int i;
	int ret;
	if(vd == NULL)
		vd = malloc(sizeof(struct vdIn));
	
	vd->width  = 640;
	vd->height = 480;
    vd->formatIn = V4L2_PIX_FMT_YUYV;
    
    printf("vd->formatIn = %d\n",vd->formatIn);
	vd->fd = open(DEVICE_NAME,O_RDWR);
	if(vd->fd < 0){
		perror("open");
		return -1;
	}
	/*  1 */
	memset(&vd->cap, 0, sizeof(struct v4l2_capability));
    ret = ioctl(vd->fd, VIDIOC_QUERYCAP, &vd->cap);
	if (ret < 0) {
		printf("Error VIDIOC_QUERYCAP device \n");
		return -1;
    }
    if ((vd->cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
		printf("Error video capture not supported.\n");
    }
    if (!(vd->cap.capabilities & V4L2_CAP_STREAMING)) {
	    printf("does not support streaming i/o\n");
    }
#if 0
	/*2 enum  fmt*/
	
	/*

		memset(&vd->fmt, 0, sizeof(struct v4l2_format));
		vd->fmt.index = 0;
		vd->fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		while ((ret = ioctl(fd, VIDIOC_ENUM_FMT, &fmt)) == 0) {
			fmt.index++;
			printf("{ pixelformat = '%c%c%c%c', description = '%s' }\n",
					fmt.pixelformat & 0xFF, (fmt.pixelformat >> 8) & 0xFF,
					(fmt.pixelformat >> 16) & 0xFF, (fmt.pixelformat >> 24) & 0xFF,
					fmt.description);			
		}
	*/
#endif	
	/* 3 VIDIOC_S_FMT  */
	memset(&vd->fmt, 0, sizeof(struct v4l2_format));
    vd->fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    vd->fmt.fmt.pix.width = vd->width;
    vd->fmt.fmt.pix.height = vd->height;
    vd->fmt.fmt.pix.pixelformat = vd->formatIn;
    vd->fmt.fmt.pix.field = V4L2_FIELD_ANY;
    ret = ioctl(vd->fd, VIDIOC_S_FMT, &vd->fmt);
    if (ret < 0) {
		printf("Unable to set format: %d.\n", ret);
		return -1;
    }

	 if ((vd->fmt.fmt.pix.width != vd->width) || (vd->fmt.fmt.pix.height != vd->height)) {
		printf(" format asked unavailable get width %d height %d \n",vd->fmt.fmt.pix.width, vd->fmt.fmt.pix.height);
		vd->width = vd->fmt.fmt.pix.width;
		vd->height = vd->fmt.fmt.pix.height;
    }
	printf("pixelformat = %d\n",vd->fmt.fmt.pix.pixelformat);
	printf("width = %d\n",		vd->fmt.fmt.pix.height);
	printf("height = %d\n",vd->fmt.fmt.pix.width);

	/* request buffers */
    memset(&vd->rb, 0, sizeof(struct v4l2_requestbuffers));
    vd->rb.count = NB_BUFFER;
    vd->rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    vd->rb.memory = V4L2_MEMORY_MMAP;

    ret = ioctl(vd->fd, VIDIOC_REQBUFS, &vd->rb);
    if (ret < 0) {
		printf("Unable to allocate buffers: %d.\n", ret);
		return -1;
    }
    /* map the buffers */
    for (i = 0; i < NB_BUFFER; i++) {
		memset(&vd->buf, 0, sizeof(struct v4l2_buffer));
		vd->buf.index = i;
		vd->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		vd->buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(vd->fd, VIDIOC_QUERYBUF, &vd->buf);
		if (ret < 0) {
		    printf("Unable to query buffer (%d).\n", ret);
		    return -1;
		}
		vd->mem[i] = mmap(0 ,vd->buf.length, PROT_READ, MAP_SHARED, vd->fd,vd->buf.m.offset);
		if (vd->mem[i] == MAP_FAILED) {
		    printf("Unable to map buffer\n");
		    return -1;
		}
		
    }
    
    /* Queue the buffers. */
    for (i = 0; i < NB_BUFFER; ++i) {
		memset(&vd->buf, 0, sizeof(struct v4l2_buffer));
		vd->buf.index = i;
		vd->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		vd->buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(vd->fd, VIDIOC_QBUF, &vd->buf);
		if (ret < 0) {
		    printf("Unable to queue buffer (%d).\n", ret);
		    return -1;
		}
    }
	return 0;
}


int video_enable(struct vdIn *vd)
{
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int ret;

    ret = ioctl(vd->fd, VIDIOC_STREAMON, &type);
    if (ret < 0) {
		printf("Unable to %s capture: %d.\n", "start", ret);
		return ret;
    }
    vd->isstreaming = 1;
    return 0;
}

int video_disable(struct vdIn *vd)
{
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int ret;

    ret = ioctl(vd->fd, VIDIOC_STREAMOFF, &type);
    if (ret < 0) {
		printf("Unable to %s capture: %d.\n", "stop", ret);
		return ret;
    }
    vd->isstreaming = 0;
    return 0;
}

int close_v4l2(struct vdIn *vd)
{
    int i;
    for (i = 0; i < NB_BUFFER ; i++)
    {
        if (vd->mem[i])
        {
            munmap(vd->mem[i], vd->buf.length);
           	vd->mem[i] = NULL;
        }
    } 
    close(vd->fd);
    //free(vd);
}


int bufferGet(struct vdIn *vd)
{
	int ret;
#if 0
	
	struct pollfd tFds[1];
     
    /* poll */
    tFds[0].fd     = vd->fd;
    tFds[0].events = POLLIN;

    ret = poll(tFds, 1, -1);
    if (ret <= 0)
    {
        printf("poll error!\n");
        return -1;
    }
#endif
    memset(&vd->buf, 0, sizeof(struct v4l2_buffer));
    vd->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    vd->buf.memory = V4L2_MEMORY_MMAP;
    ret = ioctl(vd->fd, VIDIOC_DQBUF, &vd->buf);
    if (ret < 0) 
    {
    	printf("Unable to dequeue buffer.\n");
    	return -1;
    }
    printf("index = %d\n",vd->buf.index);
	return 0;
}


int bufferPut(struct vdIn *vd)
{
	int ret;
	//memset(&vd->buf, 0, sizeof(struct v4l2_buffer));
	//vd->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	//vd->buf.memory = V4L2_MEMORY_MMAP;
	ret = ioctl(vd->fd, VIDIOC_QBUF, &vd->buf);
	if (ret < 0) {
		printf("Unable to queue buffer (%d).\n", ret);
		return ret;
	}
	return 0;
}








