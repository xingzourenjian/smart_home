#include "lcd.h"
#include "bmp.h"

/*
    show_video_interface:显示视频播放的界面
    @plcd:屏幕映射区域的首地址
    返回值：void
*/
void show_video_interface(int *plcd)
{
    show_bmp("./1.bmp", 100, 380, plcd);//上一个视频
    show_bmp("./2.bmp", 300, 380, plcd);//暂停/继续
    show_bmp("./3.bmp", 500, 380, plcd);//下一个视频
}

/*
    play_video:播放视频
    @x:获取到的触摸屏x坐标
    @y:获取到的触摸屏y坐标
    @vedio_number:记录当前播放的是第几个视频
    @stop_flag:记录视频播放状态
*/
void play_vedio(int *x, int *y, int *video_num, int *stop_flag)
{
    char cmd[256] = {0};
    if(*x>100 && *x<200 && *y>380 && *y<480)//上一个视频
    {
        *x = -1, *y = -1;
        system("killall -9 mplayer");//杀死原来播放视频的进程
        (*video_num)--;
        if(*video_num<0)
        {
            *video_num = 2;
        }
        sprintf(cmd, "maplayer -slave -quiet -geometry 0:0 -zoom -x 800 -y 480 ./%d.mp4 &", *video_num);
        system(cmd);
    }
    else if(*x>500 && *x<600 && *y>380 && *y<480)//下一个视频
    {
        *x = -1, *y = -1;
        system("killall -9 mplayer");//杀死原来播放视频的进程
        (*video_num)++;
        if(*video_num>2)
        {
            *video_num = 0;
        }
        sprintf(cmd, "maplayer -slave -quiet -geometry 0:0 -zoom -x 800 -y 480 ./%d.mp4 &", *video_num);
        system(cmd);
    }
    else if(*x>300 && *x<400 && *y>380 && *y<480 && *stop_flag == 0)//暂停播放
    {
        *x = -1, *y = -1;
        system("killall -STOP mplayer");//暂停播放
        *stop_flag = 1;
    }
    else if(*x>300 && *x<400 && *y>380 && *y<480 && *stop_flag == 1)//继续播放
    {
        *x = -1, *y = -1;
        system("killall -CONT mplayer");//暂停播放
        *stop_flag = 0;
    }
}