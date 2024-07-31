#include "camera_common.h"
#include "camera.h"
//#include "jpeglib.h"

char formats[5][16] = {0};
struct v4l2_fmtdesc fmtdesc;
struct v4l2_format  fmt;
struct v4l2_capability cap;


int CAMERA_W, CAMERA_H;

int R[256][256];
int G[256][256][256];
int B[256][256];

// 获取摄像头格式信息（固定）
bool get_caminfo(int camfd)
{
    fmtdesc.index = 0;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    bool ret = false;
    printf("当前摄像头所支持的格式有: \n");
    while((ret=ioctl(camfd, VIDIOC_ENUM_FMT, &fmtdesc)) == 0)
    {
        printf("[%d]", fmtdesc.index);
        sprintf(formats[fmtdesc.index]+0, "%c", (fmtdesc.pixelformat>>8*0)&0xFF);
        sprintf(formats[fmtdesc.index]+1, "%c", (fmtdesc.pixelformat>>8*1)&0xFF);
        sprintf(formats[fmtdesc.index]+2, "%c", (fmtdesc.pixelformat>>8*2)&0xFF);
        sprintf(formats[fmtdesc.index]+3, "%c", (fmtdesc.pixelformat>>8*3)&0xFF);

        printf("\"%s\"", formats[fmtdesc.index]);
        printf("（详细描述: %s）\n", fmtdesc.description);
        fmtdesc.index++;
        ret = true;
    }
    printf("\n");

    return ret;
}

