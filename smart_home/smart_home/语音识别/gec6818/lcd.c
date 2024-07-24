#include "lcd.h"

#define SCREEN_PATH "/dev/fb0"    //屏幕文件的路径
//#define SCREEN_PATH "/dev/ubuntu_lcd"

static int Fd_Lcd = -1;    //用来保存屏幕文件的文件描述符
static int *Plcd = NULL;    //plcd用来指向屏幕文件的映射区域                                         

/*
    Lcd_Init:对LCD屏幕进行初始化
        @return : 初始化成功返回0,初始化失败返回-1
*/
int Lcd_Init(void)
{
    //1.打开屏幕文件
    Fd_Lcd = open(SCREEN_PATH,O_RDWR);
    if(Fd_Lcd == -1)
    {
        perror("Init Lcd Failed");
        return -1;
    }

    //2.映射屏幕文件到内存中去
    Plcd = mmap(NULL,LCD_HEIGHT*LCD_WIDTH*4,PROT_READ | PROT_WRITE,MAP_SHARED,Fd_Lcd,0);
    if(Plcd == MAP_FAILED)
    {
        perror("Init Lcd Failed");
        close(Fd_Lcd);
        return -1;
    }
    return 0;
}

/*
    Lcd_Draw_Rect:在屏幕上画矩形
        @x0,y0  : 矩形的起始(左上顶点)坐标
        @w      : 要画的矩形的宽
        @h      : 要画的矩形的高
        @color  : 要画的矩形的颜色
*/
void Lcd_Draw_Rect(int x0,int y0,int w,int h,int color)
{
    int x,y;
    for(y = y0;y < y0 + h;y++)
    {
        for(x = x0;x < x0 + w;x++)
        {
            Lcd_Draw_Point(x,y,color);
        }
    }
}

/*
    Lcd_Draw_Point:在屏幕上画点
        @x,y    : 要显示的像素点的坐标
        @color  : 要显示的像素点的颜色
*/
void Lcd_Draw_Point(int x,int y,int color)
{
    if(x >= 0 && x < LCD_WIDTH && y >= 0 && y < LCD_HEIGHT)
    {
        *(Plcd + LCD_WIDTH * y + x) = color;
    }
}

/*
    Lcd_Uinit:对屏幕资源的回收
*/
void Lcd_Uinit(void)
{
    munmap(Plcd,LCD_HEIGHT*LCD_WIDTH*4);
    close(Fd_Lcd);
}

/*
    Lcd_Display_Bmp:在LCD指定的位置上显示一张BMP图片
        @x0,y0  : 表示从哪个位置开始显示图片(左上顶点的坐标)
        @bmpfile: 要显示的图片的路径
        @return : 显示成功返回0,显示失败返回-1
*/
int Lcd_Display_Bmp(int x0,int y0,char *bmpfile)
{
    //打开BMP文件
    FILE *fd_bmp = fopen(bmpfile,"r");
    if(fd_bmp == NULL)
    {
        perror("Open BMP File Failed");
        return -1;
    }
 
    unsigned char buf[4] = {0};
    //读取bmpfile指向的文件中的最前面的两个字节
    int ret = fread(buf,1,2,fd_bmp);
    if(ret == -1)
    {
        perror("Read BMP File Failed");
        return -1;
    }
    
    //判断bmpfile指向的文件是不是BMP文件
    if(buf[0] == 0x42 && buf[1] == 0x4D)
    {
        //意味bmpfile指向的文件是BMP文件
    
        //1.读取BMP文件中的宽度
        //1.1 将光标偏移至宽度数据之前
        fseek(fd_bmp,0x12,SEEK_SET);
    
        //1.2 读取宽度数据
        ret = fread(buf,1,4,fd_bmp);
        if(ret == -1)
        {
            perror("Read BMP Width Failed");
            return -1;
        }
    
        //1.3 对宽度数据进行还原 4个char --> int
        int width = (buf[0]) | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
    
        //2.读取BMP文件中的高度
        //2.1 将光标偏移至高度数据之前
        fseek(fd_bmp,0x16,SEEK_SET);
    
        //2.2 读取高度数据
        ret = fread(buf,1,4,fd_bmp);
        if(ret == -1)
        {
            perror("Read BMP Height Failed");
            return -1;
        }
    
        //2.3 对高度数据进行还原 4个char --> int
        int height = (buf[0]) | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);        
    
        //3.读取BMP文件中的色深
        //3.1 将光标偏移至色深数据之前
        fseek(fd_bmp,0x1C,SEEK_SET);
    
        //3.2 读取色深数据
        ret = fread(buf,1,2,fd_bmp);
        if(ret == -1)
        {
            perror("Read BMP Depth Failed");
            return -1;
        }
    
        //3.3 对色深数据进行还原 2个char --> short
        int depth = (buf[0]) | (buf[1] << 8);
    
        //printf("%s height is %d,width is %d,depth is %d\n",bmpfile,width,height,depth);
    
        //求出BMP文件中像素数组一行的字节大小
        unsigned int bmp_bytes_perline = abs(width) * (depth / 8);
    
        unsigned int laizi = 0;
        //求出赖子个数
        if(bmp_bytes_perline % 4)
        {
            laizi = 4 - bmp_bytes_perline % 4;
        }
    
        //求出内存中用来保存像素数组的空间的一行大小
        unsigned int mem_bytes_perline = bmp_bytes_perline + laizi;
    
        //求出内存中用来保存像素数组的空间大小
        unsigned int size = mem_bytes_perline * abs(height);
    
        //开辟用来保存像素数组的空间
        unsigned char *pixel = malloc(size);
    
        //将光标偏移至像素数组之前
        fseek(fd_bmp,54,SEEK_SET);
        
        //将像素数组读取出来保存到开辟出来的空间中去
        ret = fread(pixel,1,size,fd_bmp);
        if(ret == -1)
        {
            perror("Read Pixel Array Failed");
            return -1;
        }

        //对数据进行处理
        int x,y;
        unsigned char a,r,g,b;
        int i = 0;
        unsigned int color;
        //对BMP文件中的像素点进行遍历
        for(y = 0;y < abs(height);y++)
        {
            for(x = 0;x < abs(width);x++)
            {
                //每次从piexl指向的空间中提取三个分量R G B
                b = pixel[i++];
                g = pixel[i++];
                r = pixel[i++];
                if(depth == 24)
                {
                    a = 0x00;
                }
                else if(depth == 32)
                {
                    a = pixel[i++];
                }
                else
                {
                    printf("Unsupported BMP File Format!\n");
                    return -1;
                }

                //将其整合成一个像素点的数据
                color = (a << 24) | (r << 16) | (g << 8) | b;

                //将这个像素点显示到屏幕的对应位置上去
                Lcd_Draw_Point((width > 0) ? (x0 + x) : (x0 + width - x - 1),(height > 0) ? (y0 + height - y - 1) : (y0 + y),color);
            }
            i = i + laizi;//处理掉赖子,不能把赖子写入到屏幕中去
        }

        fclose(fd_bmp);
        free(pixel);

        return 0;
    }
    else
    {
        //意味bmpfile指向的文件是不是BMP文件
        printf("%s is not a BMP File!\n",bmpfile);
        return -1;
    }
}

