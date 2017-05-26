/*******************************************************************************
# Linux-UVC streaming input-plugin for MJPG-streamer                           #
#                                                                              #
# This package work with the Logitech UVC based webcams with the mjpeg feature #
#                                                                              #
# Copyright (C) 2005 2006 Laurent Pinchart &&  Michel Xhaard                   #
#                    2007 Lucas van Staden                                     #
#                    2007 Tom Stöveken                                         #
#                                                                              #
# This program is free software; you can redistribute it and/or modify         #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; either version 2 of the License, or            #
# (at your option) any later version.                                          #
#                                                                              #
# This program is distributed in the hope that it will be useful,              #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU General Public License for more details.                                 #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with this program; if not, write to the Free Software                  #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA    #
#                                                                              #
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <pthread.h>
#include <syslog.h>

#include "../utils.h"
//#include "../../mjpg_streamer.h"
#include "v4l2uvc.h" // this header will includes the ../../mjpg_streamer.h
#include "huffman.h"
#include "jpeg_utils.h"
#include "dynctrl.h"
//#include "uvcvideo.h"

#define INPUT_PLUGIN_NAME "UVC webcam grabber"

/*
 * UVC resolutions mentioned at: (at least for some webcams)
 * http://www.quickcamteam.net/hcl/frame-format-matrix/
 */
static const struct {
    const char *string;
    const int width, height;
} resolutions[] = {
    { "QSIF", 160,  120  },
    { "QCIF", 176,  144  },
    { "CGA",  320,  200  },
    { "QVGA", 320,  240  },
    { "CIF",  352,  288  },
    { "VGA",  640,  480  },
    { "SVGA", 800,  600  },
    { "XGA",  1024, 768  },
    { "SXGA", 1280, 1024 }
};

/* private functions and variables to this plugin */
static globals *pglobal;
static int gquality = 80;
static unsigned int minimum_size = 0;
static int dynctrls = 1;

void *cam_thread(void *);
void cam_cleanup(void *);
int input_cmd(int plugin, unsigned int control, unsigned int group, int value);


/*** plugin interface functions ***/
/******************************************************************************
Description.: This function ializes the plugin. It parses the commandline-
              parameter and stores the default and parsed values in the
              appropriate variables.
Input Value.: param contains among others the command-line string
Return Value: 0 if everything is fine
              1 if "--help" was triggered, in this case the calling programm
              should stop running and leave.
******************************************************************************/
int input_init(input_parameter *param, int id)
{
    char *dev = NULL;
    int width = 640, height = 480, fps = 5, format = V4L2_PIX_FMT_YUYV, i;
    /* initialize the mutes variable */
    if(pthread_mutex_init(&cams[id].controls_mutex, NULL) != 0) {
        IPRINT("could not initialize mutex variable\n");
        exit(EXIT_FAILURE);
    }
    
   	// video7
	dev = param->parameters;
	
   
    DBG("input id: %d\n", id);
    cams[id].id = id;
    cams[id].pglobal = param->global;

    /* allocate webcam datastructure */
    cams[id].videoIn = malloc(sizeof(struct vdIn));
    if(cams[id].videoIn == NULL) {
        IPRINT("not enough memory for videoIn\n");
        exit(EXIT_FAILURE);
    }
    memset(cams[id].videoIn, 0, sizeof(struct vdIn));

    /* display the parsed values */
    IPRINT("Using V4L2 device.: %s\n", dev);
    IPRINT("Desired Resolution: %i x %i\n", width, height);
    IPRINT("Frames Per Second.: %i\n", fps);
    IPRINT("Format............: %s\n", (format == V4L2_PIX_FMT_YUYV) ? "YUV" : "MJPEG");
    if(format == V4L2_PIX_FMT_YUYV)
        IPRINT("JPEG Quality......: %d\n", gquality);

    DBG("vdIn pn: %d\n", id);
    /* open video device and prepare data structure */
    if(init_videoIn(cams[id].videoIn, dev, width, height, fps, format, 1, cams[id].pglobal, id) < 0) {
        IPRINT("init_VideoIn failed\n");
        closelog();
        exit(EXIT_FAILURE);
    }
  
    return 0;
}

/******************************************************************************
Description.: Stops the execution of worker thread
Input Value.: -
Return Value: always 0
******************************************************************************/
int input_stop(int id)
{
    DBG("will cancel camera thread #%02d\n", id);
    pthread_cancel(cams[id].threadID);
    return 0;
}

