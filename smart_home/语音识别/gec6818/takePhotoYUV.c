//#include <termios.h>

#include "camera.h"
#include "camera_common.h"

#define SCREENSIZE 800*480*4

#define MIN(a, b) \
	({ \
		typeof(a) _a = a; \
		typeof(b) _b = b; \
		(void)(&_a==&_b); \
		_a < _b ? _a : _b; \
	})

int redoffset  ;
int greenoffset;
int blueoffset ;

pthread_t tid;

//全局变量
int fd;  //表示屏幕的文件描述符
int *plcd = NULL; //屏幕映射后的内存首地址

int lcd;
struct fb_var_screeninfo lcdinfo;
uint8_t *fb;

int SCREEN_W, SCREEN_H;
int CAMERA_W, CAMERA_H;

int R[256][256];
int G[256][256][256];
int B[256][256];

sem_t s;

uint8_t *gyuv;


void display(uint8_t *yuv)
{
	gyuv = yuv;

	static uint32_t shown = 0;

	// int R0;
	// int G0;
	// int B0;
	//int R1, G1, B1;

	uint8_t Y0, U;
	uint8_t Y1, V;

	int w = MIN(SCREEN_W, CAMERA_W);
	int h = MIN(SCREEN_H, CAMERA_H);

	uint8_t *fbtmp = fb;
	int yuv_offset, lcd_offset;
	for(int y=0; y<h; y++)
	{
		for(int x=0; x<w; x+=2)
		{
			yuv_offset = ( CAMERA_W*y + x ) * 2;
			lcd_offset = ( SCREEN_W*y + x ) * 4;
			
			Y0 = *(yuv + yuv_offset + 0);
			U  = *(yuv + yuv_offset + 1);
			Y1 = *(yuv + yuv_offset + 2);
			V  = *(yuv + yuv_offset + 3);

			*(fbtmp + lcd_offset + redoffset  +0) = R[Y0][V];
			*(fbtmp + lcd_offset + greenoffset+0) = G[Y0][U][V];
			*(fbtmp + lcd_offset + blueoffset +0) = B[Y0][U];

			*(fbtmp + lcd_offset + redoffset  +4) = R[Y1][V];
			*(fbtmp + lcd_offset + greenoffset+4) = G[Y1][U][V];
			*(fbtmp + lcd_offset + blueoffset +4) = B[Y1][U];
		}
	}
	shown++;
}


void *camera_pthread(void *arg)
{
	//打开屏幕设备,获取LCD显示器的设备参数
	//====================================================	
	// 打开LCD设备
	lcd = open("/dev/fb0", O_RDWR);
	if(lcd == -1)
	{
		perror("open \"/dev/fb0\" failed");
		exit(0);
	}

	// 获取LCD显示器的设备参数
	ioctl(lcd, FBIOGET_VSCREENINFO, &lcdinfo);

	SCREEN_W = lcdinfo.xres;
	SCREEN_H = lcdinfo.yres;
	fb = mmap(NULL, lcdinfo.xres* lcdinfo.yres * lcdinfo.bits_per_pixel/8,
				    PROT_READ | PROT_WRITE, MAP_SHARED, lcd, 0);
	if(fb == MAP_FAILED)
	{
		perror("mmap failed");
		exit(0);
	}
	bzero(fb,  lcdinfo.xres * lcdinfo.yres * 4);

	//获取RGB偏移量
	redoffset  = lcdinfo.red.offset/8;
	greenoffset= lcdinfo.green.offset/8;
	blueoffset = lcdinfo.blue.offset/8;
	//====================================================
	//int *plcd = (int *)fb;
	fd = lcd;

	//准备好YUV---->RGB映射表
	//创建一个线程去执行convert函数
	//pthread_t tid;
	pthread_create(&tid, NULL, convert, NULL);

	// 打开摄像头设备文件
	int camfd = open("/dev/video7",O_RDWR);
	if(camfd == -1)
	{
		printf("open \"/dev/video7\" faield: %s\n", strerror(errno));
		exit(0);
	}
	

	//获取摄像头的基本参数
	printf("\n摄像头的基本参数：\n");
	get_camcap(camfd);

	get_camfmt(camfd);

	// 配置摄像头的采集格式
	get_caminfo(camfd);  //获取摄像头的采集信息(视频格式信息)
	set_camfmt(camfd);	//设置摄像头的信息
	get_camfmt(camfd);	//再次获取,看有没有成功

	CAMERA_W = fmt.fmt.pix.width;
	CAMERA_H = fmt.fmt.pix.height;

	// 设置即将要申请的摄像头缓存的参数
	int nbuf = 3;

	struct v4l2_requestbuffers reqbuf;
	bzero(&reqbuf, sizeof (reqbuf));
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	reqbuf.count = nbuf;

	// 使用该参数reqbuf来申请缓存
	ioctl(camfd, VIDIOC_REQBUFS, &reqbuf);

	// 根据刚刚设置的reqbuf.count的值，来定义相应数量的struct v4l2_buffer
	// 每一个struct v4l2_buffer对应内核摄像头驱动中的一个缓存
	struct v4l2_buffer buffer[nbuf];
	int length[nbuf];
	uint8_t *start[nbuf];

	for(int i=0; i<nbuf; i++)
	{
		bzero(&buffer[i], sizeof(buffer[i]));
		buffer[i].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buffer[i].memory = V4L2_MEMORY_MMAP;
		buffer[i].index = i;
		ioctl(camfd, VIDIOC_QUERYBUF, &buffer[i]);

		length[i] = buffer[i].length;
		start[i] = mmap(NULL, buffer[i].length,	PROT_READ | PROT_WRITE,
				        MAP_SHARED,	camfd, buffer[i].m.offset);

		ioctl(camfd , VIDIOC_QBUF, &buffer[i]);
	}

	//启动摄像头数据采集
	enum v4l2_buf_type vtype= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl(camfd, VIDIOC_STREAMON, &vtype);

	struct v4l2_buffer v4lbuf;
	bzero(&v4lbuf, sizeof(v4lbuf));
	v4lbuf.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4lbuf.memory= V4L2_MEMORY_MMAP;
	
	// 开始抓取摄像头数据并在屏幕播放视频
	int i=0;
	while(1)
	{
		// 从队列中取出填满数据的缓存
		v4lbuf.index = i%nbuf;
		ioctl(camfd , VIDIOC_DQBUF, &v4lbuf);

		display(start[i%nbuf]);
	 	// 将已经读取过数据的缓存块重新置入队列中 
		v4lbuf.index = i%nbuf;
		ioctl(camfd , VIDIOC_QBUF, &v4lbuf);
		i++;
	}

	return 0;
}

