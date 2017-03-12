//--------------------------------------【程序说明】-------------------------------------------
//		程序说明：NCURoboTeam专用遥控（OPENCV版）
//		程序描述：NCURoboTeam Remote
//		开发测试所用IDE版本：Visual Studio 2015
//		开发测试所用OpenCV版本：	2.4.9
//		2017年02月 Created by @丁峰
//		2017年03月 Revised by @丁峰
//------------------------------------------------ ------------------------------------------------

//---------------------------------【头文件、命名空间包含部分】-----------------------------
//		描述：包含程序所使用的头文件和命名空间
//-------------------------------------------------------------------------------------------------
#include <opencv2/opencv.hpp>
//#include "DFCOM.h"
using namespace cv;
using namespace std;
//-----------------------------------【宏定义部分】--------------------------------------------
//  描述：定义一些辅助宏 
//------------------------------------------------------------------------------------------------ 
#define PICDIV_WINDOW_NAME "【图传窗口】"        //为窗口标题定义的宏 
#define REMOTE_WINDOW_NAME "【手柄窗口】"        //为窗口标题定义的宏 

#define KEY0    1                         //用作重置
#define KEY1    2                         //偏差A键
#define KEY2    3                         //偏差B键
#define KEY3    4                         //确认偏差键
#define KEY4    5                         //模式切换键
#define KEY1C   10                        //偏差A键继续键（活动）
#define KEY2C   11                        //偏差B键继续键（活动）
/////////////////////////////////////////////////////////////////////////////
#define KEY0B    21                         //

//-----------------------------------【全局函数声明部分】------------------------------------
//		描述：全局函数的声明
//------------------------------------------------------------------------------------------------
void on_MouseHandle(int event, int x, int y, int flags, void* param);
void DrawRectangle( cv::Mat& img, cv::Rect box );
void ShowHelpText( );
void DrawTransCir(Mat img, int x, int y, int width, int height, Scalar& color, double alpha);
void DrawDivLine(Mat &image, Rect Point1, Rect Piont2, Scalar color, int offset);
void DrawMainCircle(Mat& roi, int textx, int texty, Scalar color);
int  KeyJudge(int x, int y);
void imgReset(Mat &img);
void imgResetBackGroung(Mat &img);
void KeyProgressLButtomDown(int x, int y);
void KeyProgressMouseMove(int x, int y);

//-----------------------------------【结构体声明部分】-----------------------------------
//		描述：自定义结构体的声明
//-----------------------------------------------------------------------------------------------

//-----------------------------------【全局变量声明部分】-----------------------------------
//		描述：全局变量的声明
//-----------------------------------------------------------------------------------------------

Rect g_rectangleK0;                   //按键们的位置
Rect g_rectangleK1;
Rect g_rectangleK2;
Rect g_rectangleK3;
Rect g_rectangleK4;

bool g_bStatusClean = false;                                  //是否进行重置操作
RNG g_rng(12345);

bool g_bKey1Press=false;                                      //Key1是否按下
bool g_bKey2Press = false;                                    //Key2是否按下
bool g_bDivConfirm = false;                                   //Key3是否按下
char g_bModuleIndex =  0;                                     //Key4是否按下,模块索引,0为PICDIV，1为REMOTE

Rect g_lastrectangle = Rect(-200, -200, 0, 0);                //局部刷新用的全局Rect,记录上一次位置
Rect g_lastrectangle1 = Rect(-200, -200, 0, 0);               //局部刷新用的全局Rect,记录上一次位置
Rect g_bDivValue=Rect(0, 0, 0, 0);
Mat g_ChangeSave;                                             //局部刷新用的全局Mat
Mat g_ChangeSave1;                                            //局部刷新用的全局Mat

Mat tempImage;                                                //一副单独的图片存储（更新、刷新用）
///////////////////////////////////////////////////////////////////////////////////////////////////////////
Rect g_rectangleK0B;


//--------------------------------------------------------------------------------------