/*
   Lcd_Draw_Word:在位置为(x0,y0)处显示宽为w,高为h,颜色为color的汉字
*/
void Lcd_Draw_Word(int x0,int y0,int w,int h,unsigned char *data,int color)
{
	int i,k;
	
	for(i = 0;i < w*h/8;i++)
	{
		for(k = 0;k < 8;k++)
		{
			if((data[i]<<k) & 0x80)
			{
				Lcd_Draw_Point(x0+(i*8+k)%w,y0+i/(w/8),color);
			}
		}			
	}
}

void show_wenshidu_word(void)    //显示文字：光照、温度、气压、湿度、海拔
{
    int i = 0;

    for(i=0; i<2; i++){    //光照
        Lcd_Draw_Word(28 + (48*i), 28, 48, 43, wenshidu_font[i], 0x00ffffff);    //每个字间隔48像素，字体白色
    }
    Lcd_Draw_Word(28 + (48*i), 28, 48, 43, wenshidu_font[9], 0x00ffffff);
    for(i=2; i<4; i++){    //温度
        Lcd_Draw_Word(28 + (48*(i-2)), 28+43, 48, 43, wenshidu_font[i], 0x00ffffff);
    }
    Lcd_Draw_Word(28 + (48*(i-2)), 28+43, 48, 43, wenshidu_font[9], 0x00ffffff);
    for(i=4; i<6; i++){    //气压
        Lcd_Draw_Word(28 + (48*(i-4)), 28+43*2, 48, 43, wenshidu_font[i], 0x00ffffff);
    }
    Lcd_Draw_Word(28 + (48*(i-4)), 28+43*2, 48, 43, wenshidu_font[9], 0x00ffffff);
    for(i=0; i<2; i++){    //湿度
        Lcd_Draw_Word(28, 28+43*3, 48, 43, wenshidu_font[6], 0x00ffffff);
        Lcd_Draw_Word(28 + 48, 28+43*3, 48, 43, wenshidu_font[3], 0x00ffffff);
    }
    Lcd_Draw_Word(28 + 48*2, 28+43*3, 48, 43, wenshidu_font[9], 0x00ffffff);
    for(i=7; i<9; i++){    //海拔
        Lcd_Draw_Word(28 + (48*(i-7)), 28+43*4, 48, 43, wenshidu_font[i], 0x00ffffff);
    }
    Lcd_Draw_Word(28 + (48*(i-7)), 28+43*4, 48, 43, wenshidu_font[9], 0x00ffffff);
}

void show_num_word(int num, int len, int y){
    int i = len;

    while(i){
        switch(num%10){//x 28 + 48*3
            case 0: Lcd_Draw_Word(28 + (48*(2+i)), 28+43*(y-1), 24, 43, num_font[0], 0x00ffffff);break;
            case 1: Lcd_Draw_Word(28 + (48*(2+i)), 28+43*(y-1), 24, 43, num_font[1], 0x00ffffff);break;
            case 2: Lcd_Draw_Word(28 + (48*(2+i)), 28+43*(y-1), 24, 43, num_font[2], 0x00ffffff);break;
            case 3: Lcd_Draw_Word(28 + (48*(2+i)), 28+43*(y-1), 24, 43, num_font[3], 0x00ffffff);break;
            case 4: Lcd_Draw_Word(28 + (48*(2+i)), 28+43*(y-1), 24, 43, num_font[4], 0x00ffffff);break;
            case 5: Lcd_Draw_Word(28 + (48*(2+i)), 28+43*(y-1), 24, 43, num_font[5], 0x00ffffff);break;
            case 6: Lcd_Draw_Word(28 + (48*(2+i)), 28+43*(y-1), 24, 43, num_font[6], 0x00ffffff);break;
            case 7: Lcd_Draw_Word(28 + (48*(2+i)), 28+43*(y-1), 24, 43, num_font[7], 0x00ffffff);break;
            case 8: Lcd_Draw_Word(28 + (48*(2+i)), 28+43*(y-1), 24, 43, num_font[8], 0x00ffffff);break;
            case 9: Lcd_Draw_Word(28 + (48*(2+i)), 28+43*(y-1), 24, 43, num_font[9], 0x00ffffff);break;
        }
        num/=10;
        i--;
    }
}