#include "UDPSock.h"
#include "config.h"


void UDPSock::CreateSock(){
    udp_sock = socket(AF_INET,SOCK_DGRAM,0); //UDP
	if(udp_sock == -1)
	{
        throw "创建socket失败！";
	}
}

UDPSock::UDPSock(){
    try{
        CreateSock();
    }catch(const char *msg){
        std::cerr<<msg<<std::endl;
        exit(-1);
    }
}

UDPSock::~UDPSock(){

}


void UDPSock::ClnSockBind(char *argv[]){
    udp_addr.sin_family = AF_INET;//UDP型
    try{
	udp_addr.sin_addr.s_addr = inet_addr(argv[1]);}//服务器ip地址
    catch(const std::exception& e){
        std::cerr<<"发生错误，可能是ip地址格式非法，详细信息:"<<e.what()<<std::endl;
        exit(-1);
    }
    int sockPort = atoi(argv[2]);//服务器端口
    if (sockPort<=0||sockPort>65535)
    {
        std::cerr<<"发生错误，端口格式非法"<<std::endl;
        exit(-1);
    }
    try
    {
        udp_addr.sin_port = htons(sockPort);
    }
    catch(const std::exception& e)
    {
        std::cerr <<"发生错误，可能是端口格式非法，详细信息："<<e.what() << std::endl;
        exit(-1);
    }
	udp_addr_len=sizeof(udp_addr);//地址结构体大小
    std::cout << "服务器ip："<<argv[1]<<" 端口："<<htons(sockPort)<< std::endl;
}

void UDPSock::SendSock(const void *buffer,int sendSize){
    int sendLen;
    sendLen = sendto(udp_sock,buffer,sendSize,0,(struct sockaddr *)&udp_addr,udp_addr_len);
    if (sendLen!=sendSize)
        std::cerr<<"发送失败，请检查网络环境与Socket配置!"<<std::endl; 
}

void UDPSock::ServSockBind(char *argv[]){
    memset(&udp_addr, 0, sizeof(udp_addr));
	udp_addr.sin_family = AF_INET;
	udp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int sockPort = atoi(argv[1]);//待开服务器端口
    if (sockPort<=0||sockPort>65535)
    {
        std::cerr<<"发生错误，端口格式非法"<<std::endl;
        exit(-1);
    }
    try
    {
        udp_addr.sin_port = htons(sockPort);
    }
    catch(const std::exception& e)
    {
        std::cerr <<"发生错误，可能是端口格式非法，详细信息："<<e.what() << std::endl;
        exit(-1);
    }
    udp_addr_len = sizeof(udp_addr);
    bind(udp_sock, (struct sockaddr*)&udp_addr, sizeof(udp_addr));//绑定套接字
    std::cout << "" << std::endl;
}

int UDPSock::RecvSock(void *buffer,int recvSize){
    sockaddr_in clntAddr;//客户端地址
    int recvLen;//接受到数据的长度
    if ((recvLen = recvfrom(udp_sock, buffer, recvSize, 0, (sockaddr *) &clntAddr, (socklen_t *) &udp_addr_len)) < 0) {
        std::cerr<<"接受失败"<<std::endl;
  }
    serv_ip = inet_ntoa(clntAddr.sin_addr);
    port = ntohs(clntAddr.sin_port);
    //std::cout << "接受到来自于"<<serv_ip<<":"<<port<<" "<<recvLen<<"长度的消息"<< std::endl;
  return recvLen;
}