//-----------------------------------【main( )函数】--------------------------------------------
//		描述：控制台应用程序的入口函数，我们的程序从这里开始执行
//------------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	//【1】改变console字体颜色
	system("color 9F");

	//【2】显示欢迎和帮助文字
	ShowHelpText();
	//【3】串口连接
	//if (COM_Connect("COM3", 115200))
	//{
	//	//连接失败
	//	getchar();
	//	return -1;
	//}

	//【4】准备参数
	//图传窗口参数
	Mat dfpic;
	imgReset(dfpic);
	//手柄窗口参数
	Mat remotepic(600,960,CV_8UC3);
	remotepic = Scalar::all(0);     //全黑屏

	//【5】设置鼠标操作回调函数
	//窗口模式改为可变模式较为适合平板显示
	namedWindow(PICDIV_WINDOW_NAME, 0);
	//moveWindow(PICDIV_WINDOW_NAME, 200, 50);
	setMouseCallback(PICDIV_WINDOW_NAME, on_MouseHandle, (void*)&dfpic);
	//////////////////////////////////////
	//namedWindow(REMOTE_WINDOW_NAME, WINDOW_AUTOSIZE);
	//setMouseCallback(REMOTE_WINDOW_NAME, on_MouseHandle, (void*)&remotepic);
	//////////////////////////////////////

	//【6】程序主循环
	while (1)
	{
		
		switch (g_bModuleIndex)
		{
		case 0:
		{
			if (g_bStatusClean)imgReset(dfpic);
			imshow(PICDIV_WINDOW_NAME, dfpic);
			if (waitKey(10) == 27) break;//按下ESC键，程序退出
		}
		break;

		case 1:
		{
			if (g_bStatusClean)imgReset(remotepic);
			imshow(PICDIV_WINDOW_NAME, remotepic);
			if (waitKey(10) == 27) break;//按下ESC键，程序退出
		}
		break;


		}
	}
	return 0;
}

//--------------------------------------------------------------------------------------
//-----------------------------------【DrawTransCir】-----------------------------------
//		描述：在一个ROI区域内画一个有透明度的圆形
//      参数：
//           int x, int y, int width, int height  分别是ROI区域的坐标x,y和长宽；
//           Scalar& color                        圆的BRR颜色
//           double alpha                         透明度
//-----------------------------------------------------------------------------------------------
void DrawTransCir(Mat img, int x, int y, int width, int height,  Scalar& color, double alpha)
{
	Mat  rec=img(Rect(x,y, width, height));
	Mat  rec1 = rec.clone();
	circle(rec1, Point( width / 2,  height / 2), 20, color, -1,8);
	addWeighted(rec, alpha, rec1, 1 - alpha, 0.0, rec);
}

//-----------------------------------【DrawDivLine】-----------------------------------
//		描述：画两个ROI点往左下偏置offset个单位坐标的点的连线，并在此线中间打印出两点间偏差
//      参数：
//           Mat &image                           Mat类图；
//           Rect Point1, Rect Piont2             两个ROI点的Rect位置
//           Scalar color                         线和文字的颜色
//           int offset                           偏置值
//-----------------------------------------------------------------------------------------------

void DrawDivLine(Mat &image,Rect Point1, Rect Piont2, Scalar color,int offset)
{
	char send_message[20];
	line(image, Point(Point1.x + offset, Point1.y + offset), Point(Piont2.x + offset, Piont2.y + offset), color, 1, CV_AA, 0);
	g_bDivValue = Rect(Point1.x - Piont2.x, Point1.y - Piont2.y, 0, 0);
	_snprintf(send_message, 13, "(%d,%d)\0", g_bDivValue.x, g_bDivValue.y);
	putText(image, send_message, cvPoint((Point1.x + Piont2.x ) / 2+ offset, (Point1.y + Piont2.y ) / 2+ offset), FONT_HERSHEY_COMPLEX_SMALL, 1, color, 1, 8);

}

