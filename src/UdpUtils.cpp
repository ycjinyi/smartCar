#include "UdpUtils.h"
#include <cstring>
#include <iostream>
#include <string>



UdpUtils::UdpUtils(): clientReady(false), sendReady(false), 
recBufferEmpty(true), udpServer(0) {}

//创建UDP服务器
int UdpUtils::UdpSockInit() {
    udpServer = socket(AF_INET,SOCK_DGRAM,0);

    if(udpServer < 0) return 0;

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
//  serverAddr.sin_addr.s_addr = inet_addr("192.168.2.1");
    serverAddr.sin_port = htons(20002);

    if(bind(udpServer, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) return 0;
    std::cout << "-------初始化UDP接口-------" << std::endl;
    return 1;
}


//循环发送数据
void UdpUtils::SendInfoLoop() {
    while(1) {
        if(clientReady && sendReady) {
            sendMutex.lock();
            //准备数据
            if(sendReady) {
                //数据准备好后，需要将小端字节序转换为网络的大端字节序后发送?
                
                std::cout << "-------发送数据-------" << std::endl;
                //发送数据
                sendto(udpServer, (void*)&formationInfoSend, sizeof(FORMATION_INFO_SEND), 0,
                   (sockaddr*)&clientAddr, sizeof(clientAddr));
                sendReady = false;
            }
            sendMutex.unlock();
        }
    }
}

//循环接收处理数据
void UdpUtils::RecInfoLoop() {

    //数据缓冲区
    unsigned int buffSize = sizeof(recBuffer);
    ssize_t bytesReceived = 0;
    socklen_t addrLen = sizeof(clientAddr);

    while(1) {
        std::cout << "-------等待接收数据-------" << std::endl;
        //等待接收缓冲区空闲
        std::unique_lock<std::mutex> recLock(recMutex);
        recCV.wait(recLock, [&]{return recBufferEmpty;}); 
        //从客户端读取数据
        bytesReceived = recvfrom(udpServer, (void*)&recBuffer, buffSize, 0, 
            (struct sockaddr*)&clientAddr, &addrLen);
        std::cout << "-------接收到数据-------" << std::endl;

        if(bytesReceived <= 0) continue;

        clientReady = true;

        //接收到有效的数据
        if(bytesReceived > sizeof(MESSAGE_HEAD) && bytesReceived <= buffSize) {
            //需要将接受到的大端序网络数据转换为小端序?
            
            std::string str;
            //按照字节打印收到的数据
            for(int i = 0; i < bytesReceived; ++i) {
                char now = recBuffer[i];
                str += "0x";
                str += hexChar[now / 16];
                str += hexChar[now % 16];
                str += " ";
                if(i % 10 == 9) str += '\n';
            }
            std::cout << "数据信息->" << str << std::endl;

            //切换状态
            recBufferEmpty = false;
            //通知线程获取数据
            recCV.notify_one();
        }
    }
}

//循环处理数据
void UdpUtils::ParseInfoLoop() {
    while(1) {
        std::cout << "-------等待消费数据-------" << std::endl;
        //等待接收缓冲区空闲
        std::unique_lock<std::mutex> recLock(recMutex);
        recCV.wait(recLock, [&]{return !recBufferEmpty;}); 
        //处理数据代码

        //数据的处理举例
        MESSAGE_HEAD* head = (MESSAGE_HEAD*)&recBuffer;
        switch (head->frameID) {
            case 0xA1: //编队信息
            {
                FORMATION_INFO_RECEIVE* info = (FORMATION_INFO_RECEIVE*)&recBuffer;
                // 业务逻辑处理
            }
                break;
            case 0xA2: //控制信息
            {
                FORMATION_CONTROL* control = (FORMATION_CONTROL*)&recBuffer;
                // 业务逻辑处理
            }
                break;
        }

        std::cout << "-------正在消费数据-------" << std::endl;
        //切换状态
        recBufferEmpty = true;
        recCV.notify_one();
    }
}