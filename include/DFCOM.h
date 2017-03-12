#ifndef XIAO_H
#define XIAO_H
#include<vector>
#include <iostream>
#include <cmath>
#include "io.h"
#include <Windows.h>  

using namespace std;

#define MSG_LEN 21
#define MSG_HEADER 0xdd
#define MSG_ID 0xff

//圆柱圆心信息串口传递结构体。一帧16byte // 结构体变量位置不可擅自调动（涉及补位）
struct Pillar_t
	{

		uint8_t      FRAME_HEAD = MSG_HEADER;    //帧头
		uint8_t      DEVICE_ID = MSG_ID;         //ID
		uint8_t      SCANF_FLAG;                 //新扫描帧标志位
		uint8_t      CHECKSUM;                   //校验和
		int          CC_X;                       //圆心X坐标
		int          CC_Y;                       //圆心Y坐标
		float        CC_Tho;                     //圆心极坐标角度值
		uint8_t      pillar_sendchar[16] = {0};  //传帧数组
	};


enum {
		Timeout = 1000,               // [msec]
		EachTimeout = 2,              // [msec]
		//LineLength =1+1+1+1+4+4+4,
		SEND_DATA_MAXSIZE = 300000,
		REC_DATA_MAXSIZE =16
};

int com_changeBaudrate(long baudrate);
int com_connect(const char* device, long baudrate);
bool SetupTimeout(DWORD ReadInterval, DWORD ReadTotalMultiplier, DWORD
	ReadTotalConstant, DWORD WriteTotalMultiplier, DWORD WriteTotalConstant);
int COM_Connect(const char * com_port, const long com_baudrate);
DWORD com_send(const char* data, int size);
DWORD dfcom_send(uint8_t * data, int size);
DWORD urg_sendTag( char* tag);
DWORD com_recv(char* data, int max_size, int timeout);      //串口读取
DWORD COMREAD(char *data);


DWORD  DF_SendBinFile(FILE * fp);
DWORD urg_sendU8Tag(uint8_t * tag, int send_size);
DWORD DF_SendU8Data(uint8_t * tag, int send_size);
DWORD DF_SendCharData(char * tag);
uint32_t DF_COM_Rec(char* databuff);
uint32_t DF_REC_DATA(char* databuff);

#endif
