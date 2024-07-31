/*---------------------------------------
*功能描述:  蜂鸣器测试程序 
*创建者：   粤嵌技术部
*创建时间： 2015,01,01
---------------------------------------
*修改日志：
*修改内容：
*修改人：
*修改时间：
----------------------------------------*/
/*************************************************
*头文件
*************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//#define BUZ_ON   _IOW('B', 1, unsigned long)
//#define BUZ_OFF   _IOW('B', 0, unsigned long)

/*************************************************
*主程序
*************************************************/
int main(void)
{
	int fd;
	int ret;
	int i = 3;
	fd = open("/dev/beep", O_RDWR);            //打开设备，成功返回0
	if(fd<0){
		perror("open:");
		return -1;
	}
	
	while(i--) {
		printf("**********************buzzer On************************\n");
		ret = ioctl(fd, 0, 1);                //BUZZER on
		if(ret < 0){
			perror("ioctl:");
			return -1;
		}		
		sleep(1);
		printf("**********************buzzer off*************************\n");
		ret = ioctl(fd, 1, 1);                 //BUZZER off
		if(ret < 0){
			perror("ioctl:");
			return -1;
		}
		sleep(1);	
	}	
	
	close(fd);
	return 0;
}
