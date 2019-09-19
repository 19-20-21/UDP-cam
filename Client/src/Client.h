#ifndef CLINT_H_
#define CLINT_H_
#include "UDPSock.h"
#include "head.h"
#define JZ 8

class ClnPro {

private:
const char* rstp_addr;

public:
cv::VideoCapture cap;
ClnPro();
~ClnPro();
void setRstpAddr(const char* rstpaddr);
int openVideoStream();
int motionD(cv::Mat pre,cv::Mat curr);
};

#endif