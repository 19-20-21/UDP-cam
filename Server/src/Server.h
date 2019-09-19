#ifndef SERVER_H_
#define SERVER_H_
#include "UDPSock.h"
#define __STDC_CONSTANT_MACROS  
  

class SevPro {

private:

public:
SevPro();
~SevPro();
int myFourcc;
cv::Size size_mat;
int tflag;
};

#endif