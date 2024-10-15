#ifndef __LCD_H__
#define __LCD_H__

#include <stdio.h>//printf/scanf...
#include <sys/types.h>//open
#include <sys/stat.h>//open
#include <fcntl.h>//open
#include <unistd.h>//read/write/close
#include <string.h>//strlen

#include <stdlib.h>//malloc
#include <sys/mman.h>//mmap
#include "text.h"

//LCD显示屏的宽和高
#define LCD_WIDTH           800
#define LCD_HEIGHT          480

/*
    Lcd_Init:对LCD屏幕进行初始化
        @return : 初始化成功返回0,初始化失败返回-1
*/
int Lcd_Init(void);

/*
    Lcd_Draw_Rect:在屏幕上画矩形
        @x0,y0  : 矩形的起始(左上顶点)坐标
        @w      : 要画的矩形的宽
        @h      : 要画的矩形的高
        @color  : 要画的矩形的颜色
*/
void Lcd_Draw_Rect(int x0,int y0,int w,int h,int color);

/*
    Lcd_Display_Bmp:在LCD指定的位置上显示一张BMP图片
        @x0,y0  : 表示从哪个位置开始显示图片(左上顶点的坐标)
        @bmpfile: 要显示的图片的路径
        @return : 显示成功返回0,显示失败返回-1
*/
int Lcd_Display_Bmp(int x0,int y0,char *bmpfile);

/*
    Lcd_Draw_Point:在屏幕上画点
        @x,y    : 要显示的像素点的坐标
        @color  : 要显示的像素点的颜色
*/
void Lcd_Draw_Point(int x,int y,int color);

/*
    Lcd_Uinit:对屏幕资源的回收
*/
void Lcd_Uinit(void);

/*
   Lcd_Draw_Word:在位置为(x0,y0)处显示宽为w,高为h,颜色为color的汉字
*/
void Lcd_Draw_Word(int x0,int y0,int w,int h,unsigned char *data,int color);

void show_wenshidu_word(void);    //显示文字：光照、温度、气压、湿度、海拔

void show_num_word(int num, int len, int y);    //显示数字文字

#endif