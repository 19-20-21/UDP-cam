#include "Client.h"
#include "config.h"

//全局变量
ClnPro clnpro;
UDPSock udpSock;
std::queue<cv::Mat> matQueue;
std::queue<struct MAT_withint> sendQueue;
//线程锁
pthread_mutex_t mutex;
pthread_mutex_t mutex2;
pthread_mutex_t mutex3;


void ClnPro::setRstpAddr(const char* rstpaddr){
    this->rstp_addr = rstpaddr;
}

ClnPro::ClnPro(){
    std::cout << "程序初始化完毕" << std::endl;
}

ClnPro::~ClnPro(){
    std::cout << "退出程序" << std::endl;
}

int ClnPro::openVideoStream(){
    if(cap.open(rstp_addr)==-1)
        {
            throw "摄像头打开失败，请检查rstp流地址！";
            exit(-1);
        }
    return 0;
}

int ClnPro::motionD(cv::Mat pre,cv::Mat curr){
    cv::Mat result = curr.clone();
    cv::Mat curimageROI = pre(cv::Rect(pre.cols / 5, pre.rows / 5, pre.cols / 1.5, pre.rows / 1.5));
    cv::Mat preimageROI = curr(cv::Rect(curr.cols / 5, curr.rows / 5, curr.cols / 1.5, curr.rows / 1.5));

    cv::Mat sm_pre, sm_cur;
    
    cv::GaussianBlur(preimageROI, sm_pre, cv::Size(13, 13),2,2);
    cv::GaussianBlur(curimageROI, sm_cur, cv::Size(13, 13),2,2);
    //cv::imshow("平滑处理", sm_cur);

    cv::Mat gray_pre, gray_cur;
    cv::cvtColor(sm_pre, gray_pre, CV_RGB2GRAY);
    cv::cvtColor(sm_cur, gray_cur, CV_RGB2GRAY);
    //cv::imshow("灰度处理",gray_cur);

     cv::Mat opDiff;
    cv::absdiff(gray_pre,gray_cur,opDiff);
    //cv::imshow("帧差处理",opDiff);

    cv::Mat ez;
    threshold(opDiff, ez,30, 255, CV_THRESH_BINARY);
    //cv::imshow("二值化处理", ez);

//帧差法
    int diffNum=0;
	for (int i=0; i<ez.rows; i++)
		{
		uchar *data = ez.ptr<uchar>(i); //获取每一行的指针
		for (int j=0;j<ez.cols; j++)
		{
			if (data[j] == 255) //访问到像素值
				{
					diffNum++;
				}					
			}
		}

    /* cv::Mat pz;
    cv::Mat ele=cv::getStructuringElement(cv::MORPH_RECT, cv::Size(11, 30));
    dilate(ez, pz, ele);
    //cv::imshow("膨胀处理", pz);

    cv::Mat fs;
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(10, 16));
    erode(pz, fs, element);
    //cv::imshow("腐蚀处理", fs);
    */

    /* std::vector<std::vector<cv::Point> > contours;
    cv::findContours(fs, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    std::vector<cv::Rect> boundRect(contours.size());
    for (int i = 0; i < contours.size(); i++)
    {
        boundRect[i] = boundingRect(contours[i]);
        rectangle(result, cv::Rect(boundRect[i].x + curr.cols/5, boundRect[i].y + curr.rows/5, boundRect[i].width, boundRect[i].height), cv::Scalar(0, 255, 0), 1);//在result上绘制正外接矩形  
    }*/
    return diffNum;
}

//线程一，用来读取Mat，入队
void *t1_getMatFromCap(void *junk){
    cv::Mat frame,cleaner;
    while (1)
    {
        pthread_mutex_lock(&mutex3);
        clnpro.cap>>frame;
        pthread_mutex_unlock(&mutex3);
        if (frame.size().width==0)continue;//检查坏包
        if (matQueue.size()<=25)
        {
            pthread_mutex_lock(&mutex);
            matQueue.push(frame.clone());
            pthread_mutex_unlock(&mutex);
        }
        else
        {
            pthread_mutex_lock(&mutex);
            matQueue.pop();
            matQueue.push(frame.clone());
            pthread_mutex_unlock(&mutex);
        }
	usleep(10000);
    }
}

//线程二
void *t2_motionDetc(void *junk){
    struct MAT_withint pre,curr;
    uint diffNum;
    while (1)
    {
        std::cout << "处理前图片队列："<<matQueue.size() << std::endl;
        while (matQueue.size()<2)
        {
            sleep(1);
        }
        while (sendQueue.size()>20)
        {
                pthread_mutex_lock(&mutex2);
                sendQueue.pop();
                pthread_mutex_unlock(&mutex2);                       
        }
        if (sendQueue.empty())
        {
            pthread_mutex_lock(&mutex);
            pre.thisMat = matQueue.front().clone();
            matQueue.pop();
            curr.thisMat = matQueue.front().clone();
            matQueue.pop();
            pthread_mutex_unlock(&mutex);
            diffNum = clnpro.motionD(pre.thisMat,curr.thisMat);
            if (diffNum==0)
            {
                pre.frameInt = 5000;
                pthread_mutex_lock(&mutex2);
                sendQueue.push(pre);
                pthread_mutex_unlock(&mutex2);           
            }
            else
            {
                pre.frameInt = 100+(400/diffNum);
                curr.frameInt = 100+(400/diffNum);
                pthread_mutex_lock(&mutex2);
                sendQueue.push(pre);
                sendQueue.push(curr);
                pthread_mutex_unlock(&mutex2);                          
            }
        }
        else
        {
            pre.thisMat = sendQueue.back().thisMat.clone();
            curr.thisMat = matQueue.front().clone();
            diffNum = clnpro.motionD(pre.thisMat,curr.thisMat);
            if (diffNum==0)
            {
                curr.frameInt = 5000;
                pthread_mutex_lock(&mutex2);
                sendQueue.push(curr);
                pthread_mutex_unlock(&mutex2);           
            }
            else
            {
                curr.frameInt = 100+(400/diffNum);
                pthread_mutex_lock(&mutex2);
                sendQueue.push(curr);
                pthread_mutex_unlock(&mutex2);                          
            } 
        }        
    }
}


