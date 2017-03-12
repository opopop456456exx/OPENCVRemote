#pragma once

//#include "OpenRadar.h"

#include"DFCOM.h"
#include"stdio.h"





HANDLE HCom = INVALID_HANDLE_VALUE;
int ReadableSize = 0;

char* ErrorMessage = "no error.";

 int com_changeBaudrate(long baudrate)
{
	DCB dcb;

	GetCommState(HCom, &dcb);
	dcb.BaudRate = baudrate;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.fParity = FALSE;
	dcb.StopBits = ONESTOPBIT;
	SetCommState(HCom, &dcb);

	return 0;
}



 int com_connect(const char* device, long baudrate)
{
#if defined(RAW_OUTPUT)
	Raw_fd_ = fopen("raw_output.txt", "w");
#endif

	char adjust_device[16];
	_snprintf(adjust_device, 16, "\\\\.\\%s", device);           //第二种打开串口方式，可以打开COM10以上串口
	HCom = CreateFileA(adjust_device, GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (HCom == INVALID_HANDLE_VALUE) {
		return -1;
	}

	// Baud rate setting
	return com_changeBaudrate(baudrate);
}



bool SetupTimeout(DWORD ReadInterval, DWORD ReadTotalMultiplier, DWORD
	ReadTotalConstant, DWORD WriteTotalMultiplier, DWORD WriteTotalConstant)
{
	COMMTIMEOUTS timeouts;
	timeouts.ReadIntervalTimeout = ReadInterval;
	timeouts.ReadTotalTimeoutConstant = ReadTotalConstant;
	timeouts.ReadTotalTimeoutMultiplier = ReadTotalMultiplier;
	timeouts.WriteTotalTimeoutConstant = WriteTotalConstant;
	timeouts.WriteTotalTimeoutMultiplier = WriteTotalMultiplier;
	if (!SetCommTimeouts(HCom, &timeouts))
	{
		return false;
	}
	else
		return true;
}






//******************************  COM连接  *************************************************//
int COM_Connect(const char * com_port, const long com_baudrate)
{
	//const char com_port[]                              //设置串口号和波特率
	//const long com_baudrate = 115200;
    char message_buffer[40];

	if (com_connect(com_port, com_baudrate) < 0) {                        //如果波特率、串口设置失败
		_snprintf(message_buffer, 40,                     //_snprintf函数将可变个参数(...)按照format格式化成字符串，然后将其复制到str中
			"Cannot connect COM device: %s", com_port);              //写入错误信息到ErrorMessage内
		ErrorMessage = message_buffer;
		printf("urg_connect: %s\n", ErrorMessage);
		return 1;
	}
	else
	{
		printf("open com success!\n");
		printf("set DCB success!\n");
	}
	if (SetupTimeout(0, 0, 0, 0, 0))                             //设置超时
	{
		printf("Set timeout success!\n");
		return 0;
	}
	else
	{
		printf("Set timeout fail!\n");
		return 1;
	}
}
//******************************************************************************************//





/*
BOOL WriteFile(
HANDLE  hFile,//文件句柄
LPCVOID lpBuffer,//数据缓存区指针
DWORD   nNumberOfBytesToWrite,//你要写的字节数
LPDWORD lpNumberOfBytesWritten,//用于保存实际写入字节数的存储区域的指针
LPOVERLAPPED lpOverlapped//OVERLAPPED结构体指针
);
*/


DWORD com_send( const char* data, int size)
{
	DWORD n;
	WriteFile(HCom, data, size, &n, NULL);   //WriteFile函数将数据写入一个文件。
	return n;
}




 DWORD dfcom_send(uint8_t * data, int size)
 {
	 DWORD n;
	 PurgeComm(HCom, PURGE_TXCLEAR| PURGE_TXABORT| PURGE_RXCLEAR| PURGE_RXABORT);      //解决不了3391的BUG
	 WriteFile(HCom, data, size, &n, NULL);   //WriteFile函数将数据写入一个文件。
	 PurgeComm(HCom, PURGE_TXCLEAR | PURGE_TXABORT );      //解决不了3391的BUG
	 return n;
 }

// The command is transmitted to URG
 DWORD urg_sendTag( char * tag)
{
	DWORD n;
	int send_size = (int)strlen(tag);
	n=com_send(tag, send_size);

	return n;
}
 DWORD urg_sendU8Tag(uint8_t * tag,int send_size)
 {
	 DWORD n;
	// int send_size = (int)sizeof(tag);
	 n = dfcom_send(tag, send_size);

	 return n;
 }


 DWORD DF_SendCharData(char * tag)
 {
	 char send_message[SEND_DATA_MAXSIZE];
	 DWORD n;
	 _snprintf(send_message, SEND_DATA_MAXSIZE, "%s\r\n", tag);
	 n = urg_sendTag(send_message);
	 return n;
 }
	
 DWORD DF_SendU8Data(uint8_t * tag,int send_size)
 {
	 uint8_t send_message[SEND_DATA_MAXSIZE];
	 DWORD n;
	 
	 memcpy(send_message, tag, send_size);
	 printf("sizeof:%d\n", send_size);
	send_message[send_size] = '\r';
	 send_message[send_size+1] = '\n';
	// _snprintf(send_message, SEND_DATA_MAXSIZE, "%s\r\n", tag);
	 n = urg_sendU8Tag(send_message, send_size +2);
	 printf("actully send size:%d\n", n);
	 return n;
 }



 DWORD  DF_SendBinFile(FILE * fp)
 {
	 DWORD n;
	 //FILE *fff;
	 uint8_t send_message[SEND_DATA_MAXSIZE];
	 int size;
	 fseek(fp, 0, SEEK_END);   ///将文件指针移动文件结尾
	 size = ftell(fp); ///求出当前文件指针距离文件开始的字节数
	 if (size > 2147483646)
	 {
		 printf("the file is too big!\n");
		 return -1;
	 }
	// fseek(fp, 0, SEEK_SET);   ///将文件指针移动文件结尾
	 rewind(fp);
	//////////////////////////////////////////////////////////////////////传输数据格式！///////////////////////////////////////////////////////
	 //////////////////////////////////////////////////////////////////////////////////////////////////
	 send_message[0] = 0xDF;                                             //帧头 0xDFDF
	 send_message[1] = 0xDF;
	 send_message[2] = 0xFD;                                             //帧头 0xDFDF
	 send_message[3] = 0xFD;
	 //////////////////////////////////////////////////////////////////////////////////////////////////
	 send_message[4] = (uint8_t)(size>>24);                             //数据长度，4字节,高位在前
	 send_message[5] = (uint8_t)(size >> 16);
	 send_message[6] = (uint8_t)(size >> 8);
	 send_message[7] = (uint8_t)(size);
	// printf("read,2,3,4,5,,%x,%x,%x,%x\n", size, send_message[2], send_message[3], send_message[4], send_message[5]);
	 ////////////////////////////////////////////////////////////////////////////////////////////////////

	 fread(send_message+8, sizeof(char), size, fp);                       //数据               
	 ///////////////////////////////////////////////////////////////////////////////////////////////////
	 send_message[size+8] = '\r';                                         //结束符2字节
	 send_message[size + 9] = '\n';
	 printf("read file sucess!,size=%d\n", size);
	// send_message[size + 8] = '\0';
	 ////////////////////////////////////////////////////////////////////////////////////////////////////

	// printf("read file sucess!,size=%d,3906,3907,3908,%x,%x,%x,%x,%x,%x\n", size, send_message[3904], send_message[3905], send_message[3906], send_message[3907], send_message[3908], send_message[3909]);
	 n = dfcom_send(send_message, size+10);

	 return n;

 }


 DWORD com_recv(char* data, int max_size, int timeout)      //串口读取
{
	if (max_size <= 0) {
		return 0;
	}

	if (ReadableSize < max_size) {
		DWORD dwErrors;
		COMSTAT ComStat;
		ClearCommError(HCom, &dwErrors, &ComStat);
		ReadableSize = ComStat.cbInQue;              //当前串口中存有的数据个数
	}

	if (max_size > ReadableSize) {
		COMMTIMEOUTS pcto;
		int each_timeout = 2;

		if (timeout == 0) {
			max_size = ReadableSize;

		}
		else {
			if (timeout < 0) {
				/* If timeout is 0, this function wait data infinity */   //如果timeout为0，此函数无限制等待数据
				timeout = 0;
				each_timeout = 0;
			}

			/* set timeout */
			GetCommTimeouts(HCom, &pcto);
			pcto.ReadIntervalTimeout = timeout;
			pcto.ReadTotalTimeoutMultiplier = each_timeout;
			pcto.ReadTotalTimeoutConstant = timeout;
			SetCommTimeouts(HCom, &pcto);
		}
	}

	DWORD n;
	ReadFile(HCom, data, (DWORD)max_size, &n, NULL);
#if defined(RAW_OUTPUT)
	if (Raw_fd_) {
		for (int i = 0; i < n; ++i) {
			fprintf(Raw_fd_, "%c", data[i]);
		}
		fflush(Raw_fd_);
	}
#endif
	if (n > 0) {
		ReadableSize -= n;
	}

	return n;
}


 DWORD COMREAD(char *data)
{

	//	char str[100];

	DWORD dwErrors;
	COMSTAT ComStat;

	DWORD wCount;//读取的字节数  
	BOOL bReadStat;

	ClearCommError(HCom, &dwErrors, &ComStat);
	ReadableSize = ComStat.cbInQue;              //当前串口中存有的数据个数
												 //	printf("ReadableSize=%d", ReadableSize);
	bReadStat = ReadFile(HCom, data, 1, &wCount, NULL);
	//data  = str;
	if (!bReadStat)
	{
		printf("Fail to read the COM!\n");
		return FALSE;
	}

	return wCount;

}




 uint32_t DF_COM_Rec(char* databuff)
 {
	 char buffer0;
	 uint32_t USART_RX_STA = 0;
	 size_t l;
	 while (1)
	 {

		 if ((l = COMREAD(&buffer0)) > 0) {
			 //	printf(",receve:%s,--%d----wd:%d\n", buffer0, j++,l);
			 //if ((USART_RX_STA & 0x80000) == 0)// haven't finished receving yet
			 //{
				 if ((buffer0 == '\n'))                 //必须\n为结束                    //if ((buffer0[0] == '\n')||(buffer0[0] == 0x0d))
				 {
					 databuff[(USART_RX_STA & 0X7FFFF)] = '\0';
					 return  USART_RX_STA |= 0x80000;	//finished receving alreadly 
					 
				 }
				 databuff[(USART_RX_STA & 0X7FFFF)] = buffer0;
				 USART_RX_STA++;
				 if (USART_RX_STA > (REC_DATA_MAXSIZE - 1)) {
					 USART_RX_STA = 0;
					
					 return USART_RX_STA;
				 }//the wrong data,restart有翻页效果，饱和的数据全部丢弃，若后面的数据不超范围，后面数据都会保留。

			 }
		 else {
			 USART_RX_STA = 0;          //
			
			 return USART_RX_STA;
		 }

	 }
 }

 uint32_t DF_REC_DATA(char* databuff)
 {
	 uint32_t datalen;
	 datalen = DF_COM_Rec(databuff);
	 PurgeComm(HCom, PURGE_TXCLEAR | PURGE_TXABORT | PURGE_RXCLEAR | PURGE_RXABORT);      
	 if ((datalen & 0x80000) == 0)  //接收未完成
	 {
		 return 0;
	 }
	 else {                         //接收完成

		 return  (datalen & 0x7ffff);//得到此次数据的长度
	 }

 }
 //void DFCOMPush(Pillar_t*  pillar, vector<ArcPara>& FittedArc,int i)
 //{
	// pillar->CHECKSUM = 0;
	// pillar->CC_X = FittedArc.at(i).center.x;
	// pillar->CC_Y = FittedArc.at(i).center.y;
	// pillar->CC_Tho = (float)(pillar->CC_X / pillar->CC_Y);
	//
	///* pillar->CHECKSUM = 0;
	// pillar->CC_X = 0x11223344;
	// pillar->CC_Y = 0x55667788;
	// pillar->CC_Tho = (float)154.654;*/
	//
	// if (i == 0)
	//	 pillar->SCANF_FLAG = 1;
	// else
	//	 pillar->SCANF_FLAG = 0;

	// memcpy(pillar->pillar_sendchar , pillar, 16);
	// for (size_t j = 4; j < 16;j++)
	// {
	//	pillar->CHECKSUM += pillar->pillar_sendchar[j];
	// }
	// pillar->CHECKSUM = pillar->CHECKSUM & 0x3f;

	// memcpy(pillar->pillar_sendchar, pillar, 16);
	///* memcpy(pillar, pillar->pillar_sendchar, 16);
	// cout << " x: " << pillar->CC_X
	//	 << " y: " << pillar->CC_Y
	//	 << " r: " << pillar->CC_Tho << endl;*/
	//
	//
 //}


 //int DFCOMPop(Pillar_t *pillar,size_t len)
 //{
	//
	// int send_size = dfcom_send(pillar->pillar_sendchar, len);
	// /*char buf[16] = { 0 };

	// int send_size = urg_sendTag(buf);*/
	// return send_size;
 //}