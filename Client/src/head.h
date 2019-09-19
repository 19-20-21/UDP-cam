#ifndef _HEAD_H
#define _HEAD_H
 
       
#include <iostream>
#include <sys/types.h>         //socket所需头文件
#include <sys/socket.h>
 
//ip 地址转换所需头文件
#include <netinet/in.h>
#include <arpa/inet.h>   //htonl  htons 所需的头文件
 
#include <string.h>   //memset 所需头文件
#include <unistd.h>  //write read 所需头文件
 
#include <stdlib.h>	//malloc 所需头文件
 
#include <sys/stat.h>  //open 所需头文件
#include <fcntl.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>
#include <opencv2/highgui.hpp>// opencv
#include <opencv2/core.hpp>

#include <exception>

#include <pthread.h>
#include <queue>
#include "config.h"



struct UDP_pack
{
    uchar encode[EACH_PACK];
    int flag;//标志位,0代表包头，1代表包腹，2代表包尾
    int pack_id;//第几个包
    int pack_len;//总的包数，也同时作为当前包群的校验标准
};

struct MAT_withint
{
    cv::Mat thisMat;//Mat类
    uint frameInt;//该Mat的发送间隔
};
 
#endif