//-----------------------------------【DrawMainCircle】-----------------------------------
//		描述：画光标主图案。手点的圆不在这里画出；
//      参数：
//           Mat &roi                             Mat类图；
//           int textx,int texty                  打印的坐标数字，方便观察
//           Scalar color                         线和文字的颜色
//-----------------------------------------------------------------------------------------------
void DrawMainCircle(Mat& roi,int textx,int texty, Scalar color)
{
	char send_message[20];
	_snprintf(send_message, 10, "%d,%d\0", textx, texty);
	DrawTransCir(roi, 0, 0, 50, 50, color, 0.7);
	line(roi, Point(0, 25), Point(50, 25), color, 1, CV_AA, 0);
	circle(roi, Point(25, 25), 10, color, 1, 8);
	line(roi, Point(25, 0), Point(25, 50), color, 1, CV_AA, 0);
	putText(roi, send_message, cvPoint(50, 25), FONT_HERSHEY_COMPLEX_SMALL, 1, color, 1, 8);
}
//-----------------------------------【KeyJudge】-----------------------------------
//		描述：判断鼠标所点击的区域是否在特定虚拟按键上，若是则分别返回相应键值，否则返回0；
//            非普适，仅为代码简洁整理；
//      参数：
//           int x, int y                         分别是鼠标回调函数中的鼠标坐标x,y；
//-----------------------------------------------------------------------------------------------
int KeyJudge(int x, int y)
{
	if (g_bModuleIndex == 0)
	{
		if (x > g_rectangleK0.x&&x<(g_rectangleK0.x + g_rectangleK0.width) && y>g_rectangleK0.y&&y < (g_rectangleK0.y + g_rectangleK0.height))
		{
			int p1 = (g_rectangleK0.x + g_rectangleK0.width / 2);
			int p2 = (g_rectangleK0.y + g_rectangleK0.height / 2);
			if ((x - p1)*(x - p1) + (y - p2)*(y - p2) <= 400)
			{
				return KEY0;
			}
		}

		else if (x > g_rectangleK1.x&&x<(g_rectangleK1.x + g_rectangleK1.width) && y>g_rectangleK1.y&&y < (g_rectangleK1.y + g_rectangleK1.height))
		{
			int p1 = (g_rectangleK1.x + g_rectangleK1.width / 2);
			int p2 = (g_rectangleK1.y + g_rectangleK1.height / 2);
			if ((x - p1)*(x - p1) + (y - p2)*(y - p2) <= 400)
			{
				return KEY1;
			}
		}
		else if (x > g_rectangleK2.x&&x<(g_rectangleK2.x + g_rectangleK2.width) && y>g_rectangleK2.y&&y < (g_rectangleK2.y + g_rectangleK2.height))
		{
			int p1 = (g_rectangleK2.x + g_rectangleK2.width / 2);
			int p2 = (g_rectangleK2.y + g_rectangleK2.height / 2);
			if ((x - p1)*(x - p1) + (y - p2)*(y - p2) <= 400)
			{
				return KEY2;
			}
		}

		else if (x > g_rectangleK3.x&&x<(g_rectangleK3.x + g_rectangleK3.width) && y>g_rectangleK3.y&&y < (g_rectangleK3.y + g_rectangleK3.height))
		{
			int p1 = (g_rectangleK3.x + g_rectangleK3.width / 2);
			int p2 = (g_rectangleK3.y + g_rectangleK3.height / 2);
			if ((x - p1)*(x - p1) + (y - p2)*(y - p2) <= 400)
			{
				return KEY3;
			}
		}
		else if (x > g_rectangleK4.x&&x<(g_rectangleK4.x + g_rectangleK4.width) && y>g_rectangleK4.y&&y < (g_rectangleK4.y + g_rectangleK4.height))
		{
			int p1 = (g_rectangleK4.x + g_rectangleK4.width / 2);
			int p2 = (g_rectangleK4.y + g_rectangleK4.height / 2);
			if ((x - p1)*(x - p1) + (y - p2)*(y - p2) <= 400)
			{
				return KEY4;
			}
		}
		else if (x > (g_lastrectangle1.x + 100) && x<(g_lastrectangle1.x + 150) && y>(g_lastrectangle1.y + 100) && y < (g_lastrectangle1.y + 150))
		{
			int p1 = (g_lastrectangle1.x + 125);
			int p2 = (g_lastrectangle1.y + 125);
			if ((x - p1)*(x - p1) + (y - p2)*(y - p2) <= 400)
			{
				return KEY2C;
			}
		}

		else if (x > (g_lastrectangle.x + 100) && x<(g_lastrectangle.x + 150) && y>(g_lastrectangle.y + 100) && y < (g_lastrectangle.y + 150))
		{
			int p1 = (g_lastrectangle.x + 125);
			int p2 = (g_lastrectangle.y + 125);
			if ((x - p1)*(x - p1) + (y - p2)*(y - p2) <= 400)
			{
				return KEY1C;
			}
		}
		return 0;
	}

	if (g_bModuleIndex == 1)
	{
		if (x > g_rectangleK0B.x&&x<(g_rectangleK0B.x + g_rectangleK0B.width) && y>g_rectangleK0B.y&&y < (g_rectangleK0B.y + g_rectangleK0B.height))
		{
			int p1 = (g_rectangleK0B.x + g_rectangleK0B.width / 2);
			int p2 = (g_rectangleK0B.y + g_rectangleK0B.height / 2);
			if ((x - p1)*(x - p1) + (y - p2)*(y - p2) <= 400)
			{
				return KEY0B;
			}
		}

	}
}


