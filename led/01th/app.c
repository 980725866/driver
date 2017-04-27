#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEVICE_NAME "/dev/leds"

int main()
{
	int i;
	int fd = open(DEVICE_NAME,O_RDWR);
	while(1)
	{
		for(i=0;i<4;i++){//led on
			ioctl(fd,1,i);
		}
		sleep(2);
		for(i=0;i<4;i++){//led off
			ioctl(fd,0,i);
		}
		sleep(2);
	}
	return 0;
}

