#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_NAME "/dev/leds"

int main()
{
	char buf[512];
	int i;
	int fd = open(DEVICE_NAME,O_RDWR);
	
	for(i=0;i<3;i++){// read
		memset(buf,0,sizeof(buf));
		read(fd,buf,sizeof(buf));
		printf("read = %s\n",buf);
		
	}
	
	for(i=0;i<3;i++){//write
		memset(buf,0,sizeof(buf));
		sprintf(buf,"app to kernel %d",i);
		write(fd,buf,strlen(buf));
	}
	while(1)
	{
		for(i=0;i<4;i++){//ioctl on
			ioctl(fd,1,i);
		}
		sleep(1);
		for(i=0;i<4;i++){//ioctl off
			ioctl(fd,0,i);
		}
		sleep(1);
	}
	
	return 0;
}

