/*******************************************************************************
#                                                                              #
#      MJPG-streamer allows to stream JPG frames from an input-plugin          #
#      to several output plugins                                               #
#                                                                              #
#      Copyright (C) 2007 Tom Stöveken                                         #
#                                                                              #
# This program is free software; you can redistribute it and/or modify         #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; version 2 of the License.                      #
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

#include "../mjpg_streamer.h"
#include "../utils.h"
#include "httpd.h"

#define OUTPUT_PLUGIN_NAME "HTTP output plugin"
/*
 * keep context for each server
 */
context servers[MAX_OUTPUT_PLUGINS];

/*** plugin interface functions ***/
/******************************************************************************
Description.: Initialize this plugin.
              parse configuration parameters,
              store the parsed values in global variables
Input Value.: All parameters to work with.
              Among many other variables the "param->id" is quite important -
              it is used to distinguish between several server instances
Return Value: 0 if everything is OK, other values signal an error
******************************************************************************/
int output_init(output_parameter *param, int id)
{
    int i;
    int  port;
    char *credentials, *www_folder;
    char nocommands;

    DBG("output #%02d\n", param->id);

    port = htons(8080);
    credentials = NULL;
    www_folder = NULL;
    nocommands = 0;

    servers[param->id].id = param->id;
    servers[param->id].pglobal = param->global;
    servers[param->id].conf.port = port;
    servers[param->id].conf.credentials = credentials;
    servers[param->id].conf.www_folder = www_folder;
    servers[param->id].conf.nocommands = nocommands;

    OPRINT("www-folder-path...: %s\n", (www_folder == NULL) ? "disabled" : www_folder);
    OPRINT("HTTP TCP port.....: %d\n", ntohs(port));
    OPRINT("username:password.: %s\n", (credentials == NULL) ? "disabled" : credentials);
    OPRINT("commands..........: %s\n", (nocommands) ? "disabled" : "enabled");
    return 0;
}

/******************************************************************************
Description.: this will stop the server thread, client threads
              will not get cleaned properly, because they run detached and
              no pointer is kept. This is not a huge issue, because this
              funtion is intended to clean up the biggest mess on shutdown.
Input Value.: id determines which server instance to send commands to
Return Value: always 0
******************************************************************************/
int output_stop(int id)
{

    DBG("will cancel server thread #%02d\n", id);
    pthread_cancel(servers[id].threadID);

    return 0;
}

/******************************************************************************
Description.: This creates and starts the server thread
Input Value.: id determines which server instance to send commands to
Return Value: always 0
******************************************************************************/
int output_run(int id)
{
    DBG("launching server thread #%02d\n", id);

    /* create thread and pass context to thread function */
    pthread_create(&(servers[id].threadID), NULL, server_thread, &(servers[id]));
    pthread_detach(servers[id].threadID);

    return 0;
}

/******************************************************************************
Description.: This is just an example function, to show how the output
              plugin could implement some special command.
              If you want to control some GPIO Pin this is a good place to
              implement it. Dont forget to add command types and a mapping.
Input Value.: cmd is the command type
              id determines which server instance to send commands to
Return Value: 0 indicates success, other values indicate an error
******************************************************************************/
int output_cmd(int plugin, unsigned int control_id, unsigned int group, int value)
{
    DBG("command (%d, value: %d) for group %d triggered for plugin instance #%02d\n", control_id, value, group, plugin);
    return 0;
}
