#include <fcntl.h> 
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <stdio.h>
#include "common.h"

int fd;

//初始化串口
void init_serial(int fd);
/*发送指令*/
void send_cmd();
/*接收数据*/
int recv_data();

/**
功能：自定义读数据函数，确保数据读完整
参数：buf 数据缓冲区，即读到的数据保存到哪里去
参数：len 想要读的字节数
*/
void myread(char *buf,int len);


void *yanwu_pthread(void *arg)
{
	int ilux = 0;    //烟雾值
	pid_t buzzer_on_pid = -1;

	// if(argc != 2)
	// {
	// 	fprintf(stderr,"Usage: %s <serialName> \n",argv[0]);
	// 	return 1;
	// }

	//1.打开串口 // argv[1]: /dev/ttySAC1
	fd = open("/dev/ttySAC1",O_RDWR);
	if(fd == -1)
	{
		perror("open serial error");
		return (void*)1;
	}
	//2.初始化串口
	init_serial(fd);
	Lcd_Init();    //初始化显示屏
	Lcd_Draw_Rect(0, 0, 800, 480, 0x00ff0000);    //屏幕全红
	

	while(1)
	{
		//3.发送数据
		send_cmd();
		sleep(1);
		ilux = recv_data();
		if(ilux >= 128){
			if(buzzer_on_pid == -1){
				buzzer_on_pid = fork();
				if(buzzer_on_pid == 0){
					execlp("./buzzer", "buzzer", (char*)NULL);
					exit(EXIT_FAILURE);
				}
				else if(buzzer_on_pid < 0){
					perror("fork failed");
					exit(EXIT_FAILURE);
				}
				else{
					printf("蜂鸣器打开成功 ！\n");
				}
			}
			sleep(10);
			if(buzzer_on_pid != -1){
				kill(buzzer_on_pid, SIGTERM);
				waitpid(buzzer_on_pid, NULL, 0);
				buzzer_on_pid = -1;
			}
		}
		printf("当前烟雾值为：%d\n", ilux);
		printf("------------------\n");
		Lcd_Draw_Rect(28 + 48*3, 0, 800-(28 + 48*3), 480, 0x00000000);    //右屏幕全黑
		show_num_word(ilux, 3, 6);
		sleep(1);
	}
	
	//4.关闭串口
	close(fd);

	return 0;
}


/* 设置参数:9600速率 */
void init_serial(int fd)
{    
	//声明设置串口的结构体
	struct termios termios_new;
	//先清空该结构体
	bzero( &termios_new, sizeof(termios_new));
	//	cfmakeraw()设置终端属性，就是设置termios结构中的各个参数。
	
	cfmakeraw(&termios_new);
	//设置波特率
	//termios_new.c_cflag=(B9600);
	cfsetispeed(&termios_new, B9600);
	cfsetospeed(&termios_new, B9600);
	//CLOCAL和CREAD分别用于本地连接和接受使能，因此，首先要通过位掩码的方式激活这两个选项。    
	termios_new.c_cflag |= CLOCAL | CREAD;
	//通过掩码设置数据位为8位
	termios_new.c_cflag &= ~CSIZE;
	termios_new.c_cflag |= CS8; 
	//设置无奇偶校验
	termios_new.c_cflag &= ~PARENB;
	//一位停止位
	termios_new.c_cflag &= ~CSTOPB;
	
	tcflush(fd,TCIFLUSH);
	// 可设置接收字符和等待时间，无特殊要求可以将其设置为0
	termios_new.c_cc[VTIME] = 10;
	termios_new.c_cc[VMIN] = 1;
	// 用于清空输入/输出缓冲区
	tcflush (fd, TCIFLUSH);
	//完成配置后，可以使用以下函数激活串口设置
	if(tcsetattr(fd,TCSANOW,&termios_new) )
		printf("Setting the serial1 failed!\n");

}

void send_cmd()
{
	char cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
	write(fd,cmd,9);
}

int recv_data()
{
	char buf_yanwu[9] = {0};

	myread(buf_yanwu,9);

	int ilux = buf_yanwu[2]<<8 | buf_yanwu[3];
	//printf("yanwuzhi = %d\n",ilux);
	//printf("------------------\n");

	return ilux;
}


void myread(char *buf,int len)
{
	int ret = 0;
	int total = 0;
	while(total < len)
	{
		ret = read(fd,buf+total,len-total);
		total += ret;
	}
}

