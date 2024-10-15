#ifndef __WENSHIDU_H__
#define __WENSHIDU_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>    //mmap
#include <linux/i2c.h>
#include <sys/ioctl.h>    //ioctl
#include <linux/fb.h>    //struct fb_var_screeninfo
#include "lcd.h"
#include "uart.h"
#include "text.h"
#include <pthread.h>

void *wenshidu_pthread(void *arg);


#endif