void KeyInit(Mat &img)
{
	if (g_bModuleIndex == 0)
	{
		g_rectangleK0 = Rect(50, img.rows - 75, 50, 50);                   //按键们的位置
		g_rectangleK1 = Rect(img.cols / 2 - 75, img.rows - 75, 50, 50);
		g_rectangleK2 = Rect(img.cols / 2 + 75, img.rows - 75, 50, 50);
		g_rectangleK3 = Rect(img.cols - 100, img.rows - 75, 50, 50);
		g_rectangleK4 = Rect(img.cols - 100, 50, 50, 50);

		DrawTransCir(img, g_rectangleK0.x, g_rectangleK0.y, g_rectangleK0.width, g_rectangleK0.height, Scalar(0, 255, 0), 0.5);
		DrawTransCir(img, g_rectangleK1.x, g_rectangleK1.y, g_rectangleK1.width, g_rectangleK1.height, Scalar(0, 0, 255), 0.5);
		DrawTransCir(img, g_rectangleK2.x, g_rectangleK2.y, g_rectangleK2.width, g_rectangleK2.height, Scalar(255, 0, 0), 0.5);
		DrawTransCir(img, g_rectangleK3.x, g_rectangleK3.y, g_rectangleK3.width, g_rectangleK3.height, Scalar(0, 0, 255), 0.5);
		DrawTransCir(img, g_rectangleK4.x, g_rectangleK4.y, g_rectangleK4.width, g_rectangleK4.height, Scalar(0, 0, 255), 0.5);
	}
	if (g_bModuleIndex == 1)
	{
		g_rectangleK0B = Rect(img.cols - 100, 50, 50, 50);
		DrawTransCir(img, g_rectangleK0B.x, g_rectangleK0B.y, g_rectangleK0B.width, g_rectangleK0B.height, Scalar(0, 255, 0), 0.5);

	}
}
//-----------------------------------【imgReset】-----------------------------------
//		描述：模式重置，将图片重新加载，所有参数初始化一遍
//            非普适，仅为代码简洁整理；
//      参数：
//           Mat &img                               Mat类图；
//-----------------------------------------------------------------------------------------------
void  imgReset(Mat &img)
{
	if (g_bModuleIndex == 0)
	{
		g_lastrectangle = Rect(-200, -200, 0, 0);//记录起始点
		g_lastrectangle1 = Rect(-200, -200, 0, 0);//记录起始点
		Rect g_bDivValue = Rect(0, 0, 0, 0);
		img = imread("df.jpg");

		KeyInit(img);

		img.copyTo(tempImage);
		g_bKey1Press = false;
		g_bKey2Press = false;
		g_bStatusClean = false;
		g_bDivConfirm = false;
	}
	if (g_bModuleIndex == 1)
	{
		KeyInit(img);
		g_bStatusClean = false;
	}
}
//-----------------------------------【imgResetBackGroung】-----------------------------------
//		描述：图片重置，仅将背景图片重新加载，参数不重新初始化
//            非普适，仅为代码简洁整理；
//      参数：
//           Mat &img                               Mat类图；
//-----------------------------------------------------------------------------------------------
void  imgResetBackGroung(Mat &img)
{
	tempImage.copyTo(img);
	KeyInit(img);
}
//-----------------------------------【KeyProgressLButtomDown】-----------------------------------
//		描述：通过调用KeyJudge（）返回的键值判断是哪个按键，并执行相应的操作；这个是放在鼠标回
//            调函数中左键按下中扫描的；
//            非普适，仅为代码简洁整理；
//      参数：
//           int x, int y                         分别是鼠标回调函数中的鼠标坐标x,y；
//-----------------------------------------------------------------------------------------------
void KeyProgressLButtomDown(int x, int y)
{
	if (g_bModuleIndex == 0)
	{
		switch (KeyJudge(x, y))
		{
		case KEY0: g_bStatusClean = true;  break;
		case KEY1: g_bKey1Press = true;    break;
		case KEY2: g_bKey2Press = true;    break;
		case KEY3: g_bDivConfirm = true;   break;
		case KEY4: g_bModuleIndex = 1; g_bStatusClean = true; break;      //转换模式
		case KEY2C: g_bKey2Press = true;   break;                         //这样的顺序
		case KEY1C: g_bKey1Press = true;   break;

		default:     break;
		}
	}
	if (g_bModuleIndex == 1)
	{
		switch (KeyJudge(x, y))
		{
		case KEY0B: g_bModuleIndex = 0; g_bStatusClean = true;  break;     //转换模式
		default:     break;
		}
	}


}