//线程三，封装，发送
void *t3_processAndSend(void *junk){
    std::vector < uchar > encoded;//装编码后的数据
    cv::Mat frame,send,skip;
    struct UDP_pack upkr;
    uint frameInt;
    clock_t last_cycle = clock(); 
    while (1)
    {
        std::cout << "处理后队列："<<sendQueue.size() << std::endl;
        while (sendQueue.empty())
        {
            sleep(1);
        }

        pthread_mutex_lock(&mutex2);
        frame = sendQueue.front().thisMat.clone();
        frameInt = sendQueue.front().frameInt;
        sendQueue.pop();
        pthread_mutex_unlock(&mutex2);
        cv::resize(frame, send, cv::Size(FRAME_WIDTH, FRAME_HEIGHT), 0, 0, cv::INTER_LINEAR);
        std::vector < int > compression_params;
        compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
        compression_params.push_back(ENCODE_QUALITY);

        cv::imencode(".jpg", send, encoded, compression_params);
        //cv::imshow("send", send);
        int total_pack = 1 + encoded.size() / EACH_PACK;//计算总的发送的包数量
        //打包发送包头
        memcpy(upkr.encode,&encoded[0],EACH_PACK);
        upkr.flag = 0;
        upkr.pack_id = 1;
        upkr.pack_len = total_pack;
        udpSock.SendSock(&upkr,ARMED_PACK);
        //打包发送包腹部
        for (int i = 2; i < total_pack; i++)
            {
                memcpy(upkr.encode,&encoded[(i-1)*EACH_PACK],EACH_PACK);
                upkr.flag = 1;
                upkr.pack_id = i;
                udpSock.SendSock(&upkr,ARMED_PACK);
            }
        //打包发送包尾
        memcpy(upkr.encode,&encoded[(total_pack-1)*EACH_PACK],EACH_PACK);
        upkr.flag = 2;
        upkr.pack_id = total_pack;
        udpSock.SendSock(&upkr,ARMED_PACK);
        pthread_mutex_lock(&mutex3);
        for (int i = 0; i < frameInt/300; i++)
        {
            clnpro.cap>>skip;
        }
        pthread_mutex_unlock(&mutex3);       
        usleep(1000*frameInt);
        std::cout<<"frameint:"<<frameInt<<std::endl;
        clock_t next_cycle = clock();
        double duration = (next_cycle - last_cycle) / (double) CLOCKS_PER_SEC;
        std::cout << "\teffective FPS:" << (1 / duration) << " \tkbps:" << (EACH_PACK * total_pack / duration / 1024 * 8) << std::endl;

        std::cout << next_cycle - last_cycle;
        last_cycle = next_cycle;
    }
    
}



int main(int argc, char * argv[]) {
    if ((argc != 4)) 
    { // 如果输入的参数数量不为3
        std::cerr << "格式: " << argv[0] << " <服务器地址> <服务器端口> <Rstp流地址>\n";
        exit(1);
    }

    clnpro.setRstpAddr(argv[3]);
    udpSock.ClnSockBind(argv);
    //cv::namedWindow("send", CV_WINDOW_AUTOSIZE);
    clnpro.openVideoStream();
    if(!clnpro.cap.isOpened()){
        std::cerr<<"摄像头开启失败！"<<std::endl;
        exit(-1);
    }
    else
    {
        std::cout << "初始化完毕，按任意键开始发送数据包" << std::endl;
        getchar();
    }
    pthread_t t1,t2,t3;
    int thread_feed_back;

    std::cout << "创建线程t1" << std::endl;
    thread_feed_back = pthread_create(&t1,NULL,t1_getMatFromCap,NULL);
    if (thread_feed_back)
    {
        std::cout << "线程一创建失败" << std::endl;
        exit(-1);
    }
    std::cout << "创建线程t2" << std::endl;
    thread_feed_back = pthread_create(&t2,NULL,t2_motionDetc,NULL);
    if (thread_feed_back)
    {
        std::cout << "线程二创建失败" << std::endl;
        exit(-1);
    }
    std::cout << "创建线程t3" << std::endl;
    thread_feed_back = pthread_create(&t3,NULL,t3_processAndSend,NULL);
    if (thread_feed_back)
    {
        std::cout << "线程三创建失败" << std::endl;
        exit(-1);
    }
    pthread_join(t1,NULL);
    pthread_mutex_destroy(&mutex);
    pthread_exit(NULL);
}