#ifndef UDPSOCK_H_
#define UDPSOCK_H_

#include "head.h"

class UDPSock
{
public:
//构造函数，初始化时创建套接字，异常处理。
UDPSock();
//析构函数，关闭连接，清理缓冲区
~UDPSock();
//创建套接字
void CreateSock();
//客户端绑定地址、端口到套接字
void ClnSockBind(char *argv[]);
//服务端绑定地址、端口到套接字
void ServSockBind(char *argv[]);
//返回服务器ip和端口
std::string GetIpAndPort();
//发送数据
void SendSock(const void *buffer,int sendSize);
//接受数据
int RecvSock(void *buffer,int recvSize);

private:
int udp_sock;//套接字，由构造函数初始化
struct sockaddr_in udp_addr;//socket地址，待绑定ip、端口、套接字类型
int udp_addr_len;//socket地址长度
char *serv_ip;//服务器地址
int port;//端口
};



#endif