//-----------------------------------【KeyProgressMouseMove】-----------------------------------
//		描述：通过调用KeyJudge（）返回的键值判断是哪个按键，并执行相应的操作；这个是放在鼠标回
//            调函数中鼠标移动中扫描的；
//            非普适，仅为代码简洁整理；
//      参数：
//           int x, int y                         分别是鼠标回调函数中的鼠标坐标x,y；
//-----------------------------------------------------------------------------------------------
void KeyProgressMouseMove(int x, int y)
{
	switch (KeyJudge(x, y))
	{
	case KEY0: g_bStatusClean = true;  break;
	default:     break;
	}
}

//--------------------------------【on_MouseHandle( )函数】-----------------------------
//		描述：鼠标回调函数，根据不同的鼠标事件进行不同的操作，相当臃肿，不知道如何维护
//-----------------------------------------------------------------------------------------------
void on_MouseHandle(int event, int x, int y, int flags, void* param)
{

	Mat& image = *(cv::Mat*) param;
	switch( event)
	{
		//鼠标移动消息//平板下估计无用
	case EVENT_MOUSEMOVE:                         
		{
			KeyProgressMouseMove(x, y);
		}
		break;

		//左键按下消息
	case EVENT_LBUTTONDOWN: 
		{
			//按键检测
			KeyProgressLButtomDown(x, y);
			
			//按键处理
			if (g_bDivConfirm == true)
			{
				if ((g_lastrectangle.x != -200) && (g_lastrectangle1.x != -200))
				{
					imgResetBackGroung(image);
					Mat  rec = image(g_lastrectangle);
					DrawMainCircle(rec, g_lastrectangle.x + 25, g_lastrectangle.y + 25, Scalar(0, 0, 255));

					Mat  rec1 = image(g_lastrectangle1);
					DrawMainCircle(rec1, g_lastrectangle1.x + 25, g_lastrectangle1.y + 25, Scalar(255, 0, 0));
					DrawDivLine(image, g_lastrectangle, g_lastrectangle1, Scalar(0, 255, 0), 25);
				}
				//g_bDivConfirm一旦按下，必须重置才能清除
				break;                                                         
			}
			 if (g_bKey1Press==true)
			{
				if (g_lastrectangle.x != -200)
				{
					if (g_lastrectangle1.x != -200)
					{
						//关键，在点击的开始将缓存图更新进第一幅Save图中
						Mat temp = tempImage(g_lastrectangle);              
						g_ChangeSave = temp.clone();
					}
				}
				else
				{
					Mat rec = image(Rect(x - 125, y - 125, 150, 150));
					g_lastrectangle = Rect(x - 125, y - 125, 150, 150);
					
					g_ChangeSave = rec.clone();
				}
			}

			 if (g_bKey2Press == true)
			{
				if (g_lastrectangle1.x != -200)
				{
					if (g_lastrectangle.x != -200)
					{
						//关键，在点击的开始将缓存图更新进第一幅Save图中
						Mat temp = tempImage(g_lastrectangle1);           
						g_ChangeSave1 = temp.clone();

					}
				}
				else
				{
					Mat rec1 = image(Rect(x - 125, y - 125, 150, 150));
					g_lastrectangle1 = Rect(x - 125, y - 125, 150, 150);
					
					g_ChangeSave1 = rec1.clone();
				}
			}

			////////////////////////////

		}
		break;

		//左键抬起消息
	case EVENT_LBUTTONUP:                       
	{

		if (g_bKey1Press = true)
		{
			g_bKey1Press = false;
		}
		if (g_bKey2Press = true)
		{
			g_bKey2Press = false;
		}
		
	}

	break;

	}
	
	switch (flags)
	{
		//左键拖曳
	case CV_EVENT_FLAG_LBUTTON:
	{
		if (x + 25 > image.cols)x = image.cols - 25;
		if (y + 25 > image.rows)y = image.rows - 25;
		if (x < 0)x = 0;
		if (y < 0)y = 0;
		int px = x - 125;
		int py = y - 125;
		if (px <= 0)px = 0;
		if (py<= 0)py = 0;

		if (g_bDivConfirm != true)
		{
			if (g_bKey1Press == true) {

				//一直更新暂不活跃的那个键
				if (g_lastrectangle1.x != -200)                          
				{
					Mat temp = tempImage(g_lastrectangle1);
					Mat  rec1 = image(g_lastrectangle1);
					temp.copyTo(rec1);
					DrawMainCircle(rec1, g_lastrectangle1.x + 25, g_lastrectangle1.y + 25, Scalar(255, 0, 0));
					DrawTransCir(rec1, 100, 100, 50, 50, Scalar(255, 0, 0), 0.5);
				}
				Mat  rec = image(g_lastrectangle);
				g_ChangeSave.copyTo(rec);
				rec = image(Rect(px, py, 150, 150));
				rec.copyTo(g_ChangeSave);
				DrawMainCircle(rec, px + 25, py + 25, Scalar(0, 0, 255));
				DrawTransCir(rec, 100, 100, 50, 50, Scalar(0, 0, 255), 0.5);
				g_lastrectangle = Rect(px, py, 150, 150);
			}

			if (g_bKey2Press == true) {

				//一直更新暂不活跃的那个键
				if (g_lastrectangle.x != -200)
				{
					Mat temp = tempImage(g_lastrectangle);
					Mat  rec = image(g_lastrectangle);
					temp.copyTo(rec);
					DrawMainCircle(rec, g_lastrectangle.x + 25, g_lastrectangle.y + 25, Scalar(0, 0, 255));
					DrawTransCir(rec, 100, 100, 50, 50, Scalar(0, 0, 255), 0.5);
				}
				Mat  rec1 = image(g_lastrectangle1);
				g_ChangeSave1.copyTo(rec1);

				rec1 = image(Rect(px, py, 150, 150));

				rec1.copyTo(g_ChangeSave1);
				DrawMainCircle(rec1, px + 25, py + 25, Scalar(255, 0, 0));
				DrawTransCir(rec1, 100, 100, 50, 50, Scalar(255, 0, 0), 0.5);
				g_lastrectangle1 = Rect(px, py, 150, 150);
			}
		}
	}
	break;

	}
}

//-----------------------------------【DrawRectangle( )函数】------------------------------
//		描述：自定义的矩形绘制函数
//-----------------------------------------------------------------------------------------------
void DrawRectangle( cv::Mat& img, cv::Rect box )
{
	cv::rectangle(img,box.tl(),box.br(),cv::Scalar(g_rng.uniform(0, 255), g_rng.uniform(0,255), g_rng.uniform(0,255)));//随机颜色
}

//-----------------------------------【ShowHelpText( )函数】-----------------------------
//          描述：输出一些帮助信息
//----------------------------------------------------------------------------------------------
void ShowHelpText()
{
	//输出欢迎信息和OpenCV版本
	printf("\n\n\t\t\t\t NCURoboteam Remote\n");
	printf("\n\n\t\t\t\t      20170311\n");
	printf("\n\n\t\t\t\t当前版本为：NCURTR V1.0"  );
	printf("\n\n  ----------------------------------------------------------------------------\n");
	//输出一些帮助信息
	printf("\n\n\n\t欢迎使用\n"); 
	printf("\n\n\t请在窗口中点击鼠标左键并拖动以获取参考像素\n");

}
