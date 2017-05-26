
#ifndef _VIDEO_H_
#define _VIDEO_H_

#define NB_BUFFER 4

struct vdIn {
    int fd;
    struct v4l2_capability cap;
    struct v4l2_format fmt;
    struct v4l2_buffer buf;
    struct v4l2_requestbuffers rb;
    void *mem[NB_BUFFER];
    unsigned char *tmpbuffer;
    unsigned char *framebuffer;
    int isstreaming;
    int width;
    int height;
    int bpp;
    int formatIn;
};


int bufferPut(struct vdIn *vd);
int bufferGet(struct vdIn *vd);
int close_v4l2(struct vdIn *vd);
int video_disable(struct vdIn *vd);
int video_enable(struct vdIn *vd);
int video_init(struct vdIn *vd);

#endif /* _VIDEO_H_ */





