#include "common.h"

//#define REC_CMD  "./arecord -d4 -c1 -r16000 -traw -fS16_LE cmd.pcm"
#define REC_CMD  "arecord -d3 -c1 -r16000 -traw -fS16_LE cmd.pcm"
#define PCM_FILE "./cmd.pcm"
/* -d：录音时长（duration）
-c：音轨（channels）
-r：采样频率（rate）
-t：封装格式（type）
-f：量化位数（format） */

void catch (int sig)
{
	if (sig == SIGPIPE)
	{
		printf("killed by SIGPIPE\n");
		exit(0);
	}
}

int main(int argc, char const* argv[]) // ./voicectl 192.168.31.20
{
	pthread_t pth_id_yanwu;
	pthread_t pth_id_wenshidu;
	pthread_t pth_id_camera;
	pid_t player_pid = -1;
	pid_t camera_pid = -1;
	int ret = 0;

	Lcd_Init();    //初始化显示屏
	Lcd_Display_Bmp(0,0,"/lxk/jia.bmp");

	// if (argc != 2)
	// {
	// 	printf("Usage: %s <ubuntu-IP>\n", argv[0]);
	// 	exit(0);
	// }
	signal(SIGPIPE, catch);
	// 初始化TCP客户端套接字，用于连接到语音识别服务器(即ubuntu)
	int sockfd_tcp = init_tcp_socket("192.168.180.1");
	// 初始化本地UDP套接字
	int sockfd_udp = init_udp_socket();

	int id_num = -1; // 识别后的指令id
	while (1)
	{
		// 1，调用arecord来录一段音频
		printf("please to start REC in 3s...\n");

		// 在程序中执行一条命令  “录音的命令”
		system(REC_CMD);

				
		// 2，将录制好的PCM音频发送给语音识别引擎
		send_pcm(sockfd_tcp, PCM_FILE);

		// 3，等待对方回送识别结果（字符串ID）
		xmlChar* id = wait4id(sockfd_tcp);
		if (id == NULL)
		{
			continue;
		}


		id_num = atoi((char*)id);
		if (id_num == 999)
		{
			printf("bye-bye!\n");
			exit(1);
		}
		if(id_num == 1)
		{
    		ret = pthread_create(&pth_id_yanwu, NULL, yanwu_pthread, NULL);    //检测烟雾
			if(ret == -1){
				perror("创建线程失败");
				return -1;
			}
		}
		if(id_num == 2)
		{
			if(player_pid != -1){
				kill(player_pid, SIGTERM);
				waitpid(player_pid, NULL, 0);
				player_pid = -1;
			}
			Lcd_Display_Bmp(0,0,"/lxk/jia.bmp");
		}
		if(id_num == 3)
		{
    		//system("./mplayer1.4 wy.mp4");
			if(player_pid == -1){
				player_pid = fork();
				if(player_pid == 0){
					execlp("./mplayer1.4", "mplayer1.4", "-zoom", "-x", "800", "-y", "480", "./wy.mp4", (char*)NULL);
					exit(EXIT_FAILURE);
				}
				else if(player_pid < 0){
					perror("fork failed");
					exit(EXIT_FAILURE);
				}
				else{
					printf("打开视频成功 ！\n");
				}
			}
		}
		if(id_num == 4)
		{
    		ret = pthread_create(&pth_id_wenshidu, NULL, wenshidu_pthread, NULL);    //检测温湿度
			if(ret == -1){
				perror("创建线程失败");
				return -1;
			}
		}
		if(id_num == 5)
		{
    		pthread_cancel(pth_id_wenshidu);    //不检测温湿度了
			Lcd_Display_Bmp(0,0,"/lxk/jia.bmp");
		}
		if(id_num == 6)
		{
			pthread_cancel(pth_id_yanwu);    //不检测烟雾了
			Lcd_Display_Bmp(0,0,"/lxk/jia.bmp");
		}
		if(id_num == 7)
		{
    		// ret = pthread_create(&pth_id_camera, NULL, camera_pthread, NULL);    //打开摄像头
			// if(ret == -1){
			// 	perror("创建线程失败");
			// 	return -1;
			// }
			if(camera_pid == -1){
				camera_pid = fork();
				if(camera_pid == 0){
					execlp("./camera", "camera", (char*)NULL);
					exit(EXIT_FAILURE);
				}
				else if(camera_pid < 0){
					perror("fork failed");
					exit(EXIT_FAILURE);
				}
				else{
					printf("打开摄像头成功 ！\n");
				}
			}
		}
		if(id_num == 8)
		{
			// pthread_cancel(tid);
    		// pthread_cancel(pth_id_camera);    //关闭摄像头
			// pthread_join(pth_id_camera, NULL);
			//Lcd_Display_Bmp(0,0,"/lxk/jia.bmp");
			if(camera_pid != -1){
				kill(camera_pid, SIGTERM);
				waitpid(camera_pid, NULL, 0);
				camera_pid = -1;
			}
			Lcd_Display_Bmp(0,0,"/lxk/jia.bmp");
		}

		// udp发送数据给接收端, 接收端收到数据id后，再决定执行什么功能
		send_id(sockfd_udp, id_num);
		
	}

	return 0;
}
