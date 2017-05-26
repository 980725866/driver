
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
    int isstreaming;
    int grabmethod;
    int width;
    int height;
    int bpp;
    int formatIn;
    int formatOut;
    int framesizeIn;
    int signalquit;
    int toggleAvi;
    int getPict;
    int rawFrameCapture;
};


#endif /* _VIDEO_H_ */





