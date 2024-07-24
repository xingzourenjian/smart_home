#include "wenshidu.h"

int fd_tty = 0;

void send_cmd_to_tty(int cmd[3], int len)
{
    int ret = write(fd_tty, cmd, len);
    if(ret == -1){
        perror("写入失败\n");
    }
}

void receive_tty_data(double data[4], int *h)
{
    int temp = 0;
    char tty_data[9] = {0};    //不能是int，数据类型要匹配
    char tty_data1[15] = {0};

    int ret = read(fd_tty, tty_data, 9);    //接收数据
    if(ret == -1){
        perror("读取失败\n");
    }
    ret = read(fd_tty, tty_data1, 15);
    if(ret == -1){
        perror("读取失败\n");
    }

// 光照：14.400000
// 温度：26.220000 气压：100496.440000     湿度：74.390000 海拔：69
    Lcd_Draw_Rect(28 + 48*3, 0, 800-(28 + 48*3), 480, 0x00000000);    //右屏幕全黑

    if(tty_data[2] == 0x15){    //光照
        temp = tty_data[4]<<24 | tty_data[5]<<16 | tty_data[6]<<8 | tty_data[7];
        show_num_word((int)temp, 4, 1);
        data[0] = (double)temp/100;
        printf("光照：%lf\n", data[0]);
        printf("------------------\n");
    }
    if(tty_data1[2] == 0x45){
        temp = tty_data1[4]<<8 | tty_data1[5];    //温度
        show_num_word((int)temp, 4, 2);
        data[1] = (double)temp/100;
        temp = tty_data1[6]<<24 | tty_data1[7]<<16 | tty_data1[8]<<8 | tty_data1[9];    //气压
        show_num_word((int)temp, 8, 3);
        data[2] = (double)temp/100;
        temp = tty_data1[10]<<8 | tty_data1[11];    //湿度
        show_num_word((int)temp, 4, 4);
        data[3] = (double)temp/100;
        *h = tty_data1[12]<<8 | tty_data1[13];    //海拔
        show_num_word(*h, 2, 5);
        printf("温度：%lf\t气压：%lf\t湿度：%lf\t海拔：%d\n", data[1], data[2], data[3], *h);
        printf("------------------\n");
    } 
}

void *wenshidu_pthread(void *arg)
{
    double data[4] = {0};
    int h = 0;
    int cmd[3] = {0xa5 , 0x83 ,0x28};

    Lcd_Init();    //初始化显示屏
    fd_tty = Uart_Init("/dev/ttySAC2", 9600);    //初始化串口
    Lcd_Draw_Rect(0, 0, 800, 480, 0x00ff0000);    //屏幕全红
    show_wenshidu_word();

    send_cmd_to_tty(cmd, 3);
    while(1){
        receive_tty_data(data, &h);
        sleep(1);
    }
}