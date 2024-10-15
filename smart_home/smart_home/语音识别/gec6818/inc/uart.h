#ifndef __UART_H__
#define __UART_H__

#include <stdio.h>
#include <fcntl.h> 
#include <unistd.h>
#include <termios.h> 
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

/*
    Uart_Init:用来初始化并配置串口协议
        @file       指定要使用的串口路径名
                    比如,要使用COM2时,则"/dev/ttySAC1"
        @baudrate   波特率,直接给数值就可以了
                    根据串口链接的模块来给定，一般为9600
        @返回值        失败返回-1
                    成功返回一个配置好的串口文件描述符
*/
int Uart_Init(const char *file, int baudrate);

void uart_close(void);

#endif