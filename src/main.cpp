#include "UdpUtils.h"
#include <thread>
#include <iostream>

int main() {

    //测试当前机器的字节序：ubuntu为小端存储
    int a = 1;
    char* p = (char*)&a;
    if(p[0] == 1) std::cout << "当前机器为小端存储" << std::endl;
    else std::cout << "当前机器为大端存储" << std::endl;

    //注意：网络通信的字节序为大端

    std::cout << "-------启动-------" << std::endl;
    UdpUtils u = UdpUtils();
    u.UdpSockInit();
    

    //std::thread th1(&UdpUtils::SendInfoLoop, &u);
    std::thread th2(&UdpUtils::RecInfoLoop, &u);
    std::thread th3(&UdpUtils::ParseInfoLoop, &u);

    //th1.join();
    th2.join();
    th3.join();

    return 0;
}