/******************************************************************************
Description.: spins of a worker thread
Input Value.: -
Return Value: always 0
******************************************************************************/
int input_run(int id)
{
    cams[id].pglobal->in[id].buf = malloc(cams[id].videoIn->framesizeIn);
    if(cams[id].pglobal->in[id].buf == NULL) {
        fprintf(stderr, "could not allocate memory\n");
        exit(EXIT_FAILURE);
    }

    DBG("launching camera thread #%02d\n", id);
    /* create thread and pass context to thread function */
    pthread_create(&(cams[id].threadID), NULL, cam_thread, &(cams[id]));
    pthread_detach(cams[id].threadID);
    return 0;
}

/******************************************************************************
Description.: this thread worker grabs a frame and copies it to the global buffer
Input Value.: unused
Return Value: unused, always NULL
******************************************************************************/
void *cam_thread(void *arg)
{

    context *pcontext = arg;
    pglobal = pcontext->pglobal;

    /* set cleanup handler to cleanup allocated ressources */
    pthread_cleanup_push(cam_cleanup, pcontext);

    while(!pglobal->stop) {
        while(pcontext->videoIn->streamingState == STREAMING_PAUSED) {
            usleep(1); // maybe not the best way so FIXME
        }

        /* grab a frame */
        if(uvcGrab(pcontext->videoIn) < 0) {
            IPRINT("Error grabbing frames\n");
            exit(EXIT_FAILURE);
        }

        DBG("received frame of size: %d from plugin: %d\n", pcontext->videoIn->buf.bytesused, pcontext->id);

        /*
         * Workaround for broken, corrupted frames:
         * Under low light conditions corrupted frames may get captured.
         * The good thing is such frames are quite small compared to the regular pictures.
         * For example a VGA (640x480) webcam picture is normally >= 8kByte large,
         * corrupted frames are smaller.
         */
        if(pcontext->videoIn->buf.bytesused < minimum_size) {
            DBG("dropping too small frame, assuming it as broken\n");
            continue;
        }

        /* copy JPG picture to global buffer */
        pthread_mutex_lock(&pglobal->in[pcontext->id].db);

        /*
         * If capturing in YUV mode convert to JPEG now.
         * This compression requires many CPU cycles, so try to avoid YUV format.
         * Getting JPEGs straight from the webcam, is one of the major advantages of
         * Linux-UVC compatible devices.
         */
        if(pcontext->videoIn->formatIn == V4L2_PIX_FMT_YUYV) {
            DBG("compressing frame from input: %d\n", (int)pcontext->id);
            pglobal->in[pcontext->id].size = compress_yuyv_to_jpeg(pcontext->videoIn, pglobal->in[pcontext->id].buf, pcontext->videoIn->framesizeIn, gquality);
        } else {
            DBG("copying frame from input: %d\n", (int)pcontext->id);
            pglobal->in[pcontext->id].size = memcpy_picture(pglobal->in[pcontext->id].buf, pcontext->videoIn->tmpbuffer, pcontext->videoIn->buf.bytesused);
        }

#if 0
        /* motion detection can be done just by comparing the picture size, but it is not very accurate!! */
        if((prev_size - global->size)*(prev_size - global->size) > 4 * 1024 * 1024) {
            DBG("motion detected (delta: %d kB)\n", (prev_size - global->size) / 1024);
        }
        prev_size = global->size;
#endif

        /* copy this frame's timestamp to user space */
        pglobal->in[pcontext->id].timestamp = pcontext->videoIn->buf.timestamp;

        /* signal fresh_frame */
        pthread_cond_broadcast(&pglobal->in[pcontext->id].db_update);
        pthread_mutex_unlock(&pglobal->in[pcontext->id].db);


        /* only use usleep if the fps is below 5, otherwise the overhead is too long */
        if(pcontext->videoIn->fps < 5) {
            DBG("waiting for next frame for %d us\n", 1000 * 1000 / pcontext->videoIn->fps);
            usleep(1000 * 1000 / pcontext->videoIn->fps);
        } else {
            DBG("waiting for next frame\n");
        }
    }

    DBG("leaving input thread, calling cleanup function now\n");
    pthread_cleanup_pop(1);

    return NULL;
}

/******************************************************************************
Description.:
Input Value.:
Return Value:
******************************************************************************/
void cam_cleanup(void *arg)
{
    static unsigned char first_run = 1;
    context *pcontext = arg;
    pglobal = pcontext->pglobal;
    if(!first_run) {
        DBG("already cleaned up ressources\n");
        return;
    }

    first_run = 0;
    IPRINT("cleaning up ressources allocated by input thread\n");

    close_v4l2(pcontext->videoIn);
    if(pcontext->videoIn->tmpbuffer != NULL) free(pcontext->videoIn->tmpbuffer);
    if(pcontext->videoIn != NULL) free(pcontext->videoIn);
    if(pglobal->in[pcontext->id].buf != NULL)
        free(pglobal->in[pcontext->id].buf);
}

