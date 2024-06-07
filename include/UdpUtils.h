#ifndef UDPUTILS_H
#define UDPUTILS_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <mutex>
#include <condition_variable>

typedef struct{
    //1(8) 3(4)
    double timeStamp;
    int vehicleID; // 发送者ID ，101-104 代表底盘，0x33代表车载指控，0x44代表便携指控
                    //101 空降车  102 支援保障车    103 转35  104 黄猛士
    int frameID;  //  0xf1 底盘->指控  底盘状态 0xf2：底盘->指控  检测数据
                //  0xA1:指控->底盘 编队信息， 0xA2：指控->底盘 控制1，0xA3：指控->底盘 控制2
    int module;
    int wai;
} MESSAGE_HEAD;

typedef struct{
    //1(8) 24(4) 6(8)
    double timeStamp;
    int id;  //车辆的id
    int role; //车辆目前的角色。（初始时读取配置文件获取，系统运行后从指控获取，或者保持以前的角色）
    int engineSpeed;//N7----发动机运行状态
    int shift; //n2----档位  1:N  2:D 3:R  4:P
    int throttle;  //0-100%
    int brake;      //0-100%
    int steering;   //left +; right -
    int park;
    int horn;
    int turnLignt;  // 1:left 2:right
    int errCode;//N3----底层控制器错误码或故障码
    int controlMode;  //1:manual 2:auto
    int speed;  //n1----速度  cm/s
    int gpsHeading;  //0.01 degree
    int gpsFlag;
    int heading;//N10----车体方向角 0.01 degree
    int pitch;//N11----车体横滚角   0.01 degree
    int roll;//N12----车体俯仰角   0.01 degree
    int acc_x;   //横向加速度,右为正
    int acc_y;   //纵向加速度，前为正
    int acc_z;   //
    int satelliteNum;  //卫星数目
    int environment;   //车体环境 
    //0有烟雾干扰通讯正常 1 无烟雾干扰通讯正常 2无烟雾干扰通讯不正常 3有烟雾干扰通讯不正常
    //显式8字节对齐占位
    int align;

    double localX;
    double localY;
    double gpsX;//N8----车体当前经度
    double gpsY;//N9----车体当前纬度
    double lont; // 经度
    double lat; // 纬度
} INDIVIDUAL_VEHICLE;

//发送数据的格式
typedef  struct{
    MESSAGE_HEAD head;
    INDIVIDUAL_VEHICLE individualVehicle;
} FORMATION_INFO_SEND;

//接收数据的格式
typedef  struct{
    MESSAGE_HEAD head;
    int formationScale;  //the num of individuals in the formation
    int formationShape;   //1:Longitudinal 2:rhombus 9:no
    INDIVIDUAL_VEHICLE individualVehicle[4];
} FORMATION_INFO_RECEIVE;

typedef struct{
    MESSAGE_HEAD head;
    int actionN[20];
} FORMATION_CONTROL;

const char hexChar[16] = {'0', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

//定义udp工具的类
class UdpUtils {
public:
    UdpUtils();
    //初始化Udp服务器, 0失败, 1成功
    int UdpSockInit();
    //循环发送数据
    void SendInfoLoop();
    //循环接收数据
    void RecInfoLoop();
    //循环处理数据
    void ParseInfoLoop();
    //将待发送的小端数据转换为大端数据
    void convertSendInfo();

public:
    //是否已经获取到客户端的信息
    bool clientReady;
    //发送的数据是否已经准备完毕
    bool sendReady;
    //获取的数据是否已经取走
    bool recBufferEmpty;

    //发送数据的互斥锁
    std::mutex sendMutex;
    
    //接收数据的互斥锁
    std::mutex recMutex;
    //接收数据的条件变量
    std::condition_variable recCV;

    //udp服务器套接字
    int udpServer;
    //本机地址
    struct sockaddr_in serverAddr;
    //客户端地址
    struct sockaddr_in clientAddr;

    //发送的数据
    FORMATION_INFO_SEND formationInfoSend;
    //接受数据的缓冲
    unsigned char recBuffer[1280];
};

#endif 