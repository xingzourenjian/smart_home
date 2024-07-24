#include "uart.h"

int fd = -1;
/*
    Uart_Init:用来初始化并配置串口协议
        @file       指定要使用的串口路径名
                    比如,要使用COM2时,则"/dev/ttySAC1"
        @baudrate   波特率,直接给数值就可以了
                    根据串口链接的模块来给定，一般为9600
        @返回值        失败返回-1
                    成功返回一个配置好的串口文件描述符
*/
int Uart_Init(const char *file, int baudrate)
{ 
    fd = open(file,O_RDWR);
    if(fd == -1)
    {
        perror("open device error:");
        return -1;
    }

    struct termios myserial;
    //清空结构体
    memset(&myserial, 0, sizeof (myserial));
    
    //O_RDWR               
    myserial.c_cflag |= (CLOCAL | CREAD);
    //设置控制模式状态，本地连接，接受使能
    //设置 数据位
    myserial.c_cflag &= ~CSIZE;   //清空数据位
    myserial.c_cflag &= ~CRTSCTS; //无硬件流控制
    myserial.c_cflag |= CS8;      //数据位:8

    myserial.c_cflag &= ~CSTOPB;//   //1位停止位
    myserial.c_cflag &= ~PARENB;  //不要校验
    //myserial.c_iflag |= IGNPAR;   //不要校验
    //myserial.c_oflag = 0;  //输入模式
    //myserial.c_lflag = 0;  //不激活终端模式

    switch (baudrate)
    {
        case 9600:
            cfsetospeed(&myserial, B9600);  //设置波特率
            cfsetispeed(&myserial, B9600);
            break;
        case 115200:
            cfsetospeed(&myserial, B115200);  //设置波特率
            cfsetispeed(&myserial, B115200);
            break;
        case 19200:
            cfsetospeed(&myserial, B19200);  //设置波特率
            cfsetispeed(&myserial, B19200);
            break;
    }
        /* 刷新输出队列,清楚正接受的数据 */
    tcflush(fd, TCIFLUSH);

    /* 改变配置 */
    tcsetattr(fd, TCSANOW, &myserial);

    return fd;
}

void uart_close(void)
{
    close(fd);
}