// 获取摄像头格式信息（可调）
bool get_camfmt(int camfd)
{
    bzero(&fmt, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if(ioctl(camfd, VIDIOC_G_FMT, &fmt) == -1)
    {
        printf("获取摄像头格式信息失败: %s\n", strerror(errno));
        return false;
    }

    printf("摄像头当前的配置信息:\n");
    printf("分辨率: %d×%d\n", fmt.fmt.pix.width, fmt.fmt.pix.height);

    printf("像素格式: ");
    switch(fmt.fmt.pix.pixelformat)
    {
    case V4L2_PIX_FMT_MJPEG:
        printf("V4L2_PIX_FMT_MJPEG\n");
        break;
    case V4L2_PIX_FMT_JPEG:
        printf("V4L2_PIX_FMT_JPEG\n");
        break;
    case V4L2_PIX_FMT_MPEG:
        printf("V4L2_PIX_FMT_MPEG\n");
        break;
    case V4L2_PIX_FMT_MPEG1:
        printf("V4L2_PIX_FMT_MPEG1\n");
        break;
    case V4L2_PIX_FMT_MPEG2:
        printf("V4L2_PIX_FMT_MPEG2\n");
        break;
    case V4L2_PIX_FMT_MPEG4:
        printf("V4L2_PIX_FMT_MPEG4\n");
        break;
    case V4L2_PIX_FMT_H264:
        printf("V4L2_PIX_FMT_H264\n");
        break;
    case V4L2_PIX_FMT_XVID:
        printf("V4L2_PIX_FMT_XVID\n");
        break;
    case V4L2_PIX_FMT_RGB24:
        printf("V4L2_PIX_FMT_RGB24\n");
        break;
    case V4L2_PIX_FMT_BGR24:
        printf("V4L2_PIX_FMT_BGR24\n");
        break;
    case V4L2_PIX_FMT_YUYV:
        printf("V4L2_PIX_FMT_YUYV\n");
        break;
    case V4L2_PIX_FMT_YYUV:
        printf("V4L2_PIX_FMT_YYUV\n");
        break;
    case V4L2_PIX_FMT_YVYU:
        printf("V4L2_PIX_FMT_YVYU\n");
        break;
    case V4L2_PIX_FMT_YUV444:
        printf("V4L2_PIX_FMT_YUV444\n");
        break;
    case V4L2_PIX_FMT_YUV410:
        printf("V4L2_PIX_FMT_YUV410\n");
        break;
    case V4L2_PIX_FMT_YUV420:
        printf("V4L2_PIX_FMT_YUV420\n");
        break;
    case V4L2_PIX_FMT_YVU420:
        printf("V4L2_PIX_FMT_YVU420\n");
        break;
    case V4L2_PIX_FMT_YUV422P:
        printf("V4L2_PIX_FMT_YUV422P\n");
        break;
    default:
        printf("未知\n");
    }
    printf("\n");

    return true;
}


// 获取摄像头设备的基本参数
bool get_camcap(int camfd)
{
    bzero(&cap, sizeof(cap));
    if(ioctl(camfd, VIDIOC_QUERYCAP, &cap) == -1)
    {
        printf("获取摄像头基本信息失败: %s\n", strerror(errno));
        return false;
    }

    printf("驱动：%s\n", cap.driver);
    printf("显卡：%s\n", cap.card);
    printf("总线：%s\n", cap.bus_info);
    printf("版本：%d\n", cap.version);

    if((cap.capabilities&V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE)
    {
        printf("该设备为视频采集设备\n");
    }
    if((cap.capabilities&V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING)
    {
        printf("该设备支持流IO操作\n\n");
    }

    return true;
}

// 配置摄像头像素格式
void set_camfmt(int camfd)
{
    struct v4l2_format *tmp = calloc(1, sizeof(*tmp));
    bzero(tmp, sizeof(*tmp));

    // 设置视频设备的类型：CAPTURE即摄像头
    tmp->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    // 试图设置摄像头的分辨率
    tmp->fmt.pix.width  = 800;
    tmp->fmt.pix.height = 480;

    //printf("请选择你要配置的序号:\n");
    int n = 0;
    //scanf("%d", &n);
    //getchar();

    if(!strncmp(formats[n], "JPEG", 4)) tmp->fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;
    else if(!strncmp(formats[n], "MJPG", 4)) tmp->fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    else if(!strncmp(formats[n], "MPEG", 4)) tmp->fmt.pix.pixelformat = V4L2_PIX_FMT_MPEG;
    else if(!strncmp(formats[n], "YUYV", 4)) tmp->fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    else if(!strncmp(formats[n], "YVYU", 4)) tmp->fmt.pix.pixelformat = V4L2_PIX_FMT_YVYU;
    else if(!strncmp(formats[n], "H264", 4)) tmp->fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
    else
    {
        printf("对不起，所选格式无法配置.\n");
        exit(0);
    }

    // 设置摄像头捕获图像的方式
    tmp->fmt.pix.field = V4L2_FIELD_INTERLACED;

    // 使得以上设置参数生效
    if(ioctl(camfd, VIDIOC_S_FMT, tmp) == -1)
    {
        printf("ioctl() VIDIOC_S_FMT 失败了: %s\n", strerror(errno));
    }
}


void *convert(void *arg __attribute__((unused)))
{
    /*******************************
     R = Y + 1.042*(V-128);
     G = Y - 0.344*(U-128)-0.714*(V-128);
     B = Y + 1.772*(U-128);
    *******************************/
    pthread_detach(pthread_self());  //防止僵尸线程

    for(int i=0; i<256; i++)  //i--->Y
    {
        for(int j=0; j<256; j++)  //j--->V   j--->U
        {
            R[i][j] = i + 1.042*(j-128);
            R[i][j] = R[i][j]>255 ? 255 : R[i][j];
            R[i][j] = R[i][j]<0   ? 0   : R[i][j];

            B[i][j] = i + 1.772*(j-128);
            B[i][j] = B[i][j]>255 ? 255 : B[i][j];
            B[i][j] = B[i][j]<0   ? 0   : B[i][j];

            for(int k=0; k<256; k++)   //j--->U    k--->V
            {
                G[i][j][k] = i + 0.344*(j-128)-0.714*(k-128);
                G[i][j][k] = G[i][j][k]>255 ? 255 : G[i][j][k];
                G[i][j][k] = G[i][j][k]<0   ? 0   : G[i][j][k];
            }
        }
    }
    pthread_exit(NULL);
}

