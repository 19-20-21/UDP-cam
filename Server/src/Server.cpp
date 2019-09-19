#include "head.h"
#include "config.h"
#include "Server.h"


SevPro::SevPro(){
    std::cout << "Serv类启动" << std::endl;
}
SevPro::~SevPro(){
    std::cout << "Serv类摧毁" << std::endl;
}


//全局变量
std::queue<struct UDP_pack> packQue;
UDPSock udpSock;
SevPro sevpro;
cv::VideoWriter myVideoWriter;
//线程锁
pthread_mutex_t mutex;

void *t1_getPackBack(void *junk){
    struct UDP_pack buffer; // 接收数据的数组
    int recvMsgSize; // 接受到数据的大小
    while (1)
    {
        std::cout <<"当前队列元素个数：" <<packQue.size() << std::endl;
        recvMsgSize = udpSock.RecvSock(&buffer,BUF_SIZE);
        if(buffer.flag != 0||recvMsgSize!=ARMED_PACK)continue;

        std::cout << "即将接受包的大小："<<buffer.pack_len<< std::endl;
        pthread_mutex_lock(&mutex);
        packQue.push(buffer);
        pthread_mutex_unlock(&mutex); 
        while (buffer.flag!=2)
        {
            recvMsgSize = udpSock.RecvSock(&buffer,BUF_SIZE); 
            if (recvMsgSize!=ARMED_PACK)
            {
                std::cerr<<"收到不能识别的包，大小："<<recvMsgSize<<std::endl;
            }
            else
            {
                pthread_mutex_lock(&mutex);
                packQue.push(buffer);
                pthread_mutex_unlock(&mutex);
            }
        }
        while (packQue.size()>1000)
        {
            pthread_mutex_lock(&mutex);
            packQue.pop();
            pthread_mutex_unlock(&mutex);
        }
        
    }
}

void *t2_combAndShow(void *junk){
    clock_t last_cycle = clock();//计时器
    struct UDP_pack buffer;
    bool losePack;
    int counter;
    int pngid=0;
    std::string path = "build/img/" ;
    std::string format = ".png";
    std::string filename;
    while (1) {
        while (packQue.size()<100)
        {
            sleep(1);
        }
        
        losePack = false;
        counter = 0;
      
        while (packQue.front().flag!=0)//拿到包头
        {
            pthread_mutex_lock(&mutex);              
            packQue.pop();
            pthread_mutex_unlock(&mutex);          
        }
        buffer = packQue.front();
        int total_pack = buffer.pack_len;
        char * longbuf = new char[EACH_PACK * total_pack];
        memcpy( & longbuf[0], buffer.encode, EACH_PACK);
        counter++;
        pthread_mutex_lock(&mutex);
        packQue.pop();
        pthread_mutex_unlock(&mutex);
      
        while (packQue.front().flag!=2)
        {
            buffer = packQue.front();
            std::cout << "包总数："<<total_pack<<"bufferflag:"<<buffer.flag <<" s:"<<buffer.pack_id<<std::endl;
            if (buffer.flag==0||buffer.pack_id>total_pack)//如果flag为零,或id大于总包数，意味着拿到了其他包的包头，一定有丢包。
            {
                losePack = true;
                break;
            }
            if (counter==total_pack)//如果计数器大于了包的总长度，一定有丢包。
            {
                losePack = true;
                break;
            }
            if(buffer.pack_len!=total_pack)continue;
            memcpy(&longbuf[(buffer.pack_id-1)*EACH_PACK],buffer.encode,EACH_PACK);
            counter++;
            pthread_mutex_lock(&mutex);
            packQue.pop();
            pthread_mutex_unlock(&mutex);

        }
        if (losePack)
        {
            free(longbuf);
            continue;
            }
        buffer = packQue.front();
        if (buffer.pack_id>total_pack)
        {
            free(longbuf);
            continue;
        }
        
        memcpy(&longbuf[(buffer.pack_id-1)*EACH_PACK],buffer.encode,EACH_PACK);
        counter++;
        pthread_mutex_lock(&mutex);
        packQue.pop();
        pthread_mutex_unlock(&mutex);
    //丢包判断
        if ((double)counter/(double)buffer.pack_len!=1)
        {
            std::cout << "丢包率："<<1-((double)counter/(double)buffer.pack_len) << std::endl;
            free(longbuf);
            continue;
        }
        cv::Mat rawData = cv::Mat(1, EACH_PACK * total_pack, CV_8UC1, longbuf);
        cv::Mat frame = imdecode(rawData, CV_LOAD_IMAGE_COLOR);
        if (frame.size().width == 0) {
            std::cerr << "解码失败" << std::endl;
            free(longbuf);
            continue;
            }
        imshow("recv", frame);
        std::string fm = path+std::to_string(pngid)+format;
        std::cout<<fm<<std::endl;
        cv::imwrite(fm,frame);
        pngid++;
        
        free(longbuf);
        if (sevpro.tflag==1)
        {
            myVideoWriter = cv::VideoWriter("build/video/cap.mp4", sevpro.myFourcc, 2, sevpro.size_mat, true);
            sevpro.tflag = 2;
        }
        if (sevpro.tflag==0)
        {
            sevpro.size_mat = frame.size();
            sevpro.myFourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');//mp4
            sevpro.tflag = 1;
        }
        else
        {
            if (!frame.empty())
                myVideoWriter.write(frame);
        }

        if (cv::waitKey(20)==27)
        {
            myVideoWriter.release();
            exit(0);
        }
        
        clock_t next_cycle = clock();
        double duration = (next_cycle - last_cycle) / (double) CLOCKS_PER_SEC;
        std::cout << "\teffective FPS:" << (1 / duration) << " \tkbps:" << (EACH_PACK * total_pack / duration / 1024 * 8) << std::endl;

        std::cout << next_cycle - last_cycle;
        last_cycle = next_cycle;
        }
}




int main(int argc, char** argv) {

    if (argc != 2) { // Test for correct number of parameters
        std::cerr << "用法" << argv[0] << " <端口名>" << std::endl;
        exit(1);
    }

    cv::namedWindow("recv", CV_WINDOW_AUTOSIZE);
    udpSock.ServSockBind(argv);//绑定套接字
    pthread_t t1,t2;
    int thread_feed_back;
    std::cout << "创建线程t1" << std::endl;
    thread_feed_back = pthread_create(&t1,NULL,t1_getPackBack,NULL);
    if (thread_feed_back)
    {
        std::cout << "线程一创建失败" << std::endl;
        exit(-1);
    }
    std::cout << "创建线程t2" << std::endl;
    thread_feed_back = pthread_create(&t2,NULL,t2_combAndShow,NULL);
    if (thread_feed_back)
    {
        std::cout << "线程二创建失败" << std::endl;
        exit(-1);
    }
    pthread_join(t1,NULL);
    pthread_join(t2,NULL);
    pthread_mutex_destroy(&mutex);
    pthread_exit(NULL);
}
