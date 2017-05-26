
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
#include <dlfcn.h>
#include <fcntl.h>
#include <syslog.h>

#include "utils.h"
#include "mjpg_streamer.h"

extern int input_init(input_parameter *param, int id);
extern int input_stop(int id);
extern int input_run(int id);
extern int output_init(output_parameter *param, int id);
extern int output_stop(int id);
extern int output_run(int id);

/* globals */
static globals global;

void signal_handler(int sig)
{
    int i;

    /* signal "stop" to threads */
    LOG("setting signal to stop\n");
    global.stop = 1;
    usleep(1000 * 1000);

    /* clean up threads */
    LOG("force cancellation of threads and cleanup resources\n");
    for(i = 0; i < global.incnt; i++) {
        global.in[i].stop(i);
    }

    for(i = 0; i < global.outcnt; i++) {
        global.out[i].stop(global.out[i].param.id);
        pthread_cond_destroy(&global.in[i].db_update);
        pthread_mutex_destroy(&global.in[i].db);
    }
    usleep(1000 * 1000);

    DBG("all plugin handles closed\n");

    LOG("done\n");

    closelog();
    exit(0);
    return;
}

int main(int argc, char *argv[])
{
    //char *input  = "input_uvc.so --resolution 640x480 --fps 5 --device /dev/video0";
    char *input[MAX_INPUT_PLUGINS];
    char *output[MAX_OUTPUT_PLUGINS];
    int daemon = 0, i;
    size_t tmp = 0;

    output[0] = "output_http.so --port 8080";
    global.outcnt = 1;
	global.incnt  = 1;
	
   
   
    /* ignore SIGPIPE (send by OS if transmitting to closed TCP sockets) */
    signal(SIGPIPE, SIG_IGN);

    /* register signal handler for <CTRL>+C in order to clean up */
    if(signal(SIGINT, signal_handler) == SIG_ERR) {
        LOG("could not register signal handler\n");
        closelog();
        exit(EXIT_FAILURE);
    }

  
    /* open input plugin */
    for(i = 0; i < global.incnt; i++) {
        /* this mutex and the conditional variable are used to synchronize access to the global picture buffer */
        if(pthread_mutex_init(&global.in[i].db, NULL) != 0) {
            LOG("could not initialize mutex variable\n");
            exit(EXIT_FAILURE);
        }
        if(pthread_cond_init(&global.in[i].db_update, NULL) != 0) {
            LOG("could not initialize condition variable\n");
            exit(EXIT_FAILURE);
        }
        global.in[i].stop      = 0;
        global.in[i].buf       = NULL;
        global.in[i].size      = 0;
        global.in[i].init = input_init;      
        global.in[i].stop = input_stop;     
        global.in[i].run = input_run;               
		global.in[i].param.global = &global;
		global.in[i].param.id = i;
		
        global.in[i].param.parameters = argv[1];
        
        if(global.in[i].init(&global.in[i].param, i)) { //globals *global,char *dev,int id
            LOG("input_init() return value signals to exit\n");  
            exit(0);
        }
        
    }

    /* open output plugin */
    for(i = 0; i < global.outcnt; i++) {
      
        global.out[i].init = output_init;
        global.out[i].stop = output_stop;
        global.out[i].run = output_run;
        global.out[i].param.global = &global;
		global.out[i].param.id = i;
        if(global.out[i].init(&global.out[i].param, i)) {
            LOG("output_init() return value signals to exit\n");
            closelog();
            exit(0);
        }
        
    }

 
    DBG("starting %d input plugin\n", global.incnt);
    for(i = 0; i < global.incnt; i++) {
        if(global.in[i].run(i)) {
            LOG("can not run input plugin %d: %s\n", i, global.in[i].plugin);
            closelog();
            return 1;
        }
    }

    DBG("starting %d output plugin(s)\n", global.outcnt);
    
    for(i = 0; i < global.outcnt; i++) {
        syslog(LOG_INFO, "starting output plugin: %s (ID: %02d)", global.out[i].plugin, global.out[i].param.id);
        global.out[i].run(global.out[i].param.id);
    }

    /* wait for signals */
    pause();

    return 0;
}
