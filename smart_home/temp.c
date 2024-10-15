#if 0
    int fd_color = open("/dev/fb0", O_RDWR);
    if(fd_color == -1){
        perror("打开失败\n");
    }

    int *flcd = mmap(NULL, 800*480*4, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, fd_color, 0);
    while(1){
        for(int y=0; y<480; y++){
            for(int x=0; x<800; x++){
                *(flcd+800*y+x) = 0x00ff0000;
            }
        }
        sleep(1);
        for(int y=0; y<480; y++){
            for(int x=0; x<800; x++){
                *(flcd+800*y+x) = 0x0000ff00;
            }
        }
        sleep(1);
        for(int y=0; y<480; y++){
            for(int x=0; x<800; x++){
                *(flcd+800*y+x) = 0x000000ff;
            }
        }
        sleep(1);
    }

    munmap(flcd, 800*480*4);
    close(fd_color);

    return 0;
#endif
/*
struct fb_var_screeninfo varinfo; //可变属性
*/
#if 0
    int color[3] ={0x00ff0000, 0x0000ff00, 0x000000ff};

    int fd_color = open("/dev/fb0", O_RDWR);
    if(fd_color == -1){
        perror("打开失败\n");
    }

    int *flcd = mmap(NULL, 800*480*4*2, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, fd_color, 0);

    if(ioctl(fd_color,FBIOGET_VSCREENINFO,&varinfo )!=0){
        printf("获取屏幕参数失败\n");
    }
    varinfo.xoffset = 0;
    varinfo.yoffset = 0;

    for(int k=0,n=0; ; k++,n++){
        for(int y=0; y<varinfo.yres; y++){
            for(int x=0; x<varinfo.xres; x++){
                *(flcd+varinfo.xres*(varinfo.yres*(n%2)+y)+x) = color[k%3];
            }
        }

        varinfo.xoffset = 0;
        varinfo.yoffset = varinfo.yres*(n%2);    //n选择区域
        ioctl(fd_color, FBIOPAN_DISPLAY, &varinfo);    //选择展示偏离的页面
        sleep(1);
    }

    munmap(flcd, 800*480*4*2);
    close(fd_color);

    return 0;
#endif